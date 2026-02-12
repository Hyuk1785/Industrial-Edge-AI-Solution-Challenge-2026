/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : TIM2(2ch PWM) Motor + TIM3(1ch PWM) Servo + PC13 EXTI Button
  *
  * Wiring (너 기준 고정):
  *  - ENB = PA0 = TIM2_CH1
  *  - ENA = PA1 = TIM2_CH2
  *  - IN3 = PB0,  IN4 = PB1
  *  - IN1 = PB10, IN2 = PB11
  *  - SG90 Servo signal = PA6 = TIM3_CH1
  *
  * Power:
  *  - STM GND ↔ L298N GND ↔ Servo GND 공통
  *  - Servo는 외부 5V(1A↑) 권장(서보 때문에 모터 느려지는 거 방지)
  *
  * Button:
  *  - PC13 (B1) 보통 "눌리면 LOW" -> EXTI FALLING 권장
  *
  * Behavior:
  *  - 모터: 시작 1회 킥(100% 잠깐) 후 60% 유지
  *  - 버튼 누르면 서보: 한쪽(예: 20도)으로 이동 후 멈춤
  *    다시 누르면 서보: 90도로 복귀 후 멈춤
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN 0 */
/* ===== L298N 핀 매핑 ===== */
#define IN3_PORT GPIOB
#define IN3_PIN  GPIO_PIN_0
#define IN4_PORT GPIOB
#define IN4_PIN  GPIO_PIN_1

#define IN1_PORT GPIOB
#define IN1_PIN  GPIO_PIN_10
#define IN2_PORT GPIOB
#define IN2_PIN  GPIO_PIN_11

/* ===== PWM 채널 매핑 ===== */
#define ENB_PWM_CH TIM_CHANNEL_1   // TIM2_CH1 = PA0
#define ENA_PWM_CH TIM_CHANNEL_2   // TIM2_CH2 = PA1

/* ===== 모터 파라미터 ===== */
#define MOTOR_TARGET_DUTY      60   // 유지 듀티(%)
#define KICK_DUTY_PERCENT     100   // 출발 킥 듀티(%)
#define KICK_TIME_MS          120   // 출발 킥 시간(ms)

/* ===== 서보 파라미터 =====
   TIM3을 1MHz tick(1us)로 설정한다고 가정:
   - PSC=71  (72MHz/72 = 1MHz)
   - ARR=19999 (20ms) => 50Hz
*/
#define SERVO_HOME_DEG         60
#define SERVO_ACT_DEG          0   // 버튼 누르면 이 각도로 이동(원하면 160 등으로 바꿔)

/* ===== 버튼/상태 ===== */
volatile uint32_t g_btn_last_ms = 0;
volatile uint8_t  g_btn_event   = 0; // ISR에서 이벤트만 올림
volatile uint8_t  g_servo_pos   = 0; // 0=HOME, 1=ACT

/* --- Servo helpers --- */
static void Servo_SetPulseUs(uint16_t us)
{
  if (us < 500)  us = 500;
  if (us > 2500) us = 2500;
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, us);
}

static void Servo_SetAngle(uint8_t deg)
{
  if (deg > 180) deg = 180;
  // 0deg=500us, 180deg=2500us
  uint16_t us = 500 + (uint16_t)((2000U * deg) / 180U);
  Servo_SetPulseUs(us);
}

/* --- Motor PWM helpers (TIM2) --- */
static void PWM2_SetDutyPercent(uint32_t channel, uint8_t duty)
{
  if (duty > 100) duty = 100;
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim2);
  uint32_t ccr = ((arr + 1U) * duty) / 100U;
  if (ccr > arr) ccr = arr;
  __HAL_TIM_SET_COMPARE(&htim2, channel, ccr);
}

