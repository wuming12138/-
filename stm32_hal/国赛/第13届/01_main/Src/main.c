#include "main.h"
#include "key_led\key_led.h"
#include "adc\adc.h"
#include "tim\tim.h"
#include "uart\uart.h"
#include "iic\i2c_hal.h"
#include "lcd\lcd.h"
#include "stdio.h"
#include "string.h"

void SystemClock_Config(void);

void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);

uint32_t											uwTick_Key_Set_Point;
uint32_t											uwTick_Lcd_Set_Point;
uint32_t											uwTick_Uart_Set_Point;

//Key
uint8_t												Key_Val;
uint8_t												Key_Down;
uint8_t												Key_Up;
uint8_t												Key_Old;

//LCD
char													LCD_ShowStr[21];


//main
uint8_t												PWM_Mode;
uint8_t												AO_Flag;
uint8_t												Mode;
uint8_t												REC_Mode;
uint32_t											REC_N_Zore_Time;
uint8_t												N;
float 												AO4;
float 												AO5;
float 												AO4_sum;
float 												AO5_sum;
float 												AO4_A;
float 												AO5_A;
float 												AO4_T;
float 												AO5_T;
float 												AO4_H;
float 												AO5_H;
float 												REC_AO4[100];
float 												REC_AO5[100];
uint8_t												X = 1;
uint8_t												Y = 1;

uint32_t											OC_f;
uint32_t											Tf;
uint32_t											f;



int main(void)
{

  HAL_Init();
  SystemClock_Config();

	MX_GPIO_Init();
	LCD_Init();
	ADC2_Init();
	TIM2_Init();
	TIM3_Init();
	
	LCD_Clear(Black);
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	
	LCD_CtrlLinesConfig();
	
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);

  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
		
  }

}

void Key_Proc(void)
{
	int i = 0;
	if(uwTick - uwTick_Key_Set_Point < 100)	return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	if(Key_Down == 1)
	{
		Mode++;
		Mode %= 3;
		LCD_Clear(Black);
		
		if(Mode == 2)
		{
			AO4_A = 0;
			AO5_A = 0;	
			AO4_T = REC_AO4[0];
			AO5_T = REC_AO5[0];
			for(i = 0; i < N; i++)
			{
				
				if(REC_AO4[i] >= AO4_A)
					AO4_A = REC_AO4[i];

				if(REC_AO5[i] >= AO5_A)
					AO5_A = REC_AO5[i];
				
				
				if(REC_AO4[i] <= AO4_T)
					AO4_T = REC_AO4[i];
				
				if(REC_AO5[i] <= AO5_T)
					AO5_T = REC_AO5[i];			

				AO4_sum += REC_AO4[i];
				AO5_sum += REC_AO5[i];
				
				AO4_H = AO4_sum / N;
				AO5_H = AO5_sum / N;
				
				
				REC_Mode = 0;
			}
		}
	}
	
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			
			X %= 4;
			X++;
		}
		if(Key_Down == 3)
		{
			
			Y %= 4;
			Y++;
		}		
	}

	if(Key_Down == 4)
	{
		if(Mode == 0)
		{
			AO_Flag = 1;
			
			if(N >= 100)	
				N = 0;
		}
		else if(Mode == 1)
		{
			PWM_Mode++;
			PWM_Mode %= 2;
		}
		else if(Mode == 2)
		{
			REC_Mode++;
			REC_Mode %= 2;
			REC_N_Zore_Time = uwTick;
		}
		

	}	
	if(Key_Up == 4)
	{
		if(uwTick - REC_N_Zore_Time > 1000 && Mode == 2)
		{
			N = 0;
			AO4_sum = 0;
			AO5_sum = 0;
			AO4_A = 0;
			AO5_A = 0;
			AO4_H = 0;
			AO5_H = 0;
			AO4_T = 0;
			AO5_T = 0;			
		}
	}
	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)	return;
	uwTick_Lcd_Set_Point = uwTick;
	
	if(AO_Flag == 1)
	{
		AO4 = ADC_GetAO4() * 3.3 / 4095; 
		AO5 = ADC_GetAO5() * 3.3 / 4095; 
		REC_AO4[N] = AO4;
		REC_AO5[N] = AO5;
		
		N++;
		AO_Flag = 0;
	}
	
	if(Mode == 0)
	{
		sprintf(LCD_ShowStr, "        DATA     ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "     PA4:%.2f    ", AO4);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);		
		sprintf(LCD_ShowStr, "     PA5:%.2f    ", AO5);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);			
		sprintf(LCD_ShowStr, "     PA1:%d      ", f);
		LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);			
	}
	else if(Mode == 1)
	{
		sprintf(LCD_ShowStr, "        PARA     ");
		LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);		
		
		sprintf(LCD_ShowStr, "     X:%d      ", X);
		LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);	
		sprintf(LCD_ShowStr, "     Y:%d      ", Y);
		LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);		
	}
	else if(Mode == 2)
	{
		if(REC_Mode == 0)
		{
			sprintf(LCD_ShowStr, "        REC_PA4 ");
			LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);	
			
			sprintf(LCD_ShowStr, "     N:%d      ", N);
			LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);	
			sprintf(LCD_ShowStr, "     A:%.2f      ", AO4_A);
			LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);			
			sprintf(LCD_ShowStr, "     T:%.2f      ", AO4_T);
			LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);	
			sprintf(LCD_ShowStr, "     H:%.2f      ", AO4_H);
			LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);	
			
		}
		else if(REC_Mode == 1)
		{
			sprintf(LCD_ShowStr, "        REC_PA5 ");
			LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);		
			
			sprintf(LCD_ShowStr, "     N:%d      ", N);
			LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);	
			sprintf(LCD_ShowStr, "     A:%.2f      ", AO5_A);
			LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);			
			sprintf(LCD_ShowStr, "     T:%.2f      ", AO5_T);
			LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);	
			sprintf(LCD_ShowStr, "     H:%.2f      ", AO5_H);
			LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);		
		}
	}
}

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 200)	return;
	uwTick_Uart_Set_Point = uwTick;
	
}
	

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			Tf = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
			f = 1000000 / Tf;
		}
	}
	
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			OC_f = Tf / 2;
			
			if(PWM_Mode == 0)
			{
				OC_f = OC_f / X;
			}
			if(PWM_Mode == 1)
			{
				OC_f = OC_f * X;
			}
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_2, __HAL_TIM_GetCounter(htim) + OC_f);
		}
	}

}



















/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }	
}


/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
