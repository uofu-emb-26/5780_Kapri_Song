#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f0xx.h"

extern volatile int16_t error_integral;
extern volatile uint8_t duty_cycle;
extern volatile int16_t target_rpm;
extern volatile int16_t motor_speed;
extern volatile int8_t adc_value;
extern volatile int16_t error;
extern volatile uint8_t Kp;
extern volatile uint8_t Ki;

void motor_init(void);
void pwm_setDutyCycle(uint8_t duty);
void PI_update(void);

void pwm_init(void);
void encoder_init(void);
void ADC_init(void);

#endif /* MOTOR_H_ */