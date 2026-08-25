#include "appMain.h"
<<<<<<< HEAD

#include "cmsis_os.h"
=======
>>>>>>> 3751dfc1c22316f8a9418c86e1671eda321d99f9
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
<<<<<<< HEAD
    osDelay(1U);
  }
}

void StartTaskCLI(void *argument)
{
  (void)argument;

  appInit();
  appMain();
}
=======
  }
}
>>>>>>> 3751dfc1c22316f8a9418c86e1671eda321d99f9
