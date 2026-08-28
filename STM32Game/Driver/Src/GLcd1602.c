#include "GLcd1602.h"

#include "stm32f4xx_hal.h"

/* PCF8574T address range is 0x20-0x27. Change this if A0-A2 differ. */
#define LCD_I2C_ADDRESS_MIN 0x20U
#define LCD_I2C_ADDRESS_MAX 0x27U
#define LCD_SCL_PORT        GPIOB
#define LCD_SCL_PIN         GPIO_PIN_8
#define LCD_SDA_PORT        GPIOB
#define LCD_SDA_PIN         GPIO_PIN_9

/* Common PCF8574 LCD backpack mapping: P0=RS, P1=RW, P2=E, P3=BL. */
#define LCD_RS        0x01U
#define LCD_RW        0x02U
#define LCD_ENABLE    0x04U
#define LCD_BACKLIGHT 0x08U

#define LCD_HEALTH_INTERVAL_MS    100U
#define LCD_RECONNECT_INTERVAL_MS 500U
#define LCD_POWER_UP_DELAY_MS      50U

#define LCD_COMMAND_CLEAR    0x01U
#define LCD_COMMAND_ENTRY    0x06U
#define LCD_COMMAND_DISPLAY  0x0CU
#define LCD_COMMAND_FUNCTION 0x28U
#define LCD_COMMAND_DDRAM    0x80U

typedef enum
{
    LCD_STATE_OFFLINE_WAIT = 0,
    LCD_STATE_SCANNING,
    LCD_STATE_INITIALIZING,
    LCD_STATE_READY
} LcdState;

typedef enum
{
    LCD_INIT_NIBBLE = 0,
    LCD_INIT_COMMAND
} LcdInitOperation;

typedef struct
{
    LcdInitOperation operation;
    uint8_t value;
    uint8_t delayAfterMs;
} LcdInitStep;

static const LcdInitStep lcdInitSteps[] =
{
    { LCD_INIT_NIBBLE,  0x03U,                5U },
    { LCD_INIT_NIBBLE,  0x03U,                1U },
    { LCD_INIT_NIBBLE,  0x03U,                1U },
    { LCD_INIT_NIBBLE,  0x02U,                1U },
    { LCD_INIT_COMMAND, LCD_COMMAND_FUNCTION, 1U },
    { LCD_INIT_COMMAND, LCD_COMMAND_DISPLAY,  1U },
    { LCD_INIT_COMMAND, LCD_COMMAND_CLEAR,    2U },
    { LCD_INIT_COMMAND, LCD_COMMAND_ENTRY,    1U }
};

#define LCD_INIT_STEP_COUNT \
    ((uint8_t)(sizeof(lcdInitSteps) / sizeof(lcdInitSteps[0])))

static uint8_t cursorColumn;
static uint8_t cursorRow;
static uint8_t lcdI2cAddress = LCD_I2C_ADDRESS_MAX;
static uint8_t scanAddress;
static uint8_t initStepIndex;
static uint8_t renderColumn;
static uint8_t renderRow;
static char lcdFrame[LCD1602_ROWS][LCD1602_COLUMNS];
static bool lcdReady;
static bool frameDirty;
static bool renderAddressPending;
static bool renderHealthPending;
static LcdState lcdState;
static uint32_t nextActionTick;
static uint32_t nextHealthCheckTick;

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

    I2cSetSda(GPIO_PIN_SET);
    I2cSetScl(GPIO_PIN_SET);
    bool acknowledged =
        HAL_GPIO_ReadPin(LCD_SDA_PORT, LCD_SDA_PIN) == GPIO_PIN_RESET;
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

