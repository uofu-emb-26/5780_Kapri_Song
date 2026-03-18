#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"

void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */

const uint8_t sine_table[32] = {127,151,175,197,216,232,244,251,254,251,244,
 232,216,197,175,151,127,102,78,56,37,21,9,2,0,2,9,21,37,56,78,102};
 
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  //SystemClock_Config();


  // ADC_Calibration();

  // Threshold values
  int th1 = 10, th2 = 50, th3 = 100, th4 = 150;
  // Enable Port C Clock and set PC6 to Output
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
  GPIOC->MODER &= ~(3 << (6 * 2)); 
  GPIOC->MODER |=  (1 << (6 * 2));

  DAC_Wave();
  // Index variable to iterate through the array
  uint8_t index = 0;

  while (1)
  {
    /*
    // Read ADC data register
    uint8_t adcVal = ADC1->DR;

    if (adcVal > th1) 
      GPIOC->ODR |= (1 << 6); // Turn on LED
    else
      GPIOC->ODR &= ~(1 << 6); // Turn off
    
    if (adcVal > th2)
      GPIOC->ODR |= (1 << 7);
    else 
      GPIOC->ODR &= ~(1 << 7);

    if (adcVal > th3)
      GPIOC->ODR |= (1 << 8);
    else 
      GPIOC->ODR &= ~(1 << 8);

    if (adcVal > th4)
      GPIOC->ODR |= (1 << 9);
    else 
      GPIOC->ODR &= ~(1 << 9);*/


    GPIOC->ODR ^= (1 << 6);
    // Write the next value in the wave-table to the appropriate DAC data register
    // Because the wave table uses 8-bit data, use the 8-bit right-aligned data holding register (DAC_DHR8Rx)
    DAC->DHR8R1 = sine_table[index];

    // Increment the index and wrap around when reaching the end of the 32-element array
    index++;
    if (index >= 32) {
        index = 0;
    }

    // 1ms delay between updating the DAC to new values
    // Resulting frequency = ~31 Hz
    HAL_Delay(1);
  }
  return -1;
}

void ADC_Calibration(void)
{
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  // Configure PC6 (Red), PC7 (Green), PC8 (Orange), PC9 (Blue) as Outputs
  GPIOC->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2))); 
  GPIOC->MODER |=  ((1 << (6 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)));

  // Select GPIO pin (PC0) to use as ADC input (ACD_IN10)
  GPIOC->MODER |= (3 << (0 * 2));
  GPIOC->PUPDR &= ~(3 << (0 * 2)); // No pull-up/down resistor (00), analog mode (11)


  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC1 in RCC periph

  // Select/enable the input pin's channel for ADC conversion, hardware triggers disabled
  ADC1->CFGR1 |= ADC_CFGR1_RES_1; // Set RES to 10 for 8-bit resolution
  ADC1->CFGR1 |= ADC_CFGR1_CONT;  // Enable continuous conversion mode

  // Select/enable the input pin’s channel for ADC conversion
  ADC1->CHSELR |= ADC_CHSELR_CHSEL10; // Channel 10 (PC0)

  // Perform a self-calibration, enable, and start the ADC
  // 1. Ensure to disable ADC before attempting calibration
  if ((ADC1->CR & ADC_CR_ADEN) != 0) {
      ADC1->CR |= ADC_CR_ADDIS;
  }
  while ((ADC1->CR & ADC_CR_ADEN) != 0) {}; 
  // Start calibration
  ADC1->CR |= ADC_CR_ADCAL;
  // Wait for the hardware to signal that the calibration has completed (until ADCAL=0)
  while ((ADC1->CR & ADC_CR_ADCAL) != 0) {}; 
  
  // Set the peripheral enable
  ADC1->CR |= ADC_CR_ADEN; 
  // Wait until the ADC ready flag is set
  while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) {}; 
  
  // Start the ADC conversion
  ADC1->CR |= ADC_CR_ADSTART;
}

void DAC_Wave(void)
{
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

  // Configure the pin to analog mode (11), no pull-up/down resistors
  GPIOA->MODER |= (3 << (4 * 2)); // PA4
  GPIOA->PUPDR &= ~(3 << (4 * 2));

  // Enable DAC peripheral clk
  RCC->APB1ENR |= RCC_APB1ENR_DACEN;

  // Set DAC channel to software trigger mode
  DAC->CR &= ~DAC_CR_TEN1;

  // Enable the used DAC channel (Channel 1)
  DAC->CR |= DAC_CR_EN1;
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
