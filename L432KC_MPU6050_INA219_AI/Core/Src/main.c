/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_x-cube-ai.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpu6050.h"
#include "ina219.h"
#include "app_x-cube-ai.h"
#include "network.h"
#include "network_data.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MODE_COLLECT  0
#define MODE_INFER    1
#define CURRENT_MODE  MODE_COLLECT  // ← 수집할때: MODE_COLLECT / 추론할때: MODE_INFER
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
MPU6050_t MPU6050_Data;
INA219_t  INA219_Data;

// ── 윈도우 버퍼 ──
#define WINDOW_SIZE  50
#define FEATURE_NUM  21

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float current, voltage, power;
} SensorSample;

SensorSample window_buf[WINDOW_SIZE];
uint16_t window_idx = 0;

// ── Scaler 값 (Python에서 추출) ──
const float SCALER_MEAN[FEATURE_NUM] = {
    2005.842149f, 1065.240787f, 214.114126f,
    11577.090909f, 5699.581818f, 7027.472727f,
    1285.567402f, 21359.667551f,
    200.124709f, 436.414016f, 201.440447f,
    998.200000f, 2458.363636f, 1171.750000f,
    0.169205f, 0.023431f, 0.070600f,
    6.623082f, 0.085506f, 1.121541f, 0.260091f
};

const float SCALER_STD[FEATURE_NUM] = {
    482.364637f, 162.312108f, 214.114126f,
    4186.752281f, 1462.477962f, 1739.716323f,
    221.864917f, 198.976645f,
    26.586816f, 93.917616f, 64.067405f,
    231.108330f, 796.234539f, 547.087617f,
    0.006212f, 0.001946f, 0.005502f,
    0.162638f, 0.002384f, 0.023446f, 0.128041f
};

float g_features[FEATURE_NUM];
float g_output = 0.0f;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#include <stdio.h>

int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
	return len;
}

