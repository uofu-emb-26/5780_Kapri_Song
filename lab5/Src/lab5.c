#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"

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
  I2C_Init();
  HAL_Delay(10);

  // 5.4 Set the transaction parameters in CR2 for a WRITE operation
  I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0)); // Clear the NBYTES and SADD bit fields using bitwise masking
  
  // Set SADD = 0x69 (shifted by 1 for 7-bit address format), NBYTES = 1
  I2C2->CR2 |= (2 << 16) | (0x69 << 1); 
  I2C2->CR2 &= ~I2C_CR2_RD_WRN;
  I2C2->CR2 |= I2C_CR2_START; // Set the START bit

  // 2. Wait until either the TXIS or NACKF flag is set
  while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
  I2C2->TXDR = 0x20; // 0x20: CRTL_REG1 register address
  while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
  I2C2->TXDR = 0x0B;
  while (!(I2C2->ISR & I2C_ISR_TC)) {}
  I2C2->CR2 |= I2C_CR2_STOP;

  // Read 0x0F
  I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0)); 
  I2C2->CR2 |= (1 << 16) | (0x69 << 1); 
  I2C2->CR2 &= ~I2C_CR2_RD_WRN; // READ
  I2C2->CR2 |= I2C_CR2_START;

  while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
  I2C2->TXDR = 0x0F;
  while (!(I2C2->ISR & I2C_ISR_TC)) {}

  I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0)); 
  I2C2->CR2 |= (1 << 16) | (0x69 << 1); 
  I2C2->CR2 |= I2C_CR2_RD_WRN; // READ mode
  I2C2->CR2 |= I2C_CR2_START;
  
  // 8. Read the contents of the RXDR register
  while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))) {}
  volatile uint8_t who_am_i = I2C2->RXDR;
  while (!(I2C2->ISR & I2C_ISR_TC)) {}
  I2C2->CR2 |= I2C_CR2_STOP;

  // Check if it matches the expected value of 0xD4
  if (who_am_i == 0xD3) {
      // The sensor is communicating correctly.
      GPIOC->ODR |= (1 << 9); // Blue LED PC9
  } else {
    GPIOC->ODR |= (1 << 7); // Green LED PC6
  }

  // 9. Set the STOP bit in the CR2 register to release the I2C bus
  I2C2->CR2 |= I2C_CR2_STOP;


  // 5.6
  while (1) {
    HAL_Delay(100); // Read every 100 ms 

    // --- Phase 1: Write starting register address with auto-increment bit ---
    I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0));
    I2C2->CR2 |= (1 << 16) | (0x69 << 1); 
    I2C2->CR2 &= ~I2C_CR2_RD_WRN; 
    I2C2->CR2 |= I2C_CR2_START;

    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF))) {}
    I2C2->TXDR = 0xA8; // Address 0x28 with MSB set 
    while (!(I2C2->ISR & I2C_ISR_TC)) {}

    // --- Phase 2: Read 4 consecutive data bytes (X and Y axes) ---
    I2C2->CR2 &= ~((0xFF << 16) | (0x3FF << 0));
    I2C2->CR2 |= (4 << 16) | (0x69 << 1); // NBYTES = 4 [cite: 168]
    I2C2->CR2 |= I2C_CR2_RD_WRN; 
    I2C2->CR2 |= I2C_CR2_START;

    uint8_t data[4];
    for(int i = 0; i < 4; i++) {
        while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF))) {}
        data[i] = I2C2->RXDR;
    }
    while (!(I2C2->ISR & I2C_ISR_TC)) {}
    I2C2->CR2 |= I2C_CR2_STOP;

    // --- Phase 3: Data assembly and LED Logic ---
    // Combine 8-bit registers into 16-bit signed integers [cite: 363, 493]
    int16_t x_val = (int16_t)((data[1] << 8) | data[0]);
    int16_t y_val = (int16_t)((data[3] << 8) | data[2]);

    // Clear LEDs and apply threshold to ignore noise [cite: 495]
    GPIOC->ODR &= ~((1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));
    int threshold = 1000;

    // Map axes to specific LEDs per board orientation [cite: 496, 497]
    if (x_val > threshold)       GPIOC->ODR |= (1 << 8); // Orange (Positive X)
    else if (x_val < -threshold)  GPIOC->ODR |= (1 << 9); // Blue (Negative X)

    if (y_val > threshold)       GPIOC->ODR |= (1 << 6); // Red (Positive Y)
    else if (y_val < -threshold)  GPIOC->ODR |= (1 << 7); // Green (Negative Y)
}
  return -1;
}


void I2C_Init(void)
{
  // Enable GPIOC & GPIOB clock
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;


  // Configure PC6 (Red), PC7 (Green), PC8 (Orange), PC9 (Blue) as Outputs
  GPIOC->MODER &= ~((3 << (6 * 2)) | (3 << (7 * 2)) | (3 << (8 * 2)) | (3 << (9 * 2))); 
  GPIOC->MODER |=  ((1 << (6 * 2)) | (1 << (7 * 2)) | (1 << (8 * 2)) | (1 << (9 * 2)));

  // Set PB11 & PB13 to AF mode as I2C2_SDA
  GPIOB->MODER &= ~((3 << (11*2)) | (3 << (13 * 2))); // Clears bit before setting
  GPIOB->MODER |= ((2 << (11*2)) | (2 << (13*2))); // Set to AF mode (10)
  // Set to open-drain output type (1)
  GPIOB->OTYPER |= (1 << 11) | (1 << 13);
  // Select the correct AF for I2C2 on PB11 and PB13
  GPIOB->AFR[1] &= ~((0xF << ((11 - 8) * 4)) | (0xF << ((13 - 8) * 4)));
  GPIOB->AFR[1] |=  ((1 << ((11 - 8) * 4)) | (5 << ((13 - 8) * 4))); // AF5

  // Enable internal pull-ups for PB11 and PB13 (01 in PUPDR)
  GPIOB->PUPDR &= ~((3 << (11 * 2)) | (3 << (13 * 2)));
  GPIOB->PUPDR |=  ((1 << (11 * 2)) | (1 << (13 * 2)));

  // Set PB14 (I2C address control) as Push-Pull Output, set High
  GPIOB->MODER &= ~(3 << (14 * 2));
  GPIOB->MODER |=  (1 << (14 * 2)); // Output mode
  GPIOB->OTYPER &= ~(1 << 14);      // Push-pull (default)
  GPIOB->ODR |= (1 << 14);          // Initialize high

  // Set PC0 (SPI/I2C mode select) as Push-Pull Output, set High
  GPIOC->MODER &= ~(3 << (0 * 2));
  GPIOC->MODER |=  (1 << (0 * 2));  // Output mode
  GPIOC->OTYPER &= ~(1 << 0);       // Push-pull (default)
  GPIOC->ODR |= (1 << 0);           // Initialize high

  // Set PB15 to Input mode (00 in MODER)
  GPIOB->MODER &= ~(3 << (15 * 2)); // Clears bits 31:30 to set as Input


  // 5.3 Enable clock for I2C2
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

  // Configure I2C2 TIMINGR for 100 kHz standard-mode (8MHz clock)
  // PRESC = 1, SCLDEL = 0x4, SDADEL = 0x2, SCLH = 0xC3, SCLL = 0xC7
  I2C2->TIMINGR = (1 << 28) | (0x4 << 20) | (0x2 << 16) | (0xC3 << 8) | (0xC7 << 0);

  // Enable the I2C2 peripheral using PC bit in CR1 reg
  I2C2->CR1 |= I2C_CR1_PE;



  // 
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
