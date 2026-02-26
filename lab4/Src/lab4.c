#include "main.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_rcc.h"

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

  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  // Reset the bits for PC10 and PC11 first
  GPIOC->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER11);
  // Set them to Alternate Function (10)
  GPIOC->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1);

  // Clear AFR bits then set AF1 (USART3)
  GPIOC->AFR[1] &= ~((0xF << GPIO_AFRH_AFSEL10_Pos) | (0xF << GPIO_AFRH_AFSEL11_Pos));
  GPIOC->AFR[1] |= (1 << GPIO_AFRH_AFSEL10_Pos) | (1 << GPIO_AFRH_AFSEL11_Pos);

  // Opt: Set Pull-up to prevent floating during transitions
  GPIOC->PUPDR |= (GPIO_PUPDR_PUPDR10_0 | GPIO_PUPDR_PUPDR11_0);


  // LED GPIO setup to put before the while(1) loop:
  GPIOC->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
  // Set pins 6, 7, 8, 9 to General Purpose Output Mode (01)
  GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

  USART_Init();

  while (1)
  {
    // Print a command prompt
    USART_TransString("CMD? ");

    char color = USART_ReceiveChar();
    USART_TransChar(color); // Echo the character to the terminal

    uint32_t led_pin = 0;
    char* color_name = "";

    switch (color)
    {
      case 'r': case 'R': 
        led_pin = GPIO_ODR_6; 
        color_name = "Red"; 
        break;
      case 'b': case 'B': 
        led_pin = GPIO_ODR_7; 
        color_name = "Blue"; 
        break;
      case 'o': case 'O': 
        led_pin = GPIO_ODR_8; 
        color_name = "Orange"; 
        break;
      case 'g': case 'G': 
        led_pin = GPIO_ODR_9; 
        color_name = "Green"; 
        break;
      
      // Unknown character prints error and restarts
      default:
        USART_TransString("\r\nError: Unknown color character.\r\n");
        continue; // 'continue' immediately jumps back up to the "CMD? " prompt
    }

    // Read the second character
    char action = USART_ReceiveChar();
    USART_TransChar(action); // Echo the character

    char* action_name = "";

    switch (action)
    {
      case '0': // Turn OFF
        GPIOC->ODR &= ~led_pin; 
        action_name = "turned off";
        break;
      
      case '1': // Turn ON
        GPIOC->ODR |= led_pin;  
        action_name = "turned on";
        break;
      
      case '2': // Toggle
        GPIOC->ODR ^= led_pin;
        action_name = "toggled";
        break;
      
      // Unknown character prints error and restarts
      default:
        USART_TransString("\r\nError: Unknown action character.\r\n");
        continue; 
    }

    USART_TransString("\r\nSuccess: ");
    USART_TransString(color_name);
    USART_TransString(" LED ");
    USART_TransString(action_name);
    USART_TransString(".\r\n\n");
  }

  return -1;
}

void USART_Init(void)
{ 
  // Enable system clock (USART3)
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
  
  uint32_t fClk = HAL_RCC_GetPCLK1Freq(); // System clk freq
  // Baud rate set
  USART3->BRR = fClk/115200;

  // Enable TX, RX, USART Enable
  USART3->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
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

// Interrupt and Status Register (ISR), Transmit Data Register (TDR)
void USART_TransChar(char c)
{ 
  // Check ISR
  while (!(USART3->ISR & USART_ISR_TXE)) // TXE: Transmit data reg empty
  {
  }

  // Clears register
  USART3->TDR = c;
}

// 
void USART_TransString(char* str) 
{
  while (*str != '\0') {
    USART_TransChar(*str);
    str++; // Increment ptr to next char
  }
}

// Receiver Data Register (RDR)
char USART_ReceiveChar(void)
{
  // Wait until the RXNE flag is set (data is ready to be read)
  while (!(USART3->ISR & USART_ISR_RXNE)) 
  {
  }

  // Read the data => clears the RXNE flag
  return (char)(USART3->RDR);
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