// ── Feature 추출 ──
void extract_features(SensorSample *buf, float *features) {
    float ax[WINDOW_SIZE], ay[WINDOW_SIZE], az[WINDOW_SIZE];
    float gx[WINDOW_SIZE], gy[WINDOW_SIZE], gz[WINDOW_SIZE];
    float cur[WINDOW_SIZE], vol[WINDOW_SIZE], pwr[WINDOW_SIZE];
    float mag[WINDOW_SIZE];

    float ax_sum=0, ay_sum=0, az_sum=0;
    float gx_sum=0, gy_sum=0, gz_sum=0;
    float cur_sum=0, vol_sum=0, pwr_sum=0;
    float mag_sum=0;

    float ax_min=1e9f, ax_max=-1e9f;
    float ay_min=1e9f, ay_max=-1e9f;
    float az_min=1e9f, az_max=-1e9f;
    float gx_min=1e9f, gx_max=-1e9f;
    float gy_min=1e9f, gy_max=-1e9f;
    float gz_min=1e9f, gz_max=-1e9f;
    float cur_min=1e9f, cur_max=-1e9f;

    uint16_t cur_over = 0;

    for(int i = 0; i < WINDOW_SIZE; i++) {
        ax[i] = buf[i].ax; ay[i] = buf[i].ay; az[i] = buf[i].az;
        gx[i] = buf[i].gx; gy[i] = buf[i].gy; gz[i] = buf[i].gz;
        cur[i] = buf[i].current; vol[i] = buf[i].voltage; pwr[i] = buf[i].power;
        mag[i] = sqrtf(ax[i]*ax[i] + ay[i]*ay[i] + az[i]*az[i]);

        ax_sum += ax[i]; ay_sum += ay[i]; az_sum += az[i];
        gx_sum += gx[i]; gy_sum += gy[i]; gz_sum += gz[i];
        cur_sum += cur[i]; vol_sum += vol[i]; pwr_sum += pwr[i];
        mag_sum += mag[i];

        if(ax[i] < ax_min) ax_min = ax[i]; if(ax[i] > ax_max) ax_max = ax[i];
        if(ay[i] < ay_min) ay_min = ay[i]; if(ay[i] > ay_max) ay_max = ay[i];
        if(az[i] < az_min) az_min = az[i]; if(az[i] > az_max) az_max = az[i];
        if(gx[i] < gx_min) gx_min = gx[i]; if(gx[i] > gx_max) gx_max = gx[i];
        if(gy[i] < gy_min) gy_min = gy[i]; if(gy[i] > gy_max) gy_max = gy[i];
        if(gz[i] < gz_min) gz_min = gz[i]; if(gz[i] > gz_max) gz_max = gz[i];
        if(cur[i] < cur_min) cur_min = cur[i]; if(cur[i] > cur_max) cur_max = cur[i];

        if(cur[i] >= 0.19f) cur_over++;
    }

    float n = (float)WINDOW_SIZE;

    // STD 계산
    float ax_std=0, ay_std=0, az_std=0;
    float gx_std=0, gy_std=0, gz_std=0;
    float cur_std=0, vol_std=0, mag_std=0;

    float ax_m=ax_sum/n, ay_m=ay_sum/n, az_m=az_sum/n;
    float gx_m=gx_sum/n, gy_m=gy_sum/n, gz_m=gz_sum/n;
    float cur_m=cur_sum/n, vol_m=vol_sum/n;
    float mag_m=mag_sum/n;

    for(int i = 0; i < WINDOW_SIZE; i++) {
        ax_std  += (ax[i]-ax_m)*(ax[i]-ax_m);
        ay_std  += (ay[i]-ay_m)*(ay[i]-ay_m);
        az_std  += (az[i]-az_m)*(az[i]-az_m);
        gx_std  += (gx[i]-gx_m)*(gx[i]-gx_m);
        gy_std  += (gy[i]-gy_m)*(gy[i]-gy_m);
        gz_std  += (gz[i]-gz_m)*(gz[i]-gz_m);
        cur_std += (cur[i]-cur_m)*(cur[i]-cur_m);
        vol_std += (vol[i]-vol_m)*(vol[i]-vol_m);
        mag_std += (mag[i]-mag_m)*(mag[i]-mag_m);
    }

    ax_std  = sqrtf(ax_std/n);  ay_std  = sqrtf(ay_std/n);
    az_std  = sqrtf(az_std/n);  gx_std  = sqrtf(gx_std/n);
    gy_std  = sqrtf(gy_std/n);  gz_std  = sqrtf(gz_std/n);
    cur_std = sqrtf(cur_std/n); vol_std = sqrtf(vol_std/n);
    mag_std = sqrtf(mag_std/n);

    // Feature 배열 (Python 순서와 동일!)
    features[0]  = ax_std;
    features[1]  = ay_std;
    features[2]  = az_std;
    features[3]  = ax_max - ax_min;   // ax_p2p
    features[4]  = ay_max - ay_min;   // ay_p2p
    features[5]  = az_max - az_min;   // az_p2p
    features[6]  = mag_std;
    features[7]  = mag_m;
    features[8]  = gx_std;
    features[9]  = gy_std;
    features[10] = gz_std;
    features[11] = gx_max - gx_min;   // gx_p2p
    features[12] = gy_max - gy_min;   // gy_p2p
    features[13] = gz_max - gz_min;   // gz_p2p
    features[14] = cur_m;
    features[15] = cur_std;
    features[16] = cur_max - cur_min; // current_p2p
    features[17] = vol_m;
    features[18] = vol_std;
    features[19] = pwr_sum / n;       // power_mean
    features[20] = (float)cur_over / n; // current_over_019
}

