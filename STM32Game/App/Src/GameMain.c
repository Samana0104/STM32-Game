#include "GameMain.h"
#include "GCheat.h"
#include "GLed.h"
#include "GUsart.h"

static void GameInit(void)
{
    GledInit();
    UartInit();

#ifndef NDEBUG
    CheatInit();
#endif

}

int GameMain(void)
{
    GameInit();
    G_LOG(INFO, "Game started successfully. \r\n");

    while (1)
    {

        // SetLedState(LED_ID_1, LED_ON);
        // HAL_Delay(500);

        // SetLedState(LED_ID_1, LED_OFF);
        // HAL_Delay(500);



#ifndef NDEBUG
        CheatUpdate();
        /* 한 프레임의 모든 작업이 끝난 시점에서 프레임 시간을 측정한다. */
        CheatFrameTick();
#endif
    }

    G_LOG(INFO, "GameMain exited. \r\n");
    return 0;
}
