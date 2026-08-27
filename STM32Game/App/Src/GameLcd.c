#include "GameLcd.h"

#include "GLcd1602.h"

#define LCD_RECORD_MAX 9999U

static uint32_t ClampRecordValue(uint32_t value)
{
    return value > LCD_RECORD_MAX ? LCD_RECORD_MAX : value;
}

void GameLcdShowTitleMenu(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("1.GameStart\n2.Record");
    }
}

void GameLcdShowCombo(uint32_t combo)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("Combo : %lu !", (unsigned long)combo);
    }
}

void GameLcdShowMiss(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("Miss...");
    }
}

void GameLcdShowGameOver(void)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("GameOver");
    }
}

void GameLcdShowRecord(uint32_t bestRecord, uint32_t missCount)
{
    if (Lcd1602IsReady())
    {
        Lcd1602Printf("Best record:%04lu\nMiss Count:%04lu",
                      (unsigned long)ClampRecordValue(bestRecord),
                      (unsigned long)ClampRecordValue(missCount));
    }
}
