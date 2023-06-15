#ifndef __ADC_H
#define __ADC_H

#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

void ADC1_Init(void);
void ADC2_Init(void);

uint16_t ADC1_GetValue(void);
uint16_t ADC2_GetValue(void);

#endif
