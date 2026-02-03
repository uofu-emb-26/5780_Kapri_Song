#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"
// #include <assert.h>
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
  /* Configure the system clock */
  SystemClock_Config();

  HAL_RCC_GPIOC_CLK_Enable(); // Enable GPIO clock in the RCC
  //__HAL_RCC_GPIOC_CLK_ENABLE();
  //assert((RCC->AHBENR & RCC_AHBENR_GPIOCEN)!=0);

  // --- Initialize GPIO pins PC8 & PC9
  GPIO_InitTypeDef initStr = {GPIO_PIN_8 | GPIO_PIN_9,
                           GPIO_MODE_OUTPUT_PP,
                           GPIO_SPEED_FREQ_LOW,
                           GPIO_NOPULL};
  HAL_GPIO_Init(GPIOC, &initStr); 
  // CHECK 1: Verify MODE is Output (01) for Pin 8 (bits 17:16) and Pin 9 (bits 19:18)
  // Mask 0xF0000 isolates bits 19-16. Expected '0101' is 0x5.
  //assert((GPIOC->MODER & 0xF0000) == 0x50000);

  // CHECK 2: Verify OTYPE is Push-Pull (0) for Pin 8 and Pin 9
  // OTYPER has 1 bit per pin. Bits 8 and 9 must be 0.
  // assert((GPIOC->OTYPER & (GPIO_PIN_8 | GPIO_PIN_9)) == 0);

  // CHECK 3: Verify SPEED is Low (x0)
  // Low speed is usually 00. We check bits 16-19 again.
  //assert((GPIOC->OSPEEDR & 0xF0000) == 0);

  // CHECK 4: Verify PUPD is No Pull (00)
  // No pull is 00. We check bits 16-19.
  assert((GPIOC->PUPDR & 0xF0000) == 0);

  // --- Sets PC8 High ---
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
  // Verify PC8 is actually High in the Output Data Register (ODR)
  //assert((GPIOC->ODR & GPIO_PIN_8) != 0);
  
  while (1) {
    HAL_Delay(200); // Delay 200 ms

    //uint32_t prev_ODR = GPIOC->ODR;

    // Toggle the output state of both PC8 & PC9
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
    
    // Verify the state actually changed (XOR check)
    //assert((GPIOC->ODR & (GPIO_PIN_8 | GPIO_PIN_9)) != (prev_ODR & (GPIO_PIN_8 | GPIO_PIN_9)));
  }


}

void HAL_RCC_GPIOC_CLK_Enable(void){
  //RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
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
