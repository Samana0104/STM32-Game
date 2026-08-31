# STM32 Whack-a-Mole Game

STM32F411RE 기반 보드에서 동작하는 7구 두더지 잡기 게임입니다. GPIO 버튼과 LED로 두더지를 표현하고, 조이스틱과 LCD1602으로 메뉴를 조작하며, FND·부저·내부 Flash를 이용해 점수와 사운드 및 영구 순위 기록을 제공합니다.

## 개발 목적

본 프로젝트는 **STM32 마이크로컨트롤러를 활용한 임베디드 시스템의 이해와 실습**을 목적으로 개발했습니다.

사용자가 직접 조작하고 결과를 즉시 확인할 수 있는 게임을 주제로 선정하여 단순한 하드웨어 제어를 넘어 입력 처리, 화면 출력, 게임 로직, 상태 전환, 사운드 재생 및 비휘발성 데이터 저장을 하나의 시스템으로 통합하는 과정을 경험하는 것이 목표입니다.

특히 제한된 연산 성능과 메모리를 가진 환경에서 다음 역량을 높이는 데 중점을 두었습니다.

- GPIO, ADC, PWM, UART, SysTick 및 내부 Flash 활용
- 폴링 기반 메인 루프와 비차단 상태 머신 설계
- 입력 디바운싱과 이벤트 기반 처리
- 제한된 LCD/FND 출력 자원의 효율적인 갱신
- 게임 상태와 하드웨어 제어 계층의 역할 분리
- 데이터 무결성 검사 및 Flash 수명 관리
- 불필요한 연산과 블로킹을 줄이는 성능 최적화

## 주요 기능

- 총 5개 스테이지
- 7개 LED와 버튼을 이용한 두더지 생성 및 타격
- 스테이지별 플레이 시간, LED 점등 시간, 다음 등장 지연, 최대 동시 두더지 수 및 Life 설정
- 점수, 콤보, 최고 콤보 및 Miss 처리
- 16×2 LCD 메뉴, 카운트다운, 플레이 정보, 결과 및 순위 표시
- 1자리 스테이지 FND와 4자리 점수 FND 출력
- BGM과 효과음을 분리한 2채널 부저 출력
- 내부 Flash에 Top 100 점수 영구 저장
- 기록 초기화 및 크레딧 화면
- LCD 연결 상태 확인 및 자동 재연결
- Debug 빌드용 UART 명령과 FPS 모니터링

## 게임 진행

1. 전원을 켜면 Title 상태로 진입합니다.
2. `GameStart`를 선택하면 Stage 1의 `3 → 2 → 1 → START!` 카운트다운이 시작됩니다.
3. 켜진 LED와 같은 번호의 버튼을 누르면 두더지를 맞힌 것으로 처리합니다.
4. 현재 두더지를 모두 맞히면 LED가 모두 꺼지고, 스테이지별 랜덤 대기 시간이 지난 뒤 다음 두더지가 등장합니다.
5. 잘못된 버튼을 누르거나 점등 제한시간 안에 두더지를 잡지 못하면 Miss가 적용됩니다.
6. Miss 수만큼 Life가 감소하고 점수가 차감되며 콤보가 초기화됩니다.
7. Life가 0이 되면 즉시 게임이 종료됩니다. 제한시간을 버티면 다음 스테이지로 진행합니다.
8. Stage 5까지 완료하거나 Life가 0이 되면 Result 상태로 이동합니다.
9. `GameOver`를 2초 동안 표시한 뒤 최종 점수와 최고 콤보를 출력합니다.
10. 최종 점수가 Top 100에 포함되면 Flash 순위표에 저장합니다.

두더지가 사라진 뒤의 대기 중 버튼을 누르는 것도 오답으로 처리되며, 새로운 랜덤 대기 시간이 적용됩니다.

## 점수와 콤보

기본 타격 점수는 10점이며 현재 콤보 등급에 따라 보너스가 추가됩니다.

| 콤보 | 등급 | 타격당 보너스 |
|---:|---|---:|
| 0~9 | 없음 | 0 |
| 10~29 | Good | 10 |
| 30~59 | Nice | 13 |
| 60~99 | Great | 16 |
| 100 이상 | Perfect | 20 |

- 두더지 Miss 또는 오답 1회당 20점 차감
- 점수 최솟값은 0, 최댓값은 9999
- 연속 타격이 끊기면 현재 콤보는 0으로 초기화
- 게임 결과에는 최종 점수와 게임 중 달성한 최고 콤보를 표시

## 스테이지 설정

