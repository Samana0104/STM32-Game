#include "GCheat.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include <stdarg.h>
#include <stdio.h>

void ParserCommand(const char * cmd)
{

}

void CommandLog(LogLevel level, const char *args, ...)
{
    (void)level;

    va_list ap;
    va_start(ap, args);
    vprintf(args, ap);
    va_end(ap);
}
