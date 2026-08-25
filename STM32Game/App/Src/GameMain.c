#include "GameMain.h"
#include "GCheat.h"

static void GameInit(void)
{

}

int GameMain(void)
{
    GameInit();

    G_LOG(INFO, "Game started successfully. \r\n");

    // Main game loop
    while (1)
    {
    }

    return 0; // This line will never be reached
}
