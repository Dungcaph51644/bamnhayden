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
#include "main.h"

#define SEGMENTS (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define RGB_PERIOD 999U

typedef struct { uint8_t r, g, b; } RGB;

static const uint8_t digits[10] = {
  0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10
};
static const RGB colors[10] = {
  {255,0,0}, {255,64,0}, {255,255,0}, {64,255,0}, {0,255,0},
  {0,255,255}, {0,0,255}, {128,0,255}, {255,0,255}, {255,255,255}
};
static TIM_HandleTypeDef htim1;

static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void Display(uint8_t n);
static void SetRGB(uint8_t n, uint8_t brightness);

int main(void)
{
  HAL_Init();
  MX_GPIO_Init();
  MX_TIM1_Init();
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK ||
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK ||
      HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) Error_Handler();

  uint8_t count = 0;
  GPIO_PinState stable = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
  GPIO_PinState raw_last = stable;
  uint32_t now = HAL_GetTick(), raw_changed = now;
  uint32_t heartbeat = now, rgb_update = now;
  Display(count);

  while (1)
  {
    now = HAL_GetTick();
    GPIO_PinState raw = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
    if (raw != raw_last) { raw_last = raw; raw_changed = now; }
    if ((uint32_t)(now - raw_changed) >= 25U && raw != stable) {
      stable = raw;
      if (stable == GPIO_PIN_RESET) Display(count = (count + 1U) % 10U);
    }

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,
                      stable == GPIO_PIN_RESET ? GPIO_PIN_RESET : GPIO_PIN_SET);
    if ((uint32_t)(now - heartbeat) >= 250U) {
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
      heartbeat = now;
    }
    if ((uint32_t)(now - rgb_update) >= 10U) {
      uint32_t phase = now % 2000U;
      uint32_t ramp = phase <= 1000U ? phase : 2000U - phase;
      SetRGB(count, (uint8_t)(48U + ramp * 207U / 1000U));
      rgb_update = now;
    }
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, SEGMENTS | GPIO_PIN_15, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  gpio.Pin = SEGMENTS | GPIO_PIN_15;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &gpio);
  gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_11;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);
}

static void MX_TIM1_Init(void)
{
  TIM_OC_InitTypeDef channel = {0};
  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7U;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = RGB_PERIOD;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0U;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) Error_Handler();

  channel.OCMode = TIM_OCMODE_PWM1;
  channel.OCPolarity = TIM_OCPOLARITY_HIGH;
  channel.OCFastMode = TIM_OCFAST_DISABLE;
  for (uint32_t ch = TIM_CHANNEL_1; ch <= TIM_CHANNEL_3; ch += 4U)
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &channel, ch) != HAL_OK) Error_Handler();
}

static void Display(uint8_t n)
{
  uint8_t code = digits[n];
  GPIOA->BSRR = ((uint32_t)(code & 0x7FU) << 1) |
                ((uint32_t)(~code & 0x7FU) << 17);
}

static void SetRGB(uint8_t n, uint8_t brightness)
{
  const uint8_t value[] = {colors[n].r, colors[n].g, colors[n].b};
  for (uint32_t i = 0; i < 3U; ++i)
    __HAL_TIM_SET_COMPARE(&htim1, i << 2U,
                          (uint32_t)value[i] * brightness * RGB_PERIOD / 65025U);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}