static bool I2cProbeAddress(uint8_t address)
{
    I2cStart();
    bool acknowledged = I2cWriteByte((uint8_t)(address << 1));
    I2cStop();
    return acknowledged;
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

static bool LcdHealthIsValid(void)
{
    uint8_t expanderState = 0U;

    /* RW and E are always low between transfers. Both become high after a
       PCF8574 power reset, allowing a quick unplug/replug to be detected. */
    return LcdReadExpander(&expanderState) &&
           (expanderState & (LCD_RW | LCD_ENABLE)) == 0U;
}

static bool LcdPulseEnable(uint8_t value)
{
    /* Each PCF8574 transfer already keeps E stable far longer than the LCD's
       pulse requirement, so millisecond HAL_Delay calls are unnecessary. */
    return LcdWriteExpander(value) &&
           LcdWriteExpander(value | LCD_ENABLE) &&
           LcdWriteExpander(value & (uint8_t)~LCD_ENABLE);
}

static bool LcdWriteNibble(uint8_t nibble, uint8_t mode)
{
    uint8_t value = (uint8_t)((nibble & 0x0FU) << 4) | mode;
    return LcdPulseEnable(value);
}

static bool LcdSend(uint8_t value, uint8_t mode)
{
    return LcdWriteNibble((uint8_t)(value >> 4), mode) &&
           LcdWriteNibble(value, mode);
}

static bool LcdCommand(uint8_t command)
{
    return LcdSend(command, 0U);
}

static void LcdRestartRender(bool checkHealthFirst)
{
    frameDirty = true;
    renderRow = 0U;
    renderColumn = 0U;
    renderAddressPending = true;
    renderHealthPending = checkHealthFirst && lcdReady;
}

static void LcdSetOffline(uint32_t now)
{
    lcdReady = false;
    lcdState = LCD_STATE_OFFLINE_WAIT;
    nextActionTick = now + LCD_RECONNECT_INTERVAL_MS;
    renderHealthPending = false;
}

static void LcdBeginScan(uint32_t now)
{
    if (!I2cRecoverBus())
    {
        LcdSetOffline(now);
        return;
    }

    scanAddress = LCD_I2C_ADDRESS_MIN;
    lcdState = LCD_STATE_SCANNING;
}

static void LcdUpdateScan(uint32_t now)
{
    if (I2cProbeAddress(scanAddress))
    {
        lcdI2cAddress = scanAddress;
        initStepIndex = 0U;
        nextActionTick = now + LCD_POWER_UP_DELAY_MS;
        lcdState = LCD_STATE_INITIALIZING;
        return;
    }

    if (scanAddress < LCD_I2C_ADDRESS_MAX)
    {
        scanAddress++;
        return;
    }

    LcdSetOffline(now);
}

static void LcdUpdateInitialization(uint32_t now)
{
    if (!TickReached(now, nextActionTick))
    {
        return;
    }

    if (initStepIndex >= LCD_INIT_STEP_COUNT)
    {
        lcdReady = true;
        lcdState = LCD_STATE_READY;
        nextHealthCheckTick = now + LCD_HEALTH_INTERVAL_MS;
        LcdRestartRender(false);
        return;
    }

    const LcdInitStep *step = &lcdInitSteps[initStepIndex];
    bool succeeded = step->operation == LCD_INIT_NIBBLE
        ? LcdWriteNibble(step->value, 0U)
        : LcdCommand(step->value);

    if (!succeeded)
    {
        LcdSetOffline(now);
        return;
    }

    initStepIndex++;
    nextActionTick = now + step->delayAfterMs;
}

static void LcdUpdateRender(uint32_t now)
{
    static const uint8_t rowAddress[LCD1602_ROWS] = {0x00U, 0x40U};

    if (!frameDirty)
    {
        return;
    }

    if (renderHealthPending)
    {
        renderHealthPending = false;
        nextHealthCheckTick = now + LCD_HEALTH_INTERVAL_MS;
        if (!LcdHealthIsValid())
        {
            LcdSetOffline(now);
            return;
        }
        return;
    }

    if (renderAddressPending)
    {
        if (!LcdCommand(
                (uint8_t)(LCD_COMMAND_DDRAM | rowAddress[renderRow])))
        {
            LcdSetOffline(now);
            return;
        }

        renderAddressPending = false;
        return;
    }

    if (!LcdSend((uint8_t)lcdFrame[renderRow][renderColumn], LCD_RS))
    {
        LcdSetOffline(now);
        return;
    }

    renderColumn++;
    if (renderColumn < LCD1602_COLUMNS)
    {
        return;
    }

    renderColumn = 0U;
    renderRow++;
    if (renderRow >= LCD1602_ROWS)
    {
        frameDirty = false;
        return;
    }

    renderAddressPending = true;
}

static void LcdUpdateReady(uint32_t now)
{
    if (TickReached(now, nextHealthCheckTick))
    {
        nextHealthCheckTick = now + LCD_HEALTH_INTERVAL_MS;
        if (!LcdHealthIsValid())
        {
            LcdSetOffline(now);
            return;
        }
        renderHealthPending = false;
        return;
    }

    LcdUpdateRender(now);
}

void Lcd1602Init(void)
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

    lcdState = LCD_STATE_OFFLINE_WAIT;
    nextActionTick = HAL_GetTick();
    LcdRestartRender(false);
}

void Lcd1602Update(void)
{
    uint32_t now = HAL_GetTick();

    switch (lcdState)
    {
        case LCD_STATE_OFFLINE_WAIT:
            if (TickReached(now, nextActionTick))
            {
                LcdBeginScan(now);
            }
            break;

        case LCD_STATE_SCANNING:
            LcdUpdateScan(now);
            break;

        case LCD_STATE_INITIALIZING:
            LcdUpdateInitialization(now);
            break;

        case LCD_STATE_READY:
            LcdUpdateReady(now);
            break;

        default:
            LcdSetOffline(now);
            break;
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
    LcdRestartRender(lcdReady);
}

void Lcd1602SetCursor(uint8_t column, uint8_t row)
{
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
}

void Lcd1602WriteString(const char *text)
{
    bool changed = false;

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

            cursorColumn = 0U;
            cursorRow++;
            if (*text == '\n')
            {
                text++;
                continue;
            }
        }

        if (lcdFrame[cursorRow][cursorColumn] != *text)
        {
            lcdFrame[cursorRow][cursorColumn] = *text;
            changed = true;
        }
        text++;
        cursorColumn++;
    }

    if (changed)
    {
        LcdRestartRender(lcdReady);
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
