#include "GFnd.h"

/* 자릿수 선택 핀 */
#define DIGIT_PORT     GPIOC
#define DIG_SINGLE_PIN GPIO_PIN_6 /* 1자리 FND COM */
#define DIG1_PIN       GPIO_PIN_2 /* 4자리 FND 1000의 자리 */
#define DIG2_PIN       GPIO_PIN_3 /* 4자리 FND 100의 자리 */
#define DIG3_PIN       GPIO_PIN_4 /* 4자리 FND 10의 자리 */
#define DIG4_PIN       GPIO_PIN_5 /* 4자리 FND 1의 자리 */

/* FND 제어용 74HC595 핀 */
#define FND_DATA_PORT  GPIOB
#define FND_DATA_PIN   GPIO_PIN_0
#define FND_CLOCK_PORT GPIOB
#define FND_CLOCK_PIN  GPIO_PIN_2
#define FND_LATCH_PORT GPIOB
#define FND_LATCH_PIN  GPIO_PIN_1

/* Common Cathode 기준 폰트 테이블 */
static const uint8_t segmentTable[10] = {
    0x3F, /* 0 (A B C D E F) */
    0x06, /* 1 (B C) */
    0x5B, /* 2 (A B D E G) */
    0x4F, /* 3 (A B C D G) */
    0x66, /* 4 (B C F G) */
    0x6D, /* 5 (A C D F G) */
    0x7D, /* 6 (A C D E F G) */
    0x07, /* 7 (A B C) */
    0x7F, /* 8 (A B C D E F G) */
    0x6F  /* 9 (A B C D F G) */
};

static uint8_t displayDigits[FND_TOTAL_DIGITS] = {0, 0, 0, 0, 0};
static uint8_t currentDigitIndex = 0;

static void ShiftOutSegmentData(uint8_t segByte)
{
    /* Q7(DP)부터 Q0(A)까지 MSB First 전송 */
    for (int i = 7; i >= 0; i--)
    {
        HAL_GPIO_WritePin(FND_CLOCK_PORT, FND_CLOCK_PIN, GPIO_PIN_RESET);

        if ((segByte >> i) & 0x01)
        {
            HAL_GPIO_WritePin(FND_DATA_PORT, FND_DATA_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(FND_DATA_PORT, FND_DATA_PIN, GPIO_PIN_RESET);
        }

        HAL_GPIO_WritePin(FND_CLOCK_PORT, FND_CLOCK_PIN, GPIO_PIN_SET);
    }

    /* 래치 클록 발생 (출력 갱신) */
    HAL_GPIO_WritePin(FND_LATCH_PORT, FND_LATCH_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FND_LATCH_PORT, FND_LATCH_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(FND_LATCH_PORT, FND_LATCH_PIN, GPIO_PIN_RESET);
}

static void SelectDigit(uint8_t index)
{
    HAL_GPIO_WritePin(DIGIT_PORT, DIG_SINGLE_PIN, (index == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIGIT_PORT, DIG1_PIN,       (index == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIGIT_PORT, DIG2_PIN,       (index == 2) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIGIT_PORT, DIG3_PIN,       (index == 3) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIGIT_PORT, DIG4_PIN,       (index == 4) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void FndInit(void)
{
    ClearFnd();
    SelectDigit(0xFF);
}

void ClearFnd(void)
{
    ShiftOutSegmentData(0x00);
}

void SetFndSingleDigit(uint8_t digit)
{
    if (digit > 9)
    {
        digit = 9;
    }
    displayDigits[0] = digit;
}

void SetFnd4DigitNumber(uint16_t number)
{
    if (number > FND_MAX_4DIGIT)
    {
        number = FND_MAX_4DIGIT;
    }

    displayDigits[1] = (number / 1000) % 10;
    displayDigits[2] = (number / 100) % 10;
    displayDigits[3] = (number / 10) % 10;
    displayDigits[4] = number % 10;
}

void UpdateFnd(void)
{
    /* 잔상 방지를 위해 자릿수 전환 전 소등 */
    SelectDigit(0xFF);

    uint8_t digitVal = displayDigits[currentDigitIndex];
    ShiftOutSegmentData(segmentTable[digitVal]);

    SelectDigit(currentDigitIndex);

    currentDigitIndex = (currentDigitIndex + 1) % FND_TOTAL_DIGITS;
}