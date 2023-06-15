#ifndef __SEG_H__
#define __SEG_H__

#include "main.h"

#define		SER_L					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET)
#define		SER_H					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET)

#define		RCK_L					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET)
#define		RCK_H					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET)

#define		SCK_L					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define		SCK_H					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)


void Seg_Init(void);
void Seg_Display_Value(uint8_t Bit1, uint8_t Bit2, uint8_t Bit3);


#endif

