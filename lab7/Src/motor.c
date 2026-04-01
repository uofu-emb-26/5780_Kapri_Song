#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f0xx.h>
#include <SEGGER_RTT.h>
#include "motor.h"
#include "Legacy/stm32_hal_legacy.h"

volatile int16_t error_integral;
volatile uint8_t duty_cycle;
volatile int16_t target_rpm;
volatile int16_t motor_speed;
volatile int8_t adc_value;
volatile int16_t error;
volatile uint8_t Kp = 150;
volatile uint8_t Ki = 3;

static uint8_t buf0[1024];
static uint8_t buf1[1024];
static uint8_t buf2[1024];

union byte_split {
    uint32_t uword;
    int32_t word;
    uint8_t bytes[4];
};

void log_init(void) {
    SEGGER_RTT_ConfigUpBuffer(0, "", buf0, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_ConfigUpBuffer(1, "", buf1, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_ConfigUpBuffer(2, "", buf2, 1024, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
}

void log_data(void) {
    __disable_irq();
    uint32_t duty_cycle_copy = duty_cycle;
    int32_t target_rpm_copy = target_rpm;
    int32_t motor_speed_copy = motor_speed;
    __enable_irq();

    union byte_split data;
    data.uword = duty_cycle_copy;
    SEGGER_RTT_Write (0, &data.bytes, 4);
    data.word = target_rpm_copy;
    SEGGER_RTT_Write (1, &data.bytes, 4);
    data.word = motor_speed_copy;
    SEGGER_RTT_Write (2, &data.bytes, 4);
}

void motor_init(void) {
    log_init();
    pwm_init();
    encoder_init();
    ADC_init();
}

void pwm_init(void) {
    GPIOA->MODER |= (1 << 9);
    GPIOA->MODER &= ~(1 << 8);

    GPIOA->AFR[0] &= 0xFFF0FFFF;
    GPIOA->AFR[0] |= (1 << 18);
    
    GPIOA->MODER &= 0xFFFFC3FF;
    GPIOA->MODER |= (1 << 10) | (1 << 12);

    GPIOA->ODR |= (1 << 5);
    GPIOA->ODR &= ~(1 << 6);

    RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14->CR1 = 0;
    TIM14->CCMR1 = 0;
    TIM14->CCER = 0;

    TIM14->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE);
    TIM14->CCER |= TIM_CCER_CC1E;

    TIM14->PSC = 1;
    TIM14->ARR = 1200;
    TIM14->CCR1 = 0;

    TIM14->CR1 |= TIM_CR1_CEN;
}

void pwm_setDutyCycle(uint8_t duty) {
    if (duty <= 100) {
        TIM14->CCR1 = ((uint32_t)duty*TIM14->ARR) / 100;

    }
}

void encoder_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER &= ~(GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0);
    GPIOB->MODER |= (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1);
    GPIOB->AFR[0] |= ((1 << 16) | (1 << 20));

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->CCMR1 = 0;
    TIM3->CCER = 0;
    TIM3->SMCR = 0;
    TIM3->CR1 = 0;

    TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0);
    TIM3->SMCR |= (TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0);
    TIM3->ARR = 0xFFFF;
    TIM3->CNT = 0x7FFF;
    TIM3->CR1 |= TIM_CR1_CEN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    TIM6->PSC = 11;
    TIM6->ARR = 30000;

    TIM6->DIER |= TIM_DIER_UTE;
    TIM6->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    NVIC_SetPriority(TIM6_DAC_IRQn,2);
}

void TIM6_DAC_IRQHandler(void) {
    motor_speed = (TIM3->CNT - 0x7FFF);
    TIM3->CNT = 0x7FFF;

    PI_update();

    TIM6->SR &= ~TIM_SR_UIF;
}

void ADC_init(void) {
    GPIOA->MODER |= (GPIO_MODER_MODER1_0 | GPIO_MODER_MODER1_1);

    RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

    ADC1->CFGR1 = 0;
    ADC1->CFGR1 |= ADC_CFGR1_CONT;
    ADC1->CHSELR |= ADC_CHSELR_CHSEL1;

    ADC1->CR = 0;
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL);

    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CR |= ADC_CR_ADSTART;
}

void PI_update(void) {
    __disable_irq();

    int16_t error = target_rpm - motor_speed;

    error_integral += error;

    if (error_integral > 3200) {
        error_integral = 3200;
    } else if (error_integral < 0) {
        error_integral = 0;
    }

    int32_t raw_output = (KP * error) + (KI * error_integral);

    int16_t output = (int16_t) (raw_output >> 5); // Divides by 32

    if (output > 100) {
        output = 100;
    } else if (output < 0) {
        output = 0;
    }

    pwm_setDutyCycle(output);
    duty_cycle = output;

    if (ADC1->ISR & ADC_ISR_EOC) {
        adc_value = ADC1->DR;
    }
    __enable_irq();
}