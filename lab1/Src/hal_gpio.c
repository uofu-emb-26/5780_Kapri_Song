#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>

void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    if (GPIOx == GPIOC) {
        // MODER: (01) General purpose output mode
        GPIOC->MODER &= ~(0xFF << 12); // clear all bits from 13-19
        GPIOC->MODER |= (0x55 << 12);

        // OTYPER: (0) Push-pull output type
        GPIOC->OTYPER &= ~(0xFF << 6);

        // OSPEEDR: (x0) Low speed
        GPIOC->OSPEEDR &= ~(0xFF << 12);
    }
    if (GPIOx == GPIOA) {
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

/*
GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    return -1;
}
*/

/*
void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
}
*/

/*
void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
}
*/
