#ifndef __GPIO_H__
#define __GPIO_H__


#include "main.h"


void Key_Led_Init(void);
void LED_Disp(uint8_t uwLED);
uint8_t Key_Scan(void);

#endif
