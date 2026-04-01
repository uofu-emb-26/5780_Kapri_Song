#include <stdio.h>
#include <stdlib.h>
#include "stm32f0xx.h"
#include "motor.h"
#include "SEGGER_RTT.h"

volatile uint32_t debouncer;

void LED_init(void) {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  GPIOC->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;
  GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);
  GPIOC->OSPEEDR &= ~((GPIO_OSPEEDR_OSPEEDR8_0 | GPIO_OSPEEDR_OSPEEDR8_1) | 
                      (GPIO_OSPEEDR_OSPEEDR9_0 | GPIO_OSPEEDR_OSPEEDR9_1));
  GPIOC->PUPDR &= ~((GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR8_1) |
                    (GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR9_1));
  GPIOC->ODR &= ~(GPIO_ODR_8 | GPIO_ODR_9);
}

void button_init(void) {
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

  GPIOA->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1);
  GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR0_0 | GPIO_OSPEEDR_OSPEEDR0_1);
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1;

}

void Lab7_Systick_Callback(void) {
  debouncer = (debouncer >> 1);
  if (GPIOA->IDR & (1 << 0)) {
    debouncer |= 0x1;
  }
  if (debouncer == 0x7FFFFFFF) {
    __disable_irq();
    switch(target_rpm) {
      case 80: 
        target_rpm = 50;
        break;
      case 50:
        target_rpm = 81;
        break;
      case 0: 
        target_rpm = 80;
        break;
      default: 
        target_rpm = 0;
        break;
    }
    __enable_irq();
  }
}

volatile uint32_t encoder_count = 0;

int main(void) {
  debouncer = 0;
  HAL_Init();
  LED_init();
  button_init();
  motor_init();

  while (1) {
    GPIOC->ODR ^= GPIO_ODR_9;
    encoder_count = TIM2->CNT;
    HAL_Delay(128);
  }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add their own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
  /* User can add their own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
