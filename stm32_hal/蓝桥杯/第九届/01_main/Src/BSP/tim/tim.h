#ifndef __TIM_H__
#define __TIM_H__


#include "main.h"


extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;

void TIM3_PWM_Init(void);
void TIM6_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


#endif
