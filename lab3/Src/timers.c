#include "stm32f0xx_hal.h"

void TIM2_Init (void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 7999;
    TIM2->ARR = 250;

    TIM2->DIER = TIM_DIER_UIE; // Update interrupt
    
    TIM2->CR1 |= TIM_CR1_CEN; // Start timer
}
/*
void TIM3_Init (TIM_TypeDef){

    TIM3->PSC = 7999;
    TIM3->ARR = 249;

    TIM3->DIER = TIM_DIER_UIE;

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
}*/

void TIM2_IRQHandler(){
    if (TIM2->SR & TIM_SR_UIF) {
        
        // 1. Toggle PC8 and PC9
        // Using XOR (^) flips the bit: if it's 1 it becomes 0, if 0 it becomes 1.
        GPIOC->ODR ^= (1 << 8); 
        GPIOC->ODR ^= (1 << 9);

        // 2. IMPORTANT: Clear the pending flag
        TIM2->SR &= ~TIM_SR_UIF;
    }
}