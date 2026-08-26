#ifndef G_JOYSTICK_H
#define G_JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    JOYSTICK_CENTER = 0,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT,
    JOYSTICK_UP,
    JOYSTICK_DOWN
} JoystickDirection;

/* 조이스틱 GPIO와 ADC1을 초기화한다. */
void GJoystickInit(void);

/* 두 축의 ADC 값을 읽고 현재 방향과 이동 이벤트를 갱신한다. */
void UpdateJoystickState(void);

/* 현재 조이스틱 방향을 반환한다. */
JoystickDirection GetJoystickDirection(void);

/* 조이스틱이 중앙을 벗어나 있는지 반환한다. */
bool IsJoystickMoved(void);

/* 직전 업데이트에서 중앙으로부터 움직이기 시작했는지 반환한다. */
bool WasJoystickMoved(void);

/* 가장 최근에 측정한 12-bit ADC 원시 값을 반환한다. */
uint16_t GetJoystickX(void);
uint16_t GetJoystickY(void);

#endif /* G_JOYSTICK_H */