| Stage | 시간 | 두더지 점등 시간 | 다음 등장 지연 | 최대 동시 등장 | 초기 Life |
|---:|---:|---:|---:|---:|---:|
| 1 | 20초 | 1500ms | 500~900ms | 1 | 10 |
| 2 | 25초 | 1000~1200ms | 450~800ms | 2 | 8 |
| 3 | 30초 | 800~1100ms | 400~700ms | 3 | 7 |
| 4 | 60초 | 700~1100ms | 350~600ms | 5 | 6 |
| 5 | 180초 | 600~1000ms | 300~500ms | 7 | 5 |

동시 등장 수는 1부터 해당 스테이지의 최댓값 사이에서 무작위로 결정됩니다. 스테이지 종료 2초 전부터는 새로운 두더지를 생성하지 않습니다.

## 조작 방법

보드에 장착된 조이스틱 방향은 케이스 배치 기준으로 사용하므로 화면 목록에서 `RIGHT`가 아래 이동, `LEFT`가 위 이동 역할을 합니다.

### Title

LCD에는 한 번에 두 메뉴가 표시됩니다.

```text
>1.GameStart
 2.Record
```

```text
>3.Reset Record
 4.Credit
```

| 입력 | 동작 |
|---|---|
| 조이스틱 LEFT | 이전 메뉴 |
| 조이스틱 RIGHT | 다음 메뉴 |
| 조이스틱 UP | 선택한 메뉴 실행 |

### Record

- Top 100 점수를 내림차순으로 관리
- LCD 한 페이지에 두 순위 표시
- RIGHT: 다음 순위 페이지
- LEFT: 이전 순위 페이지
- DOWN: Title로 복귀

### Credit

- 두 명씩 총 네 명의 크레딧 표시
- RIGHT: 다음 이름
- LEFT: 이전 이름
- DOWN: Title로 복귀

참여자:

1. Hanbit Byeon
2. Sunho Kim
3. Minsik Kim
4. GiBeom Nam

### Playing

- LED 1~7은 두더지 위치를 의미합니다.
- LED와 같은 번호의 버튼 1~7을 눌러 타격합니다.
- 켜지지 않은 위치의 버튼을 누르면 오답으로 처리합니다.

### Result

- 진입 직후 `GameOver` 표시
- 2초 후 `Your Score`와 `Max Combo` 표시
- 조이스틱을 움직이면 Title로 복귀

## 기획 명세 반영 현황

| 초기 기획 항목 | 현재 구현 |
|---|---|
| 5개 스테이지 | 구현 완료 |
| 스테이지별 등장 속도 | 점등 시간과 등장 전 대기 범위를 각각 설정 |
| 스테이지별 두더지 수 | 1개부터 스테이지별 최대 1/2/3/5/7개까지 무작위 등장 |
| 두더지를 놓치면 실패 | Miss 수만큼 Life 감소, Life 0이면 Result 전환 |
| 점수 출력 | 4자리 FND 및 LCD 결과 화면에 표시 |
| GameStart/Record 메뉴 | 구현 완료, Reset Record와 Credit 메뉴 추가 |
| Combo/Miss 표시 | 콤보 등급·획득 점수·Life·Miss 감점을 LCD에 표시 |
| GameOver 후 기록 표시 | 2초 후 최종 점수와 최고 콤보 표시 |
| Best record/Miss Count | Top 100 영구 순위표로 확장했으며 Miss Count는 저장하지 않음 |
| 상황별 사운드 | Title, InGame, Result, Credit BGM과 버튼/시작/성공/실패 효과음 구현 |

## 상태 머신

게임 화면과 로직은 다음 6개 상태로 분리되어 있습니다.

```text
Title ── Record
  │       └── Title
  ├──── Credit
  │       └── Title
  └──── Ready ── Playing
                    ├── Ready (다음 스테이지)
                    └── Result ── Title
```

각 상태는 `Enter`, `Update`, `Exit` 함수를 가지며 `GameState`가 현재 상태의 핸들러만 호출합니다.

| 상태 | 역할 |
|---|---|
| TitleState | 메인 메뉴 선택 및 기록 초기화 |
| RecordState | Top 100 순위 조회 |
| CreditState | 참여자 목록 표시 |
| ReadyState | 스테이지 설정 및 시작 카운트다운 |
| InGameState | 두더지 생성, 입력 판정, 점수·콤보·Life 처리 |
| ResultState | 기록 저장, GameOver 및 결과 표시 |

## 하드웨어 구성

### 주요 부품

- STM32 NUCLEO-F411RE / STM32F411RET6
- LED 7개와 74HC595 계열 시프트 레지스터
- Active-Low 버튼 7개
- 아날로그 2축 조이스틱
- LCD1602 + PCF8574T I/O Expander
- 1자리 + 4자리 7-Segment FND
- PWM 구동 부저 2개 또는 2채널 출력
- USART2 디버그 터미널

### 주요 핀

