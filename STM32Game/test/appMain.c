#include "appMain.h"

#include "myMole.h"

void appInit(void)
{
  moleInit();
}

void appMain(void)
{
  while (1)
  {
    moleRun();
    osDelay(1U);
  }
}

void StartTaskCLI(void *argument)
{
  (void)argument;

  appInit();
  appMain();
}
