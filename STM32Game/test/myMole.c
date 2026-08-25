#include "myMole.h"

#include "main.h"
#include "mySn74hc595.h"

/*
 * 두더지 게임 동작
 * 1. 시작 버튼을 눌렀다 떼면 게임이 시작/종료된다.
 * 2. 게임 시작 시 LED 하나가 임의로 켜진다.
 * 3. 켜진 LED에 대응하는 버튼을 누르면 점수가 증가한다.
 * 4. 정답 처리 후 직전과 다른 LED가 임의로 켜진다.
 *
 * 버튼은 풀업 입력이므로 평상시 SET(1), 누르면 RESET(0)이다.
 * LED는 SN74HC595의 Q0~Q2 출력이 HIGH이면 켜지는 active-high 방식이다.
 */
#define MOLE_COUNT              3U
#define MOLE_BUTTON_COUNT       4U
#define MOLE_START_BUTTON       3U
#define MOLE_NO_ACTIVE_LED      0xFFU
#define MOLE_DEBOUNCE_TIME_MS   20U

typedef struct
{
  /* 버튼이 연결된 GPIO Peripheral과 해당 핀 번호 */
  GPIO_TypeDef *port;
  uint16_t pin;
} MoleGpio;

typedef struct
{
  /* 20 ms 동안 유지되어 실제 상태로 확정된 값 */
  GPIO_PinState stable_state;
  /* 직전에 읽은 원시 GPIO 값 */
  GPIO_PinState previous_raw_state;
  /* 원시 GPIO 값이 마지막으로 변한 시각 */
  uint32_t changed_tick;
} MoleButton;

/* 게임 버튼 3개와 시작 버튼 1개: NUCLEO Arduino digital header 사용 */
static const MoleGpio mole_buttons[MOLE_BUTTON_COUNT] =
{
  {GPIOA, GPIO_PIN_8},   /* Button 1: PA8,  D7 */
  {GPIOB, GPIO_PIN_10},  /* Button 2: PB10, D6 */
  {GPIOB, GPIO_PIN_4},   /* Button 3: PB4,  D5 */
  {GPIOA, GPIO_PIN_10}   /* Start:    PA10, D2 */
};

static MoleButton button_states[MOLE_BUTTON_COUNT];
static uint32_t score;
static uint32_t random_state;
static uint8_t active_led;
static bool game_running;

volatile uint8_t mole_button_raw[MOLE_BUTTON_COUNT];
volatile uint8_t mole_button_stable[MOLE_BUTTON_COUNT];

/* Q0~Q2를 모두 LOW로 만들어 세 LED를 끈다. */
static void moleAllLedOff(void)
{
  sn74hc595Write(0U);
}

static void moleLedOn(uint8_t led_number)
{
  uint8_t led_value = 0U;

  if (led_number < MOLE_COUNT)
  {
    /* Q0=LED 1, Q1=LED 2, Q2=LED 3: 항상 하나의 LED만 켠다. */
    led_value = (uint8_t)(1U << led_number);
  }

  sn74hc595Write(led_value);
}

/* xorshift 방식으로 다음 의사 난수(pseudorandom number)를 만든다. */
static uint32_t moleRandom(void)
{
  random_state ^= random_state << 13U;
  random_state ^= random_state >> 17U;
  random_state ^= random_state << 5U;

  return random_state;
}

static void moleSelectFirstLed(void)
{
  /* 게임을 시작할 때는 세 LED 모두 같은 확률로 선택한다. */
  active_led = (uint8_t)(moleRandom() % MOLE_COUNT);
  moleLedOn(active_led);
}

static void moleSelectNextLed(void)
{
  uint8_t next_led;

  /* 직전 LED를 제외한 두 LED 중 하나를 선택해 두더지가 반드시 이동하게 한다. */
  next_led = (uint8_t)(moleRandom() % (MOLE_COUNT - 1U));
  if (next_led >= active_led)
  {
    next_led++;
  }

  active_led = next_led;
  moleLedOn(active_led);
}

static bool moleButtonChangedTo(uint8_t button_number,
                                GPIO_PinState target_state,
                                uint32_t current_tick)
{
  MoleButton *button = &button_states[button_number];
  GPIO_PinState raw_state = HAL_GPIO_ReadPin(mole_buttons[button_number].port,
                                             mole_buttons[button_number].pin);

  mole_button_raw[button_number] = (uint8_t)raw_state;

  /* 원시 값이 바뀔 때마다 디바운싱 시간을 처음부터 다시 측정한다. */
  if (raw_state != button->previous_raw_state)
  {
    button->previous_raw_state = raw_state;
    button->changed_tick = current_tick;
  }

  /* 새 상태가 20 ms 이상 유지됐을 때만 실제 상태 변화로 확정한다. */
  if ((raw_state != button->stable_state)
      && ((current_tick - button->changed_tick) >= MOLE_DEBOUNCE_TIME_MS))
  {
    button->stable_state = raw_state;
    mole_button_stable[button_number] = (uint8_t)raw_state;

    return (raw_state == target_state);
  }

  return false;
}

static void moleGameStart(uint32_t current_tick)
{
  score = 0U;
  game_running = true;

  /* 사용자가 시작 버튼을 뗀 시각이 매번 달라지는 점을 난수 씨앗으로 사용한다. */
  random_state = current_tick ^ 0xA5A5F411U;
  if (random_state == 0U)
  {
    random_state = 1U;
  }

  moleSelectFirstLed();
}

static void moleGameStop(void)
{
  /* 종료 시 점수는 조회할 수 있도록 유지하고 LED와 진행 상태만 정리한다. */
  game_running = false;
  active_led = MOLE_NO_ACTIVE_LED;
  moleAllLedOff();
}

void moleInit(void)
{
  uint8_t i;
  uint32_t current_tick = HAL_GetTick();

  sn74hc595Init();
  score = 0U;
  random_state = 1U;
  active_led = MOLE_NO_ACTIVE_LED;
  game_running = false;

  /* 부팅 당시 입력값을 초기 상태로 저장해 가짜 버튼 이벤트가 생기지 않게 한다. */
  for (i = 0U; i < MOLE_BUTTON_COUNT; i++)
  {
    GPIO_PinState state = HAL_GPIO_ReadPin(mole_buttons[i].port,
                                           mole_buttons[i].pin);
    button_states[i].stable_state = state;
    button_states[i].previous_raw_state = state;
    button_states[i].changed_tick = current_tick;
    mole_button_raw[i] = (uint8_t)state;
    mole_button_stable[i] = (uint8_t)state;
  }
}

void moleRun(void)
{
  uint8_t i;
  uint32_t current_tick = HAL_GetTick();

  /* 시작 버튼은 손을 뗀 순간(RESET -> SET)마다 시작과 종료를 전환한다. */
  if (moleButtonChangedTo(MOLE_START_BUTTON, GPIO_PIN_SET, current_tick))
  {
    if (game_running)
    {
      moleGameStop();
    }
    else
    {
      moleGameStart(current_tick);
    }

    return;
  }

  for (i = 0U; i < MOLE_COUNT; i++)
  {
    /* 게임 중 두더지 버튼을 누른 순간(SET -> RESET)에 정답을 판정한다. */
    if (moleButtonChangedTo(i, GPIO_PIN_RESET, current_tick)
        && game_running
        && (i == active_led))
    {
      score++;
      moleSelectNextLed();
    }
  }
}

bool moleIsRunning(void)
{
  return game_running;
}

uint32_t moleGetScore(void)
{
  return score;
}

uint8_t moleGetActiveLed(void)
{
  return active_led;
}