/* speed: -100~100, kick=1이면 시작 시 잠깐 100% */
static void MotorB_SetSpeed(int16_t speed, uint8_t kick)
{
  if (speed > 100) speed = 100;
  if (speed < -100) speed = -100;

  if (speed == 0) {
    HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_RESET);
    PWM2_SetDutyPercent(ENB_PWM_CH, 0);
    return;
  }

  if (speed > 0) {
    HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_SET);
    speed = -speed;
  }

  if (kick) {
    PWM2_SetDutyPercent(ENB_PWM_CH, KICK_DUTY_PERCENT);
    HAL_Delay(KICK_TIME_MS);
  }
  PWM2_SetDutyPercent(ENB_PWM_CH, (uint8_t)speed);
}

static void MotorA_SetSpeed(int16_t speed, uint8_t kick)
{
  if (speed > 100) speed = 100;
  if (speed < -100) speed = -100;

  if (speed == 0) {
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
    PWM2_SetDutyPercent(ENA_PWM_CH, 0);
    return;
  }

  if (speed > 0) {
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_SET);
    speed = -speed;
  }

  if (kick) {
    PWM2_SetDutyPercent(ENA_PWM_CH, KICK_DUTY_PERCENT);
    HAL_Delay(KICK_TIME_MS);
  }
  PWM2_SetDutyPercent(ENA_PWM_CH, (uint8_t)speed);
}

/* ===== 버튼 인터럽트 콜백 ===== */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#ifdef B1_Pin
  if (GPIO_Pin == B1_Pin)
#else
  if (GPIO_Pin == GPIO_PIN_13)
#endif
  {
    uint32_t now = HAL_GetTick();
    if (now - g_btn_last_ms < 200) return; // 디바운스
    g_btn_last_ms = now;
    g_btn_event = 1; // 메인 루프에서 처리
  }
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  // PWM 시작
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // ENB
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); // ENA
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // Servo PA6

  // 초기 상태
  PWM2_SetDutyPercent(TIM_CHANNEL_1, 0);
  PWM2_SetDutyPercent(TIM_CHANNEL_2, 0);
  Servo_SetAngle(SERVO_HOME_DEG);

  // 모터: 시작 1회 킥
  MotorA_SetSpeed(MOTOR_TARGET_DUTY, 1);
  MotorB_SetSpeed(MOTOR_TARGET_DUTY, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 모터 유지(킥 없이)
    MotorA_SetSpeed(MOTOR_TARGET_DUTY, 0);
    MotorB_SetSpeed(MOTOR_TARGET_DUTY, 0);

    // 버튼 이벤트 처리: 서보 토글(한쪽 이동/복귀)
    if (g_btn_event)
    {
      g_btn_event = 0;
      g_servo_pos ^= 1U;

      if (g_servo_pos) Servo_SetAngle(SERVO_ACT_DEG);
      else             Servo_SetAngle(SERVO_HOME_DEG);
    }

    HAL_Delay(10);
  }
  /* USER CODE END WHILE */
}

/* =================== CubeMX 생성 코드 영역 ===================
   아래 4개 함수는 CubeMX가 생성한 걸 그대로 두는 게 제일 안전함.
   (너 IOC 설정에 따라 핀 AF/클럭 설정이 자동으로 맞춰짐)
   여기서는 “동작 예시” 형태로 제공. 네 프로젝트 자동생성본과 다르면,
   자동생성본을 우선하고, USER CODE 블록만 이식해.
*/

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;            // 1MHz tick
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;              // 1kHz PWM
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) { Error_Handler(); }

  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) { Error_Handler(); }

  HAL_TIM_MspPostInit(&htim2);
}

static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;            // 1MHz tick (1us)
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;            // 20ms => 50Hz
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) { Error_Handler(); }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;               // 1.5ms = 90deg 근처
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) { Error_Handler(); }

  HAL_TIM_MspPostInit(&htim3);
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 모터 방향핀 출력
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // PC13 버튼 EXTI (대부분 눌리면 LOW -> FALLING)
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // EXTI interrupt init
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif
