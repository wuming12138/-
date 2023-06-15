#ifndef __TIM_H
#define __TIM_H
#include "main.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim15;

void TIM2_PWM_IC_Init(void);
void TIM3_PWM_IC_Init(void);
void TIM4_OC_Init(void);
void TIM15_PWM_Init(void);

#endif
