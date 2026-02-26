#include "stm32f0xx_hal.h"

void TIM2_Init (void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 7999;
    TIM2->ARR = 250;

    TIM2->DIER |= TIM_DIER_UIE; // Update interrupt
    
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN; // Start timer
}

void TIM3_Init (void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = 9;
    TIM3->ARR = 1000;

    // Set Channels 1 and 2 to Output (00)
    TIM3->CCMR1 &= ~TIM_CCMR1_CC1S; 
    TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;
    
    TIM3->CCMR2 &= ~TIM_CCMR2_CC3S; 
    TIM3->CCMR2 &= ~TIM_CCMR2_CC4S;

    TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Clears before setting
    // Set output Channel 1(OC1M) to PWM Mode 2 (111)
    TIM3->CCMR1 |= (7<<4);
    // Set channel 2 (OC2M) to PWM Mode 1 (110)
    TIM3->CCMR1 |= (6<<12);

    // Enable output compare preload for both channels
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;

    // Set output enable bits for channels 1 & 2 in CCER
    TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    //Set CCR for both channels to 20% of ARR
    TIM3->CCR1 = 200;
    TIM3->CCR2 = 200;

    TIM3->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(){
    if (TIM2->SR & TIM_SR_UIF) {
        
        // Toggle PC8 and PC9
        // Using XOR (^) flips the bit: if it's 1 it becomes 0, if 0 it becomes 1.
        GPIOC->ODR ^= (1 << 8); 
        GPIOC->ODR ^= (1 << 9);

        // Clear the pending flag
        TIM2->SR &= ~TIM_SR_UIF;
    }
}

void PWM_Pins_Init(void) {
    // Enable GPIOC Clock
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Configure PC6 and PC7 to Alternate Function Mode (10)
    // Pin 6 uses bits 13:12, Pin 7 uses bits 15:14 in the MODER register
    GPIOC->MODER &= ~((3 << 12) | (3 << 14)); // Clear the bits first
    GPIOC->MODER |=  ((2 << 12) | (2 << 14)); // Set to 10 (AF mode)

    // Select the specific Alternate Function (AF0)
    // The AFR array has two registers: 
    // AFR[0] is AFRL (Pins 0-7)
    // AFR[1] is AFRH (Pins 8-15)
    // Since we are using Pins 6 and 7, use AFR[0]
    
    // Pin 6 uses bits 27:24, Pin 7 uses bits 31:28
    // Need AF0, clear the bits to 0.
    GPIOC->AFR[0] &= ~((0xF << 24) | (0xF << 28)); 
}