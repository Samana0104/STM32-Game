#include "GCheat.h"
#include "GLed.h"
#include "GUsart.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"

#define CHEAT_MAX_ARGS 4U
#define CHEAT_REPORT_INTERVAL_MS 1000U

typedef void (*CommandHandler)(int argc, char *argv[]);

typedef struct
{
    const char *name;
    const char *usage;
    CommandHandler handler;
} CommandEntry;

static uint32_t lastFrameTick;
static uint32_t currentFpsMilli;
static bool fpsMonitoring;
static uint32_t fpsLastReport;

static bool gpioMonitoring;
static GPIO_TypeDef *monitoredPort;
static char monitoredPortName;
static uint32_t monitoredPin;
static uint32_t gpioLastReport;

static void CommandHelp(int argc, char *argv[]);
static void CommandWrite(int argc, char *argv[]);
static void CommandRead(int argc, char *argv[]);
static void CommandLed(int argc, char *argv[]);
static void CommandFps(int argc, char *argv[]);
static void CommandStop(int argc, char *argv[]);

static const CommandEntry commandTable[] =
{
    { "help",  "help",                    CommandHelp  },
    { "write", "write <A|B|C|H> <0-15> <0|1>", CommandWrite },
    { "read",  "read <A|B|C|H> <0-15>",       CommandRead  },
    { "led",   "led <1-7|all> <0|1>",           CommandLed   },
    { "fps",   "fps",                     CommandFps   },
    { "stop",  "stop",                    CommandStop  }
};

static void ReportGpio(void)
{
    const GPIO_PinState state = HAL_GPIO_ReadPin(
        monitoredPort, (uint16_t)(1UL << monitoredPin));

    G_LOG(INFO, "GPIO %c%lu -> %u\r\n", monitoredPortName,
          (unsigned long)monitoredPin,
          (unsigned int)(state == GPIO_PIN_SET));
}

static void ReportFps(void)
{
    G_LOG(INFO, "FPS: %lu.%03lu\r\n",
          (unsigned long)(currentFpsMilli / 1000U),
          (unsigned long)(currentFpsMilli % 1000U));
}

static GPIO_TypeDef *ParsePort(const char *text)
{
    if ((text == NULL) || (text[0] == '\0') || (text[1] != '\0'))
    {
        return NULL;
    }

    switch (toupper((unsigned char)text[0]))
    {
        case 'A': return GPIOA;
        case 'B': return GPIOB;
        case 'C': return GPIOC;
        case 'H': return GPIOH;
        default:  return NULL;
    }
}

static bool ParseNumber(const char *text, uint32_t max, uint32_t *value)
{
    char *end;
    const unsigned long parsed = strtoul(text, &end, 10);

    if ((text == end) || (*end != '\0') || (parsed > max))
    {
        return false;
    }

    *value = (uint32_t)parsed;
    return true;
}

static void CommandHelp(int argc, char *argv[])
{
    G_LOG(INFO, "Commands:\r\n");
    for (size_t i = 0U; i < (sizeof(commandTable) / sizeof(commandTable[0])); ++i)
    {
        G_LOG(INFO, "%s\r\n", commandTable[i].usage);
    }
    G_LOG(WARNING, "write only works on pins configured as GPIO output.\r\n");
}

static void CommandWrite(int argc, char *argv[])
{
    uint32_t pin;
    uint32_t value;

    if (argc != 4)
    {
        G_LOG(WARNING, "Usage: write <A|B|C|H> <0-15> <0|1>\r\n");
        return;
    }

    GPIO_TypeDef *const port = ParsePort(argv[1]);
    if ((port == NULL) || !ParseNumber(argv[2], 15U, &pin) ||
        !ParseNumber(argv[3], 1U, &value))
    {
        G_LOG(WARNING, "Invalid GPIO port, pin, or value.\r\n");
        return;
    }

    /* MODER=01인 일반 GPIO 출력 핀만 허용하여 UART/SWD 핀을 보호한다. */
    if (((port->MODER >> (pin * 2U)) & 0x3U) != 0x1U)
    {
        G_LOG(DANGER, "GPIO %s%lu is not configured as output.\r\n",
              argv[1], (unsigned long)pin);
        return;
    }

    HAL_GPIO_WritePin(port, (uint16_t)(1UL << pin),
                      value ? GPIO_PIN_SET : GPIO_PIN_RESET);

    G_LOG(INFO, "GPIO %s%lu <- %lu\r\n", argv[1],
          (unsigned long)pin, (unsigned long)value);
}

static void CommandRead(int argc, char *argv[])
{
    uint32_t pin;

    if (argc != 3)
    {
        G_LOG(WARNING, "Usage: read <A|B|C|H> <0-15>\r\n");
        return;
    }

    GPIO_TypeDef *const port = ParsePort(argv[1]);
    if ((port == NULL) || !ParseNumber(argv[2], 15U, &pin))
    {
        G_LOG(WARNING, "Invalid GPIO port or pin.\r\n");
        return;
    }

    monitoredPort = port;
    monitoredPortName = (char)toupper((unsigned char)argv[1][0]);
    monitoredPin = pin;
    gpioMonitoring = true;
    gpioLastReport = HAL_GetTick();

    ReportGpio();
    G_LOG(INFO, "GPIO monitoring started (every 3 seconds).\r\n");
}

