#ifndef __ADC_H
#define __ADC_H

#include "main.h"

#define			ADC_RP5					ADC_CHANNEL_17
#define			ADC_RP6					ADC_CHANNEL_13
#define			ADC_R37					ADC_CHANNEL_15

extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc1;


void ADC1_Init(void);
void ADC2_Init(void);
uint16_t GetADC1(void);
uint16_t GetADC2(void);

uint16_t Get_Btn(void);
uint8_t Read_Btn(void);

uint16_t Get_ADC2_Channel(uint32_t Channel);


#endif

