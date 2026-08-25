#include "GameMain.h"
#include "GCheat.h"
#include "GLed.h"
#include "GUsart.h"

static void GameInit(void)
{
    GledInit();
}

int GameMain(void)
{
    char command[UART_COMMAND_SIZE];

    GameInit();
    G_LOG(INFO, "Game started successfully. \r\n");

    while (1)
    {
        if (UartReadCommand(command, sizeof(command)))
        {
            ParseCommand(command);
        }

        SetLedState(LED_ID_1, LED_ON);
        HAL_Delay(500);

        SetLedState(LED_ID_1, LED_OFF);
        HAL_Delay(500);
    }

    return 0;
}
