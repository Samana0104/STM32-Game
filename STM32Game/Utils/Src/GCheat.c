#include "GCheat.h"
#include "GUsart.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include <stdarg.h>
#include <stdio.h>

void ParserCommand(const char * cmd)
{

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

    (void)UartWriteAsync((const uint8_t *)message, (uint16_t)totalLength);
}