| 기능 | MCU 핀 | 설명 |
|---|---|---|
| 조이스틱 X | PC0 / ADC1_IN10 | 12-bit ADC |
| 조이스틱 Y | PC1 / ADC1_IN11 | 12-bit ADC |
| 버튼 1~7 | PA9, PA8, PB10, PB4, PB5, PB3, PA10 | 내부 Pull-up, Active-Low |
| LED DATA/CLOCK/LATCH | PA6, PA7, PB6 | 시프트 레지스터 제어 |
| FND DATA/LATCH/CLOCK | PB0, PB1, PB2 | 시프트 레지스터 제어 |
| FND Digit | PC2~PC6 | 1자리 스테이지 + 4자리 점수 |
| LCD SCL/SDA | PB8, PB9 | Software I2C, PCF8574 주소 0x20~0x27 탐색 |
| BGM 부저 | PA0 / TIM2_CH1 | PWM 주파수 출력 |
| 효과음 부저 | PA1 / TIM5_CH2 | PWM 주파수 출력 |
| USART2 TX/RX | PA2, PA3 | 로그 및 Debug 명령 |

조이스틱은 ADC 허용 전압에 맞춰 3.3V 전원을 사용해야 합니다.

## 사운드 시스템

`SoundPlayer`는 BGM과 효과음을 독립 상태로 관리하며 `HAL_GetTick()` 기반으로 음표를 순차 재생합니다.

- BGM 채널: Title, InGame, Result, Credit
- 효과음 채널: Button, Start, Success, Fail
- 음원 데이터: `STM32Game/Res/Sound`
- PWM 출력: `GTimer` → `GBuzzer`

단음 PWM 부저 특성상 실제 악기 음색이나 화음 대신 음계, 템포 및 아르페지오 패턴으로 상황별 분위기를 표현합니다.

## 순위와 Flash 저장

`GameRecord`는 최대 100개의 `uint32_t` 점수를 내림차순 배열로 관리합니다.

- 새 점수의 위치는 이진 탐색으로 결정
- 삽입 위치 이후 점수는 `memmove()`로 오른쪽 이동
- 0점 또는 100위 밖의 점수는 저장하지 않음
- 순위가 변경될 때 배열 전체를 Flash에 저장
- Reset Record 선택 시 순위 배열과 Flash 데이터 초기화

`GData`는 1024바이트 RAM 버퍼와 Flash Sector 7을 관리합니다.

| 항목 | 값 |
|---|---|
| Flash 저장 시작 주소 | `0x08060000` |
| 사용 섹터 | Sector 7 |
| 관리 버퍼 | 1024 bytes |
| 헤더 | 16 bytes |
| Payload | 1008 bytes |
| 데이터 식별자 | `GDAT` magic |
| 데이터 버전 | 2 |
| 무결성 검사 | 32-bit FNV-1a |

링커 스크립트는 펌웨어 영역을 384KB로 제한하여 `0x08060000~0x0807FFFF` 영역을 사용자 데이터용으로 예약합니다.

## LCD와 FND 갱신

- LCD 드라이버는 Software I2C 상태 머신으로 동작합니다.
- LCD 연결 상태를 주기적으로 검사하고 분리 또는 오류 발생 시 자동 재연결합니다.
- 재연결 후 보관된 16×2 프레임을 다시 출력합니다.
- FND 멀티플렉싱은 SysTick 인터럽트에서 1ms마다 갱신합니다.
- 게임 상태 로직은 메인 루프에서 처리하여 출력 갱신과 분리합니다.

## Debug UART 명령

Debug 빌드에서는 USART2를 통해 다음 명령을 사용할 수 있습니다.

| 명령 | 설명 |
|---|---|
| `help` | 명령 목록 출력 |
| `read <A\|B\|C\|H> <0-15>` | GPIO 입력 읽기 |
| `write <A\|B\|C\|H> <0-15> <0\|1>` | GPIO 출력 쓰기 |
| `led <1-7\|all> <0\|1>` | 두더지 LED 제어 |
| `fps` | 1초 간격 메인 루프 FPS 출력 |
| `stop` | GPIO/FPS 모니터링 중지 |

Release 빌드에서는 `G_LOG`와 Cheat 명령 처리가 제외됩니다.

## 프로젝트 구조

