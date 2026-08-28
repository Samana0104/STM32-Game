#include "GameLcd.h"

#include "GLcd1602.h"

#define LCD_RECORD_MAX 9999U

static uint32_t ClampRecordValue(uint32_t value)
{
    return value > LCD_RECORD_MAX ? LCD_RECORD_MAX : value;
}

void GameLcdShowTitleMenu(bool recordSelected)
{
    Lcd1602Printf(recordSelected
                      ? " 1.GameStart\n>2.Record"
                      : ">1.GameStart\n 2.Record");
}

void GameLcdShowCountdown(uint8_t stage, const char *text)
{
    if (text != NULL)
    {
        Lcd1602Printf("STAGE %u\n%s", (unsigned int)stage, text);
    }
}

void GameLcdShowCombo(uint32_t combo)
{
    Lcd1602Printf("Combo : %lu !", (unsigned long)combo);
}

void GameLcdShowMiss(void)
{
    Lcd1602Printf("Miss...");
}

void GameLcdShowGameOver(void)
{
    Lcd1602Printf("GameOver");
}

void GameLcdShowRecord(uint32_t bestRecord, uint32_t missCount)
{
    Lcd1602Printf("Best record:%04lu\nMiss Count:%04lu",
                  (unsigned long)ClampRecordValue(bestRecord),
                  (unsigned long)ClampRecordValue(missCount));
}
