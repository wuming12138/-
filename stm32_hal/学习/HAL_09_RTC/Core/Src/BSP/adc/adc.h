#ifndef __ADC_H
#define __ADC_H

#include "main.h"

extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc1;


void ADC1_Init(void);
void ADC2_Init(void);
uint16_t GetADC1(void);
uint16_t GetADC2(void);

#endif

