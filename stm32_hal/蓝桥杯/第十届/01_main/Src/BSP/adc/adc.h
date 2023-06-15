#ifndef __ADC_H__
#define __ADC_H__


#include "main.h"

extern ADC_HandleTypeDef hadc2;


void ADC2_Init(void);
uint16_t ADC_GetValue(void);

#endif
