/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */







// Bảng mã LED 7 thanh Anode chung (PA1 -> PA7)
const uint8_t LED7_ANODE[10] = {
    0x40, // Số 0
    0x79, // Số 1
    0x24, // Số 2
    0x30, // Số 3
    0x19, // Số 4
    0x12, // Số 5
    0x02, // Số 6
    0x78, // Số 7
    0x00, // Số 8
    0x10  // Số 9
};

uint8_t count = 0;                       // Biến lưu giá trị đếm (0 đến 9)
GPIO_PinState button_state_last = GPIO_PIN_SET;
TIM_HandleTypeDef htim1;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGBColor;

static const RGBColor digit_colors[10] = {
    {255,   0,   0},
    {255,  64,   0},
    {255, 255,   0},
    { 64, 255,   0},
    {  0, 255,   0},
    {  0, 255, 255},
    {  0,   0, 255},
    {128,   0, 255},
    {255,   0, 255},
    {255, 255, 255}
};

// Khai báo nguyên mẫu hàm hiển thị
void LED7_Display(uint8_t number);
static void MX_TIM1_Init(void);
static void RGB_SetBrightness(uint8_t number, uint8_t brightness);








/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK ||
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK ||
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN 2 */

  // Đọc trạng thái chân PA11 thực tế lúc vừa bật nguồn để làm mốc
    button_state_last = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);

    // Hiển thị chuẩn số 0 ban đầu ra LED 7 thanh
    LED7_Display(count);

    uint32_t last_heartbeat = HAL_GetTick();
    uint32_t last_rgb_update = last_heartbeat;
    GPIO_PinState button_raw_last = button_state_last;
    uint32_t button_raw_changed_at = last_heartbeat;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {












      uint32_t now = HAL_GetTick();
      GPIO_PinState button_raw = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);

      if (button_raw != button_raw_last)
      {
          button_raw_last = button_raw;
          button_raw_changed_at = now;
      }

      if ((uint32_t)(now - button_raw_changed_at) >= 25U &&
          button_raw != button_state_last)
      {
          button_state_last = button_raw;
          if (button_state_last == GPIO_PIN_RESET)
          {
              count = (uint8_t)((count + 1U) % 10U);
              LED7_Display(count);
          }
      }

      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,
                        button_state_last == GPIO_PIN_RESET ? GPIO_PIN_RESET : GPIO_PIN_SET);

      if ((uint32_t)(now - last_heartbeat) >= 250U)
      {
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
          last_heartbeat = now;
      }

      if ((uint32_t)(now - last_rgb_update) >= 10U)
      {
          uint32_t phase = now % 2000U;
          uint8_t brightness = (uint8_t)(48U +
              (phase <= 1000U ? phase : 2000U - phase) * 207U / 1000U);
          RGB_SetBrightness(count, brightness);
          last_rgb_update = now;
      }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 PA7 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

static void MX_TIM1_Init(void)
{
  TIM_OC_InitTypeDef channel_config = {0};

  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7U;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999U;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0U;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  channel_config.OCMode = TIM_OCMODE_PWM1;
  channel_config.Pulse = 0U;
  channel_config.OCPolarity = TIM_OCPOLARITY_HIGH;
  channel_config.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  channel_config.OCFastMode = TIM_OCFAST_DISABLE;
  channel_config.OCIdleState = TIM_OCIDLESTATE_RESET;
  channel_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &channel_config, TIM_CHANNEL_1) != HAL_OK ||
      HAL_TIM_PWM_ConfigChannel(&htim1, &channel_config, TIM_CHANNEL_2) != HAL_OK ||
      HAL_TIM_PWM_ConfigChannel(&htim1, &channel_config, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
}

static void RGB_SetBrightness(uint8_t number, uint8_t brightness)
{
  const RGBColor color = digit_colors[number];

  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)color.red * brightness * 999U / 65025U);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)color.green * brightness * 999U / 65025U);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)color.blue * brightness * 999U / 65025U);
}






void LED7_Display(uint8_t number)
{
    uint8_t code = LED7_ANODE[number];

    // Quét từng bit từ đoạn a (PA1) đến đoạn g (PA7)
    for (int i = 0; i < 7; i++)
    {
        if ((code >> i) & 0x01)
        {
            // Bit = 1 -> Xuất Mức HIGH -> TẮT đoạn LED Anode chung
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 << i, GPIO_PIN_SET);
        }
        else
        {
            // Bit = 0 -> Xuất Mức LOW -> SÁNG đoạn LED Anode chung
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 << i, GPIO_PIN_RESET);
        }
    }
}











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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
