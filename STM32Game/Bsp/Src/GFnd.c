#include "GFnd.h"

#define DIGIT_PORT     GPIOC
#define DIG_SINGLE_PIN GPIO_PIN_6 /* 1자리 FND COM */
#define DIG1_PIN       GPIO_PIN_2 /* 4자리 FND 1000의 자리 */
#define DIG2_PIN       GPIO_PIN_3 /* 4자리 FND 100의 자리 */
#define DIG3_PIN       GPIO_PIN_4 /* 4자리 FND 10의 자리 */
#define DIG4_PIN       GPIO_PIN_5 /* 4자리 FND 1의 자리 */

static const uint8_t segmentTable[10] = {
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F  /* 9 */
};

static uint8_t displayDigits[FND_TOTAL_DIGITS] = {0, 0, 0, 0, 0};
static uint8_t currentDigitIndex = 0;

static void WriteSegments(uint8_t byte)
{
    /* A: PB0, B: PA6, C: PA7, D: PB1 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  (byte & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6,  (byte & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7,  (byte & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,  (byte & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    /* E: PB2, F: PB6, G: PB7, DP: PB12 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2,  (byte & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6,  (byte & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,  (byte & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (byte & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
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
    WriteSegments(0x00);
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
    ClearFnd();
    SelectDigit(currentDigitIndex);

    uint8_t digitVal = displayDigits[currentDigitIndex];
    WriteSegments(segmentTable[digitVal]);

    currentDigitIndex = (currentDigitIndex + 1) % FND_TOTAL_DIGITS;
}