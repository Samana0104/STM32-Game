#include "GameLcd.h"

#include "GLcd1602.h"

#define LCD_RECORD_MAX 9999U
#define LCD_LIFE_MAX     10U

static uint32_t ClampRecordValue(uint32_t value)
{
    return value > LCD_RECORD_MAX ? LCD_RECORD_MAX : value;
}

static void FormatLifeSymbols(uint8_t life,
                              char symbols[LCD_LIFE_MAX + 1U])
{
    uint8_t symbolCount = life > LCD_LIFE_MAX ? LCD_LIFE_MAX : life;

    memset(symbols, '$', symbolCount);
    symbols[symbolCount] = '\0';
}

static const char *GetComboRankText(GameComboRank rank)
{
    switch (rank)
    {
        case GAME_COMBO_RANK_GOOD:
            return "Good";

        case GAME_COMBO_RANK_NICE:
            return "Nice";

        case GAME_COMBO_RANK_GREAT:
            return "Great";

        case GAME_COMBO_RANK_PERFECT:
            return "Perfect";

        case GAME_COMBO_RANK_NONE:
        default:
            return "";
    }
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

void GameLcdShowCombo(uint32_t combo, uint8_t life,
                      GameComboRank announcedRank, uint32_t awardedScore)
{
    char lifeSymbols[LCD_LIFE_MAX + 1U];

    FormatLifeSymbols(life, lifeSymbols);
    if (announcedRank != GAME_COMBO_RANK_NONE)
    {
        Lcd1602Printf("%s +%lu\nLife: %s",
                      GetComboRankText(announcedRank),
                      (unsigned long)awardedScore,
                      lifeSymbols);
        return;
    }

    Lcd1602Printf("%lu Combo\nLife: %s",
                  (unsigned long)combo,
                  lifeSymbols);
}

void GameLcdShowMiss(uint8_t life, uint32_t scorePenalty)
{
    char lifeSymbols[LCD_LIFE_MAX + 1U];

    FormatLifeSymbols(life, lifeSymbols);
    Lcd1602Printf("Miss -%lu\nLife: %s",
                  (unsigned long)scorePenalty,
                  lifeSymbols);
}

void GameLcdShowGameOver(void)
{
    Lcd1602Printf("GameOver");
}

void GameLcdShowResult(uint32_t score, uint32_t maxCombo)
{
    Lcd1602Printf("Your Score:%lu\nMax Combo:%lu",
                  (unsigned long)ClampRecordValue(score),
                  (unsigned long)ClampRecordValue(maxCombo));
}

void GameLcdShowRecordPage(uint32_t firstRank, uint32_t firstScore,
                           uint32_t secondRank, uint32_t secondScore)
{
    Lcd1602Printf("%lu. %lu\n%lu. %lu",
                  (unsigned long)firstRank,
                  (unsigned long)ClampRecordValue(firstScore),
                  (unsigned long)secondRank,
                  (unsigned long)ClampRecordValue(secondScore));
}
