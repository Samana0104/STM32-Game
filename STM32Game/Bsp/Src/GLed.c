#include "GLed.h"
#include "stm32f4xx_hal.h"


// 74HC595 제어용 3선 핀 매핑 (Nucleo-F411RE 기준)
#define LED_DATA_PORT    GPIOA
#define LED_DATA_PIN     GPIO_PIN_6   // A0 (PA0) - 직렬 데이터 입력 핀 (SER)

#define LED_CLOCK_PORT   GPIOA
#define LED_CLOCK_PIN    GPIO_PIN_7   // A2 (PA4) - 시프트 클럭 핀 (SRCLK)

#define LED_LATCH_PORT   GPIOB
#define LED_LATCH_PIN    GPIO_PIN_6   // A1 (PA1) - 래치/레지스터 클럭 핀 (RCLK)

#define SHIFT_BIT_COUNT  8            // 74HC595가 처리하는 기본 비트 수 (8비트)


// 현재 74HC595에 출력 중인 LED들의 상태 값을 보관하는 섀도우 레지스터(Shadow Register)
static uint8_t ledRegisterState = 0x00;


// 8비트 데이터를 MSB(최상위 비트)부터 한 비트씩 74HC595에 전송하는 내부 함수
static void ShiftOutData(uint8_t data)
{
    HAL_GPIO_WritePin(LED_CLOCK_PORT, LED_CLOCK_PIN, GPIO_PIN_RESET);
    
    for (int i = 0; i < SHIFT_BIT_COUNT; i++)
    {
        // 최상위 비트(MSB, 0x80)부터 차례대로 검사
        if (data & 0x80)
        {
            HAL_GPIO_WritePin(LED_DATA_PORT, LED_DATA_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(LED_DATA_PORT, LED_DATA_PIN, GPIO_PIN_RESET);
        }
        
        // 클럭 펄스 생성 (Rising Edge)
        HAL_GPIO_WritePin(LED_CLOCK_PORT, LED_CLOCK_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_CLOCK_PORT, LED_CLOCK_PIN, GPIO_PIN_SET);
        
        // 다음 비트 전송을 위해 데이터를 왼쪽으로 시프트
        data <<= 1;
    }
}

// 래치 핀을 제어하여 시프트된 8비트 데이터를 실제 출력 핀(Q0~Q7)으로 일괄 반영하는 내부 함수
static void UpdateLedHardware(void)
{
    // 래치를 LOW로 내려 데이터가 들어오는 동안 출력이 변하지 않도록 고정
    HAL_GPIO_WritePin(LED_LATCH_PORT, LED_LATCH_PIN, GPIO_PIN_RESET);
    
    // 8비트 데이터 직렬 전송 수행
    ShiftOutData(ledRegisterState);
    
    // 래치를 HIGH로 올려 전송된 데이터를 출력 핀에 일제히 반영
    HAL_GPIO_WritePin(LED_LATCH_PORT, LED_LATCH_PIN, GPIO_PIN_SET);
}


// LED 모듈 초기화 함수 (레지스터 상태 초기화 및 하드웨어 갱신)
void GledInit(void)
{
    GPIO_InitTypeDef gpioInit = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;

    gpioInit.Pin = LED_DATA_PIN;
    HAL_GPIO_Init(LED_DATA_PORT, &gpioInit);

    gpioInit.Pin = LED_CLOCK_PIN;
    HAL_GPIO_Init(LED_CLOCK_PORT, &gpioInit);

    gpioInit.Pin = LED_LATCH_PIN;
    HAL_GPIO_Init(LED_LATCH_PORT, &gpioInit);

    HAL_GPIO_WritePin(LED_DATA_PORT, LED_DATA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_CLOCK_PORT, LED_CLOCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_LATCH_PORT, LED_LATCH_PIN, GPIO_PIN_RESET);

    ledRegisterState = 0x00;  // 모든 LED 상태 끔(OFF)으로 초기화
    UpdateLedHardware();      // 초기 상태를 하드웨어에 반영
}


// 특정 LED의 ON/OFF 상태를 설정하는 함수
void SetLedState(LedId id, LedState state)
{
    // 정의된 LED 범위를 벗어날 경우 예외 처리
    if (id >= LED_MAX)
    {
        return;
    }

    // 요청받은 상태에 따라 레지스터 변수의 특정 비트 조작
    if (state == LED_ON)
    {
        ledRegisterState |= (1 << id);  // 해당 LED 비트만 1로 설정 (켜기)
    }
    else
    {
        ledRegisterState &= ~(1 << id); // 해당 LED 비트만 0으로 설정 (끄기)
    }

    // 변경된 레지스터 상태를 74HC595 하드웨어에 즉시 반영
    UpdateLedHardware();
}
