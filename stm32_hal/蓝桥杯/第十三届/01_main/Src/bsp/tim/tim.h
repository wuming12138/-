#ifndef __TIM_H__
#define __TIM_H__


#include "main.h"

extern TIM_HandleTypeDef htim2;


void TIM2_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


#endif
