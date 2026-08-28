#include "GLcd1602.h"

#include "stm32f4xx_hal.h"

/* PCF8574T address range is 0x20-0x27. Change this if A0-A2 differ. */
#define LCD_I2C_ADDRESS_MIN   0x20U
#define LCD_I2C_ADDRESS_MAX   0x27U
#define LCD_SCL_PORT          GPIOB
#define LCD_SCL_PIN           GPIO_PIN_8
#define LCD_SDA_PORT          GPIOB
#define LCD_SDA_PIN           GPIO_PIN_9

/* Common PCF8574 LCD backpack mapping: P0=RS, P1=RW, P2=E, P3=BL. */
#define LCD_RS                0x01U
#define LCD_RW                0x02U
#define LCD_ENABLE            0x04U
#define LCD_BACKLIGHT         0x08U

#define LCD_HEALTH_INTERVAL_MS    100U
#define LCD_RECONNECT_INTERVAL_MS 500U
#define LCD_HEALTH_MASK            (LCD_RW | LCD_ENABLE | LCD_BACKLIGHT)

#define LCD_COMMAND_CLEAR     0x01U
#define LCD_COMMAND_HOME      0x02U
#define LCD_COMMAND_ENTRY     0x06U
#define LCD_COMMAND_DISPLAY   0x0CU
#define LCD_COMMAND_FUNCTION  0x28U
#define LCD_COMMAND_DDRAM     0x80U

static uint8_t cursorColumn;
static uint8_t cursorRow;
static uint8_t lcdI2cAddress = LCD_I2C_ADDRESS_MAX;
static uint8_t lcdLastOutput;
static char lcdFrame[LCD1602_ROWS][LCD1602_COLUMNS];
static bool lcdReady;
static uint32_t nextHealthCheckTick;
static uint32_t nextReconnectTick;

static void I2cDelay(void)
{
    volatile uint32_t count = SystemCoreClock / 400000U;
    while (count-- > 0U)
    {
        __NOP();
    }
}

static void I2cSetScl(GPIO_PinState state)
{
    HAL_GPIO_WritePin(LCD_SCL_PORT, LCD_SCL_PIN, state);
    I2cDelay();
}

static void I2cSetSda(GPIO_PinState state)
{
    HAL_GPIO_WritePin(LCD_SDA_PORT, LCD_SDA_PIN, state);
    I2cDelay();
}

static bool TickReached(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

static void I2cStart(void)
{
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);
    I2cSetSda(GPIO_PIN_RESET);
    I2cSetScl(GPIO_PIN_RESET);
}

static void I2cStop(void)
{
    I2cSetSda(GPIO_PIN_RESET);
    I2cSetScl(GPIO_PIN_SET);
    I2cSetSda(GPIO_PIN_SET);
}

static bool I2cRecoverBus(void)
{
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);

    if (HAL_GPIO_ReadPin(LCD_SCL_PORT, LCD_SCL_PIN) == GPIO_PIN_RESET)
    {
        return false;
    }

    for (uint8_t pulse = 0U; pulse < 9U; pulse++)
    {
        if (HAL_GPIO_ReadPin(LCD_SDA_PORT, LCD_SDA_PIN) == GPIO_PIN_SET)
        {
            break;
        }

        I2cSetScl(GPIO_PIN_RESET);
        I2cSetScl(GPIO_PIN_SET);
    }

    I2cStop();
    return HAL_GPIO_ReadPin(LCD_SCL_PORT, LCD_SCL_PIN) == GPIO_PIN_SET &&
           HAL_GPIO_ReadPin(LCD_SDA_PORT, LCD_SDA_PIN) == GPIO_PIN_SET;
}

static bool I2cWriteByte(uint8_t data)
{
    for (uint8_t mask = 0x80U; mask != 0U; mask >>= 1U)
    {
        I2cSetSda((data & mask) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET);
        I2cSetScl(GPIO_PIN_SET);
        I2cSetScl(GPIO_PIN_RESET);
    }

    /* Release SDA and read the receiver's active-low acknowledge bit. */
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);
    bool acknowledged = HAL_GPIO_ReadPin(LCD_SDA_PORT, LCD_SDA_PIN) == GPIO_PIN_RESET;
    I2cSetScl(GPIO_PIN_RESET);
    return acknowledged;
}

