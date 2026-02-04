#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>

void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    if (GPIOx == GPIOC) {
        // MODER: (01) General purpose output mode
        GPIOC->MODER &= ~(0xF << 12); // clear bits Pin6 & Pin7
        GPIOC->MODER |= (0x5 << 12);

        // OTYPER: (0) Push-pull output type
        GPIOC->OTYPER &= ~(GPIO_PIN_6 | GPIO_PIN_7);

        // OSPEEDR: (x0) Low speed
        GPIOC->OSPEEDR &= ~(0xF << 12);

        // PUPDR: no pull-up/pull-down resistors
        GPIOC->PUPDR &= ~(0xF << 12);
    }
    else if (GPIOx == GPIOA) {
        // MODER: (00) Digital input mode
        GPIOA->MODER &= ~(0x3); // binary

        // OSPEEDR: (00) Low speed
        GPIOA->OSPEEDR &= ~(0x3);

        // PUPDR: (10) Pull-down resister
        GPIOA->PUPDR &= ~(0x3);
        GPIOA->PUPDR |= 0x2;
    }
}


/*
void My_HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
}
*/


GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    if((GPIOx->IDR & GPIO_Pin) != 0) // GPIO_Pin is there, set to IDR
        return GPIO_PIN_SET; // Button Pressed (High)
    else
        return GPIO_PIN_RESET; // Released (Low)
}



void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (PinState == GPIO_PIN_SET)
        GPIOx->BSRR = GPIO_Pin; // Turn LED on: write to lower half of BSRR => sets pin
    else
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16; // Turn off: upper half of BSRR to reset pin
}

void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}

