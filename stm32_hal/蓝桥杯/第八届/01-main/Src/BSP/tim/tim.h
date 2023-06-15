#ifndef __TIM_H__
#define __TIM_H__


#include "main.h"


extern TIM_HandleTypeDef htim16;
extern TIM_HandleTypeDef htim17;

void PA6_PWM_Init(void);
void PA7_PWM_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#endif