static uint8_t I2cReadByte(void)
{
    uint8_t data = 0U;

    I2cSetSda(GPIO_PIN_SET);
    for (uint8_t bit = 0U; bit < 8U; bit++)
    {
        data <<= 1U;
        I2cSetScl(GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(LCD_SDA_PORT, LCD_SDA_PIN) == GPIO_PIN_SET)
        {
            data |= 1U;
        }
        I2cSetScl(GPIO_PIN_RESET);
    }

    /* A single byte is read, so terminate it with a NACK. */
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_RESET);
    return data;
}

static bool LcdWriteExpander(uint8_t value)
{
    uint8_t data = value | LCD_BACKLIGHT;
    I2cStart();
    bool acknowledged = I2cWriteByte((uint8_t)(lcdI2cAddress << 1));
    if (acknowledged)
    {
        acknowledged = I2cWriteByte(data);
    }
    I2cStop();

    if (acknowledged)
    {
        lcdLastOutput = data;
    }
    return acknowledged;
}

static bool LcdReadExpander(uint8_t *value)
{
    if (value == NULL)
    {
        return false;
    }

    I2cStart();
    bool acknowledged =
        I2cWriteByte((uint8_t)((lcdI2cAddress << 1) | 0x01U));
    if (acknowledged)
    {
        *value = I2cReadByte();
    }
    I2cStop();
    return acknowledged;
}

static bool LcdExpanderStateMatches(void)
{
    uint8_t expanderState = 0U;

    return LcdReadExpander(&expanderState) &&
           ((expanderState ^ lcdLastOutput) & LCD_HEALTH_MASK) == 0U;
}

static bool LcdPulseEnable(uint8_t value)
{
    if (!LcdWriteExpander(value | LCD_ENABLE))
    {
        return false;
    }
    HAL_Delay(1);
    if (!LcdWriteExpander(value & (uint8_t)~LCD_ENABLE))
    {
        return false;
    }
    HAL_Delay(1);
    return true;
}

static bool LcdWriteNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t value = (uint8_t)((nibble & 0x0FU) << 4) | mode;
    return LcdWriteExpander(value) && LcdPulseEnable(value);
}

static bool LcdSend(uint8_t value, uint8_t mode)
{
    return LcdWriteNibble((uint8_t)(value >> 4), mode) &&
           LcdWriteNibble(value, mode);
}

static bool LcdCommand(uint8_t command)
{
    if (!LcdSend(command, 0U))
    {
        return false;
    }

    if (command == LCD_COMMAND_CLEAR || command == LCD_COMMAND_HOME)
    {
        HAL_Delay(2);
    }
    return true;
}

static void LcdSetOffline(void)
{
    lcdReady = false;
    nextReconnectTick = HAL_GetTick();
}

static bool LcdPrepareTransfer(void)
{
    if (!lcdReady)
    {
        return false;
    }

    if (!LcdExpanderStateMatches())
    {
        LcdSetOffline();
        return false;
    }
    return true;
}

static bool LcdFindDevice(void)
{
    for (uint8_t address = LCD_I2C_ADDRESS_MIN;
         address <= LCD_I2C_ADDRESS_MAX; address++)
    {
        I2cStart();
        bool acknowledged = I2cWriteByte((uint8_t)(address << 1));
        I2cStop();
        if (acknowledged)
        {
            lcdI2cAddress = address;
            return true;
        }
    }
    return false;
}

static bool LcdInitializeController(void)
{
    HAL_Delay(50);
    if (!LcdWriteNibble(0x03U, 0U))
    {
        return false;
    }
    HAL_Delay(5);
    if (!LcdWriteNibble(0x03U, 0U))
    {
        return false;
    }
    HAL_Delay(1);
    if (!LcdWriteNibble(0x03U, 0U) || !LcdWriteNibble(0x02U, 0U))
    {
        return false;
    }

    return LcdCommand(LCD_COMMAND_FUNCTION) &&
           LcdCommand(LCD_COMMAND_DISPLAY) &&
           LcdCommand(LCD_COMMAND_CLEAR) &&
           LcdCommand(LCD_COMMAND_ENTRY);
}

static bool LcdRestoreCursor(void)
{
    static const uint8_t rowAddress[LCD1602_ROWS] = {0x00U, 0x40U};
    uint8_t row = cursorRow < LCD1602_ROWS ? cursorRow : LCD1602_ROWS - 1U;
    uint8_t column =
        cursorColumn < LCD1602_COLUMNS ? cursorColumn : LCD1602_COLUMNS - 1U;

    return LcdCommand(
        (uint8_t)(LCD_COMMAND_DDRAM | (rowAddress[row] + column)));
}