```text
stm32-game/
├── Core/                       # CubeMX 생성 초기화 및 인터럽트 코드
│   ├── Inc/
│   └── Src/
├── Drivers/                    # STM32 HAL 및 CMSIS
├── STM32Game/
│   ├── App/                    # 게임 규칙과 서비스
│   │   ├── GameMain            # 초기화 및 메인 업데이트 루프
│   │   ├── GameRecord          # Top 100 순위 관리
│   │   ├── GameStageConfig     # 스테이지 타입과 난이도 설정
│   │   └── SoundPlayer         # BGM/효과음 시퀀서
│   ├── Bsp/                    # GPIO/타이머/UART 보드 지원 계층
│   │   ├── GButton
│   │   ├── GLed
│   │   ├── GTimer
│   │   └── GUsart
│   ├── Driver/                 # 장치 드라이버
│   │   ├── GBuzzer
│   │   ├── GFnd
│   │   ├── GJoystick
│   │   └── GLcd1602
│   ├── State/                  # 화면 및 게임 상태 머신
│   │   ├── TitleState
│   │   ├── RecordState
│   │   ├── CreditState
│   │   ├── ReadyState
│   │   ├── InGameState
│   │   └── ResultState
│   ├── Res/Sound/              # 음표 기반 사운드 리소스
│   └── Utils/                  # Flash 데이터와 Debug 도구
│       ├── GData
│       └── GCheat
├── cmake/                      # ARM GCC/Clang 툴체인 설정
├── CMakeLists.txt
├── CMakePresets.json
├── STM32-Game.ioc              # STM32CubeMX 프로젝트
└── STM32F411xx_FLASH.ld        # Flash/RAM 링커 스크립트
```

## 빌드

### 필요 도구

- CMake 3.22 이상
- Ninja
- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`)
- STM32CubeProgrammer 또는 ELF 다운로드가 가능한 STM32 개발 환경

### Debug

```sh
cmake --preset Debug
cmake --build --preset Debug
```

### Release

```sh
cmake --preset Release
cmake --build --preset Release
```

생성된 ELF 파일은 각각 다음 경로에 위치합니다.

```text
build/Debug/STM32-Game.elf
build/Release/STM32-Game.elf
```

## 구현 방향

- 메인 루프에서는 입력, LCD 상태, 사운드 및 현재 게임 상태를 반복 갱신합니다.
- 시간 기반 기능은 긴 대기 함수 대신 `HAL_GetTick()`과 상태 변수로 처리합니다.
- FND처럼 일정한 주기가 필요한 출력은 SysTick 인터럽트에서 처리합니다.
- 하드웨어 접근, 게임 규칙, 상태 전환 및 리소스 데이터를 폴더별로 분리합니다.
- 스테이지 타입과 난이도는 `GameStageConfig`, 점수·콤보 정책은 해당 규칙을 사용하는 `InGameState.c`, 음원은 `Res/Sound`에서 조정할 수 있습니다.

## 74HC595 내부 구조

74HC595는 **8-bit Serial-In / Parallel-Out 시프트 레지스터**로, 적은 수의 MCU 핀으로 최대 8개의 출력을 제어할 수 있습니다.

내부에는 크게 두 개의 8-bit 레지스터가 존재합니다.

* **Shift Register**: D Flip-Flop 8개가 직렬로 연결된 구조로, `SH_CP` 클럭이 들어올 때마다 입력 데이터가 한 비트씩 이동
* **Storage Register**: Shift Register의 8-bit 데이터를 저장하며, `ST_CP` Latch 신호가 들어오면 `Q0~Q7` 출력에 동시에 반영

D Flip-Flop은 클럭의 특정 엣지에서 입력 `D` 값을 저장하고 다음 클럭이 들어올 때까지 해당 값을 유지하는 1-bit 저장 소자입니다.

```text
SER
 │
 ▼
[D Flip-Flop × 8]   Shift Register
 │        SH_CP
 ▼
[D Flip-Flop × 8]   Storage Register
 │        ST_CP
 ▼
Q0 ~ Q7
```

따라서 MCU에서는 데이터를 한 비트씩 전송하며 `CLOCK`을 8번 발생시킨 뒤, 마지막에 `LATCH`를 한 번 발생시켜 8개의 출력을 동시에 갱신합니다.

```text
DATA → CLOCK × 8 → LATCH → Q0~Q7 갱신
```

## 74HC595 전원 안정화

74HC595의 출력이 빠르게 전환될 때 발생하는 순간적인 전압 변동과 고주파 노이즈를 줄이기 위해 **0.1µF(104) 세라믹 디커플링 커패시터**를 사용합니다.

* 사용 부품: `0.1µF(104)` 세라믹 커패시터
* 연결 위치: `16번 VCC`와 `8번 GND` 사이
* 연결 방식: VCC-GND 사이 병렬 연결
* 사용 수량: 74HC595 하나당 1개
* 배치 위치: IC의 VCC/GND 핀에 최대한 가깝게 배치

```text
3.3V ──┬── 74HC595 16번 VCC
       │
      104
       │
GND ───┴── 74HC595 8번 GND
```

디커플링 커패시터는 출력 전환 순간 필요한 전류를 보조하고 전원선의 고주파 노이즈를 우회시켜 전원 전압을 안정화하며, 이를 통해 74HC595의 오작동 가능성을 줄입니다.