// ── 정규화 ──
void normalize(float *features) {
    for(int i = 0; i < FEATURE_NUM; i++) {
        features[i] = (features[i] - SCALER_MEAN[i]) / SCALER_STD[i];
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_X_CUBE_AI_Init();
  /* USER CODE BEGIN 2 */


  printf("=== Motor Anomaly Detection ===\r\n");

  if(MPU6050_Init(&hi2c1)) printf("MPU6050: OK\r\n");
  if(INA219_Init(&hi2c1))  printf("INA219:  OK\r\n");

#if CURRENT_MODE == MODE_COLLECT
  printf("Mode: DATA COLLECT\r\n");
  printf("time_s,ax,ay,az,gx,gy,gz,current,voltage,power\r\n");  // CSV 헤더!
#else
  printf("Mode: AI INFER\r\n");
  printf("AI Model: OK\r\n");
#endif

//  printf("=== Motor Monitoring System ===\r\n");
//
//  if(MPU6050_Init(&hi2c1)) printf("MPU6050: OK\r\n");
//  if(INA219_Init(&hi2c1))  printf("INA219:  OK\r\n");
//
//  printf("ax,ay,az,gx,gy,gz,current,voltage,power\r\n");


//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
//
//  printf("STM32 L432KC Started!\r\n");

//  printf("\r\n=== L432KC + MPU6050 Test ===\r\n");
//
//  // MPU6050 초기화
//  if(MPU6050_Init(&hi2c1)) {
//      printf("MPU6050 Ready!\r\n\r\n");
//  } else {
//      printf("MPU6050 Init Failed!\r\n");
//      Error_Handler();
//  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE END WHILE */

	  /* USER CODE BEGIN 3 */


	  MPU6050_Read_All(&hi2c1, &MPU6050_Data);
	  INA219_Read_All(&hi2c1, &INA219_Data);

#if CURRENT_MODE == MODE_COLLECT
	  printf("%.3f,%d,%d,%d,%d,%d,%d,%.3f,%.2f,%.2f\r\n",
			  HAL_GetTick() / 1000.0f,
			  MPU6050_Data.Accel_X_RAW,
			  MPU6050_Data.Accel_Y_RAW,
			  MPU6050_Data.Accel_Z_RAW,
			  MPU6050_Data.Gyro_X_RAW,
			  MPU6050_Data.Gyro_Y_RAW,
			  MPU6050_Data.Gyro_Z_RAW,
			  INA219_Data.current,
			  INA219_Data.voltage,
			  INA219_Data.power);

#else
	  window_buf[window_idx % WINDOW_SIZE].ax      = (float)MPU6050_Data.Accel_X_RAW;
	  window_buf[window_idx % WINDOW_SIZE].ay      = (float)MPU6050_Data.Accel_Y_RAW;
	  window_buf[window_idx % WINDOW_SIZE].az      = (float)MPU6050_Data.Accel_Z_RAW;
	  window_buf[window_idx % WINDOW_SIZE].gx      = (float)MPU6050_Data.Gyro_X_RAW;
	  window_buf[window_idx % WINDOW_SIZE].gy      = (float)MPU6050_Data.Gyro_Y_RAW;
	  window_buf[window_idx % WINDOW_SIZE].gz      = (float)MPU6050_Data.Gyro_Z_RAW;
	  window_buf[window_idx % WINDOW_SIZE].current = INA219_Data.current;
	  window_buf[window_idx % WINDOW_SIZE].voltage = INA219_Data.voltage;
	  window_buf[window_idx % WINDOW_SIZE].power   = INA219_Data.power;
	  window_idx++;

	  if(window_idx >= WINDOW_SIZE && window_idx % 10 == 0) {
		  extract_features(window_buf, g_features);
		  normalize(g_features);
		  MX_X_CUBE_AI_Process();

		  static uint8_t defect_count = 0;
		  static uint8_t vote_total   = 0;
		  vote_total++;
		  if(g_output > 0.5f) defect_count++;
		  printf("[%d/%d] score: %.4f\r\n", defect_count, vote_total, g_output);
		  if(vote_total >= 5) {
			  if(defect_count >= 3)
				  printf("=== [DEFECT DETECTED] %d/5 ===\r\n", defect_count);
			  else
				  printf("=== [NORMAL] %d/5 ===\r\n", defect_count);
			  defect_count = 0;
			  vote_total   = 0;
		  }
	  }
#endif

	  HAL_Delay(100);
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_LED_Pin */
  GPIO_InitStruct.Pin = USER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
