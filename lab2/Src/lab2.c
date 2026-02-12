#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_gpio.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"
#include <stdint.h>
#ifndef assert
#define assert(condition) do { if (!(condition)) while(1); } while(0)
#endif

void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef initStr = {GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, 
                          GPIO_MODE_OUTPUT_PP,
                          GPIO_SPEED_FREQ_LOW,
                          GPIO_NOPULL};
  My_HAL_GPIO_Init(GPIOC, &initStr);


  GPIO_InitTypeDef initBtn = {GPIO_PIN_0,
                          GPIO_MODE_INPUT,
                          GPIO_SPEED_FREQ_LOW,
                          GPIO_PULLDOWN};
  My_HAL_GPIO_Init(GPIOA, &initBtn);
  assert((SYSCFG->EXTICR[0] & SYSCFG_EXTICR1_EXTI0) == 0);
  //assert((EXTI->IMR & EXTI_IMR_MR0) == 0);
  //assert((EXTI->RTSR & EXTI_RTSR_TR0) == 0);
  Enable_EXTI_Button();

  uint32_t val = SYSCFG->EXTICR[0] & SYSCFG_EXTICR1_EXTI0;
  assert(val == SYSCFG_EXTICR1_EXTI0_PA);
  
  /*
  assert((EXTI->IMR & EXTI_IMR_MR0) == EXTI_IMR_MR0);
assert((EXTI->RTSR & EXTI_RTSR_TR0) == EXTI_RTSR_TR0);*/
  // Sets PC9 high
  My_HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);

  /* Configure the system clock */
  SystemClock_Config();
  //uint32_t time = 0;
  
  // Enable the EXTI0_1 Interrupt in the NVIC
  NVIC_EnableIRQ(EXTI0_1_IRQn); 
  // Set Priority to 1 (High Priority) 
  NVIC_SetPriority(EXTI0_1_IRQn, 1);

  while (1)
  {
    My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
    HAL_Delay(500);
    /*if ((HAL_GetTick() - time) >= 500U) {
      time = HAL_GetTick();
      My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
      // HAL_Delay(500);
    }*/
  }
  return -1;
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
/**
 * @brief  EXTI Line 0 and 1 Interrupt Handler
 */
void EXTI0_1_IRQHandler(void)
{
    // 1. Toggle Green (PC9) and Orange (PC8) LEDs
    My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
    My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

    // 2. Clear the EXTI Pending Flag for Line 0
    // The manual notes that these bits require a "different action" to clear .
    // For the EXTI Pending Register (PR), writing a '1' to the bit CLEARS it.
    EXTI->PR |= EXTI_PR_PR0; 
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
