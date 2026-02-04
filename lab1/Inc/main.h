#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "stm32f0xx_it.h"

void Error_Handler(void);
void HAL_RCC_GPIOC_CLK_Enable(void);
void HAL_RCC_GPIOA_CLK_Enable(void);
void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