static void CommandLed(int argc, char *argv[])
{
    uint32_t value;

    if (argc != 3)
    {
        G_LOG(WARNING, "Usage: led <1-7|all> <0|1>\r\n");
        return;
    }

    if (!ParseNumber(argv[2], 1U, &value))
    {
        G_LOG(WARNING, "LED state must be 0 (off) or 1 (on).\r\n");
        return;
    }

    const LedState state = value ? LED_ON : LED_OFF;

    if (strcmp(argv[1], "all") == 0)
    {
        for (LedId id = LED_ID_1; id < LED_MAX; ++id)
        {
            SetLedState(id, state);
        }

        G_LOG(INFO, "All LEDs <- %lu\r\n", (unsigned long)value);
        return;
    }

    uint32_t ledNumber;
    if (!ParseNumber(argv[1], (uint32_t)LED_MAX, &ledNumber) ||
        (ledNumber == 0U))
    {
        G_LOG(WARNING, "LED number must be 1-7 or 'all'.\r\n");
        return;
    }

    SetLedState((LedId)(ledNumber - 1U), state);
    G_LOG(INFO, "LED %lu <- %lu\r\n", (unsigned long)ledNumber,
          (unsigned long)value);
}

static void CommandFps(int argc, char *argv[])
{
    (void)argv;

    if (argc != 1)
    {
        G_LOG(WARNING, "Usage: fps\r\n");
        return;
    }

    fpsMonitoring = true;
    fpsLastReport = HAL_GetTick();

    ReportFps();
    G_LOG(INFO, "FPS monitoring started (every 3 seconds).\r\n");
}

static void CommandStop(int argc, char *argv[])
{
    (void)argv;

    if (argc != 1)
    {
        G_LOG(WARNING, "Usage: stop\r\n");
        return;
    }

    gpioMonitoring = false;
    fpsMonitoring = false;
    G_LOG(INFO, "All monitoring stopped.\r\n");
}

void ParseCommand(const char *cmd)
{
    char input[UART_COMMAND_SIZE];
    char *argv[CHEAT_MAX_ARGS];
    int argc = 0;

    if (cmd == NULL)
    {
        return;
    }

    strncpy(input, cmd, sizeof(input) - 1U);
    input[sizeof(input) - 1U] = '\0';

    char *cursor = input;
    while (*cursor != '\0')
    {
        while (isspace((unsigned char)*cursor))
        {
            ++cursor;
        }

        if (*cursor == '\0')
        {
            break;
        }

        if (argc >= (int)CHEAT_MAX_ARGS)
        {
            G_LOG(WARNING, "Too many command arguments.\r\n");
            return;
        }

        argv[argc++] = cursor;
        while ((*cursor != '\0') && !isspace((unsigned char)*cursor))
        {
            ++cursor;
        }

        if (*cursor != '\0')
        {
            *cursor++ = '\0';
        }
    }

    if (argc == 0)
    {
        return;
    }

    for (size_t i = 0U; i < (sizeof(commandTable) / sizeof(commandTable[0])); ++i)
    {
        if (strcmp(argv[0], commandTable[i].name) == 0)
        {
            commandTable[i].handler(argc, argv);
            return;
        }
    }

    G_LOG(WARNING, "Unknown command: %s (type 'help')\r\n", argv[0]);
}

void CheatUpdate(void)
{
    char command[UART_COMMAND_SIZE];

    if (UartReadCommand(command, sizeof(command)))
    {
        ParseCommand(command);
    }

    const uint32_t now = HAL_GetTick();

    if (gpioMonitoring &&
        ((now - gpioLastReport) >= CHEAT_REPORT_INTERVAL_MS))
    {
        gpioLastReport = now;
        ReportGpio();
    }

    if (fpsMonitoring &&
        ((now - fpsLastReport) >= CHEAT_REPORT_INTERVAL_MS))
    {
        fpsLastReport = now;
        ReportFps();
    }
}

void CheatInit(void)
{
    lastFrameTick = HAL_GetTick();
    currentFpsMilli = 0U;
    fpsMonitoring = false;
    gpioMonitoring = false;
}

void CheatFrameTick(void)
{
    const uint32_t now = HAL_GetTick();
    const uint32_t frameTimeMs = now - lastFrameTick;

    if (frameTimeMs > 0U)
    {
        currentFpsMilli = 1000000U / frameTimeMs;
    }

    lastFrameTick = now;
}

void CommandLog(LogLevel level, const char *args, ...)
{
    static const char *const prefix[] =
    {
        "[INFO] ",
        "[WARNING] ",
        "[DANGER] "
    };
    char message[192];
    int length = 0;

    if ((unsigned int)level < (sizeof(prefix) / sizeof(prefix[0])))
    {
        length = snprintf(message, sizeof(message), "%s", prefix[level]);
    }

    if ((length < 0) || ((size_t)length >= sizeof(message)))
    {
        return;
    }

    va_list ap;
    va_start(ap, args);
    const int written = vsnprintf(&message[length], sizeof(message) - (size_t)length,
                                  args, ap);
    va_end(ap);

    if (written < 0)
    {
        return;
    }

    size_t totalLength = (size_t)length + (size_t)written;
    if (totalLength >= sizeof(message))
    {
        totalLength = sizeof(message) - 1U;
    }

    UartWriteAsync((const uint8_t *)message, (uint16_t)totalLength);
}
