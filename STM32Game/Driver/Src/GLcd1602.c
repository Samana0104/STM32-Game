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
#define LCD_ENABLE            0x04U
#define LCD_BACKLIGHT         0x08U

#define LCD_COMMAND_CLEAR     0x01U
#define LCD_COMMAND_HOME      0x02U
#define LCD_COMMAND_ENTRY     0x06U
#define LCD_COMMAND_DISPLAY   0x0CU
#define LCD_COMMAND_FUNCTION  0x28U
#define LCD_COMMAND_DDRAM     0x80U

static uint8_t cursorColumn;
static uint8_t cursorRow;
static uint8_t lcdI2cAddress = LCD_I2C_ADDRESS_MAX;
static bool lcdReady;

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

static bool LcdWriteExpander(uint8_t value)
{
    uint8_t data = value | LCD_BACKLIGHT;
    I2cStart();
    bool acknowledged = I2cWriteByte((uint8_t)(lcdI2cAddress << 1));
    acknowledged = I2cWriteByte(data) && acknowledged;
    I2cStop();
    return acknowledged;
}

static void LcdPulseEnable(uint8_t value)
{
    LcdWriteExpander(value | LCD_ENABLE);
    HAL_Delay(1);
    LcdWriteExpander(value & (uint8_t)~LCD_ENABLE);
    HAL_Delay(1);
}

static void LcdWriteNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t value = (uint8_t)((nibble & 0x0FU) << 4) | mode;
    LcdWriteExpander(value);
    LcdPulseEnable(value);
}

static void LcdSend(uint8_t value, uint8_t mode)
{
    LcdWriteNibble((uint8_t)(value >> 4), mode);
    LcdWriteNibble(value, mode);
}

static void LcdCommand(uint8_t command)
{
    LcdSend(command, 0U);

    if (command == LCD_COMMAND_CLEAR || command == LCD_COMMAND_HOME)
    {
        HAL_Delay(2);
    }
}

bool Lcd1602Init(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    lcdReady = false;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpioInit.Pin = LCD_SCL_PIN | LCD_SDA_PIN;
    gpioInit.Mode = GPIO_MODE_OUTPUT_OD;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpioInit);
    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);

    bool deviceReady = false;
    for (uint8_t address = LCD_I2C_ADDRESS_MIN;
         address <= LCD_I2C_ADDRESS_MAX; address++)
    {
        I2cStart();
        deviceReady = I2cWriteByte((uint8_t)(address << 1));
        I2cStop();
        if (deviceReady)
        {
            lcdI2cAddress = address;
            break;
        }
    }
    if (!deviceReady)
    {
        return false;
    }

    HAL_Delay(50);
    LcdWriteNibble(0x03U, 0U);
    HAL_Delay(5);
    LcdWriteNibble(0x03U, 0U);
    HAL_Delay(1);
    LcdWriteNibble(0x03U, 0U);
    LcdWriteNibble(0x02U, 0U);

    LcdCommand(LCD_COMMAND_FUNCTION); /* 4-bit, 2-line, 5x8 font */
    LcdCommand(LCD_COMMAND_DISPLAY);  /* display on, cursor off */
    LcdCommand(LCD_COMMAND_CLEAR);
    LcdCommand(LCD_COMMAND_ENTRY);    /* cursor moves to the right */

    cursorColumn = 0U;
    cursorRow = 0U;
    lcdReady = true;
    return true;
}

bool Lcd1602IsReady(void)
{
    return lcdReady;
}

void Lcd1602Clear(void)
{
    LcdCommand(LCD_COMMAND_CLEAR);
    cursorColumn = 0U;
    cursorRow = 0U;
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

    LcdCommand((uint8_t)(LCD_COMMAND_DDRAM | (rowAddress[row] + column)));
    cursorColumn = column;
    cursorRow = row;
}

void Lcd1602WriteString(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    while (*text != '\0' && cursorRow < LCD1602_ROWS)
    {
        if (*text == '\n' || cursorColumn >= LCD1602_COLUMNS)
        {
            if (cursorRow + 1U >= LCD1602_ROWS)
            {
                break;
            }

            Lcd1602SetCursor(0U, cursorRow + 1U);
            if (*text == '\n')
            {
                text++;
                continue;
            }
        }

        LcdSend((uint8_t)*text++, LCD_RS);
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
