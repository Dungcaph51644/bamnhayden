/* Copyright (c) 2026 STMicroelectronics. Licensed under BSD-3-Clause. */
#include "main.h"

#define SEGMENTS (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | \
                  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define BUTTON GPIO_PIN_11
#define LED GPIO_PIN_15

static const uint8_t digits[10] = {
  0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10
};

static void Display(uint8_t n)
{
  uint8_t code = digits[n];
  GPIOA->BSRR = ((uint32_t)(code & 0x7FU) << 1) |
                ((uint32_t)(~code & 0x7FU) << 17);
}

int main(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint8_t count = 0;

  HAL_Init();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOA, SEGMENTS | LED, GPIO_PIN_SET);
  gpio.Pin = SEGMENTS | LED;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio);
  gpio.Pin = BUTTON;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);
  Display(count);

  while (1)
    if (HAL_GPIO_ReadPin(GPIOA, BUTTON) == GPIO_PIN_RESET) {
      HAL_Delay(25);
      if (HAL_GPIO_ReadPin(GPIOA, BUTTON) == GPIO_PIN_RESET) {
        Display(count = (count + 1U) % 10U);
        HAL_GPIO_WritePin(GPIOA, LED, GPIO_PIN_RESET);
        while (HAL_GPIO_ReadPin(GPIOA, BUTTON) == GPIO_PIN_RESET) {}
        HAL_GPIO_WritePin(GPIOA, LED, GPIO_PIN_SET);
        HAL_Delay(25);
      }
    }
}
