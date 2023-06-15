#ifndef __ADC_H
#define __ADC_H

#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void ADC1_Init(void);
uint16_t GetADC1(void);

#endif

