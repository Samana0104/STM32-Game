#pragma once

#include "main.h"
#include <stdbool.h>


#define BUTTON_COUNT 3


/*
 * 버튼 ID
 */
typedef enum
{
    BUTTON_1 = 0,
    BUTTON_2,
    BUTTON_3

} ButtonId;


/*
 * 버튼 상태
 */
typedef enum
{
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED

} ButtonState;


/*
 * 버튼 상태 업데이트
 */
void UpdateButtonState(void);


/*
 * 특정 버튼 상태 반환
 */
ButtonState GetButtonState(ButtonId id);


/*
 * 버튼이 눌려있는지 확인
 */
bool IsButtonPressed(ButtonId id);


/*
 * 버튼이 떼져있는지 확인
 */
bool IsButtonReleased(ButtonId id);