static bool LcdRenderFrame(void)
{
    static const uint8_t rowAddress[LCD1602_ROWS] = {0x00U, 0x40U};

    for (uint8_t row = 0U; row < LCD1602_ROWS; row++)
    {
        if (!LcdCommand((uint8_t)(LCD_COMMAND_DDRAM | rowAddress[row])))
        {
            return false;
        }

        for (uint8_t column = 0U; column < LCD1602_COLUMNS; column++)
        {
            if (!LcdSend((uint8_t)lcdFrame[row][column], LCD_RS))
            {
                return false;
            }
        }
    }

    return LcdRestoreCursor();
}

static bool LcdTryConnect(void)
{
    lcdReady = false;
    if (!I2cRecoverBus() || !LcdFindDevice() ||
        !LcdInitializeController() || !LcdRenderFrame())
    {
        nextReconnectTick = HAL_GetTick() + LCD_RECONNECT_INTERVAL_MS;
        return false;
    }

    lcdReady = true;
    nextHealthCheckTick = HAL_GetTick() + LCD_HEALTH_INTERVAL_MS;
    return true;
}

bool Lcd1602Init(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    memset(lcdFrame, ' ', sizeof(lcdFrame));
    cursorColumn = 0U;
    cursorRow = 0U;
    lcdReady = false;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpioInit.Pin = LCD_SCL_PIN | LCD_SDA_PIN;
    gpioInit.Mode = GPIO_MODE_OUTPUT_OD;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpioInit);
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);

    return LcdTryConnect();
}

void Lcd1602Update(void)
{
    uint32_t now = HAL_GetTick();

    if (lcdReady)
    {
        if (TickReached(now, nextHealthCheckTick))
        {
            nextHealthCheckTick = now + LCD_HEALTH_INTERVAL_MS;
            if (!LcdExpanderStateMatches())
            {
                LcdSetOffline();
            }
        }
        return;
    }

    if (TickReached(now, nextReconnectTick))
    {
        LcdTryConnect();
    }
}

bool Lcd1602IsReady(void)
{
    return lcdReady;
}

void Lcd1602Clear(void)
{
    memset(lcdFrame, ' ', sizeof(lcdFrame));
    cursorColumn = 0U;
    cursorRow = 0U;

    if (LcdPrepareTransfer() && !LcdCommand(LCD_COMMAND_CLEAR))
    {
        LcdSetOffline();
    }
}

void Lcd1602SetCursor(uint8_t column, uint8_t row)
{
    static const uint8_t rowAddress[LCD1602_ROWS] = {0x00U, 0x40U};

    if (column >= LCD1602_COLUMNS)
    {
        column = LCD1602_COLUMNS - 1U;
    }
    if (row >= LCD1602_ROWS)
    {
        row = LCD1602_ROWS - 1U;
    }

    cursorColumn = column;
    cursorRow = row;

    if (LcdPrepareTransfer() &&
        !LcdCommand((uint8_t)(LCD_COMMAND_DDRAM |
                              (rowAddress[row] + column))))
    {
        LcdSetOffline();
    }
}

void Lcd1602WriteString(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    bool canWrite = LcdPrepareTransfer();

    while (*text != '\0' && cursorRow < LCD1602_ROWS)
    {
        if (*text == '\n' || cursorColumn >= LCD1602_COLUMNS)
        {
            if (cursorRow + 1U >= LCD1602_ROWS)
            {
                break;
            }

            Lcd1602SetCursor(0U, cursorRow + 1U);
            canWrite = lcdReady;
            if (*text == '\n')
            {
                text++;
                continue;
            }
        }

        lcdFrame[cursorRow][cursorColumn] = *text;
        if (canWrite && !LcdSend((uint8_t)*text, LCD_RS))
        {
            LcdSetOffline();
            canWrite = false;
        }
        text++;
        cursorColumn++;
    }
}

void Lcd1602Printf(const char *format, ...)
{
    char buffer[(LCD1602_COLUMNS * LCD1602_ROWS) + 1U];
    va_list args;

    if (format == NULL)
    {
        return;
    }

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Lcd1602Clear();
    Lcd1602WriteString(buffer);
}
