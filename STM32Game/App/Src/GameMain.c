#include "GameMain.h"
#include "GCheat.h"
#include "GLed.h"

static void GameInit(void)
{
    GledInit();
}

int GameMain(void)
{
    GameInit();

    G_LOG(INFO, "Game started successfully. \r\n");

    // Main game loop
    while (1)
    {
        // 1번 LED 켜기
        SetLedState(LED_ID_1, LED_ON);
        HAL_Delay(500); // 0.5초 대기
        
        // 1번 LED 끄기
        SetLedState(LED_ID_1, LED_OFF);
        HAL_Delay(500); // 0.5초 대기
    }

    return 0; // This line will never be reached
}
