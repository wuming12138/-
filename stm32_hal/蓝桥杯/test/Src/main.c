#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "uart\uart.h"
#include "iic\i2c_hal.h"
#include "adc\adc.h"
#include "tim\tim.h"
#include "rtc\rtc.h"


void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);

//uwtick
uint32_t				uwTick_Key_Set_Point;
uint32_t				uwTick_Lcd_Set_Point;
uint32_t				uwTick_Uart_Set_Point;

//Key
uint8_t					Key_Val;
uint8_t					Key_Down;
uint8_t					Key_Up;
uint8_t					Key_Old;

//LCD
char						StrShow[21];

//UART
char						TxDate[50];
uint8_t					RxDate;

//IIC
uint8_t					AT24C02Date[5] = {11,22,33,44,55};
uint8_t					AT24C02_Read_Date[5];

//Time
uint32_t				Tim_Count = 0;
uint16_t				PWM1_IC_T;
uint16_t				PWM1_IC_t;
uint16_t				PWM2_IC_T;
uint16_t				PWM2_IC_t;
float						PWM1_Duty;
float						PWM2_Duty;
uint8_t					PWM_PA2;
uint8_t					PWM_PA3;

//RTC
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

int main(void)
{

  HAL_Init();
  SystemClock_Config();

	Key_Led_Init();
	LCD_Init();
	UART_Init();
	I2CInit();
	ADC1_Init();
	ADC2_Init();
	TIM6_Init();
	TIM2_IC_Init();
	TIM3_IC_Init();
	TIM4_OC_Init();
	TIM15_PWM_Init();
	RTC_Init();
	
	//LCD
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//UART
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	
	//24C02
	IIC_24C02_WriteDate(AT24C02Date, 0, 5);
	IIC_24C02_ReadDate(AT24C02_Read_Date, 0, 5);
	
	//TIM
	HAL_TIM_Base_Start_IT(&htim6);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2);
	
	
	HAL_TIM_Base_Start(&htim15);
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);
	
	
	

  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
  }
}


void Key_Proc(void)
{
	if((uwTick - uwTick_Key_Set_Point) < 100)	return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	if(Key_Down == 1)
	{
		LED_Disp(0xFF);
		
	}
	if(Key_Down == 2)
	{
		LED_Disp(0x00);
	}
	if(Key_Val == 3)
	{
		PWM_PA3 += 10;
		if(PWM_PA3 > 100)
		{
			PWM_PA3 = 0;
		}	
		__HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_2, PWM_PA3);
		
	}
}

void Lcd_Proc(void)
{
	if((uwTick - uwTick_Lcd_Set_Point) < 100)	return;
	uwTick_Lcd_Set_Point = uwTick;

	
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);	
	sprintf(StrShow, " AT:%02d%02d%02d%02d%02d, %04d", AT24C02_Read_Date[0], AT24C02_Read_Date[1], AT24C02_Read_Date[2], AT24C02_Read_Date[3], AT24C02_Read_Date[4], Tim_Count);
	LCD_DisplayStringLine(Line0, (unsigned char *)StrShow);
	sprintf(StrShow, " R37:%04d   %.1fV", ADC2_GetValue(), (float)(ADC2_GetValue()*3.3/4095));
	LCD_DisplayStringLine(Line1, (unsigned char *)StrShow);
	sprintf(StrShow, " R38:%04d   %.1fV", ADC1_GetValue(), (float)(ADC1_GetValue()*3.3/4095));
	LCD_DisplayStringLine(Line2, (unsigned char *)StrShow);
	sprintf(StrShow, " R39:%06dHz  %2.1f%%", 1000000/PWM2_IC_T, PWM2_Duty * 100);
	LCD_DisplayStringLine(Line3, (unsigned char *)StrShow);
	sprintf(StrShow, " R40:%06dHz  %2.1f%%", 1000000/PWM1_IC_T, PWM1_Duty * 100);
	LCD_DisplayStringLine(Line4, (unsigned char *)StrShow);
	sprintf(StrShow, " PA2:%03d  PA3:%03d", PWM_PA2, PWM_PA3);
	LCD_DisplayStringLine(Line5, (unsigned char *)StrShow);
	sprintf(StrShow, " 20%d-%d-%d", sDate.Year, sDate.Month, sDate.Date);
	LCD_DisplayStringLine(Line6, (unsigned char *)StrShow);
	sprintf(StrShow, " %d:%d:%d", sTime.Hours, sTime.Minutes, sTime.Seconds);
	LCD_DisplayStringLine(Line7, (unsigned char *)StrShow);
	
}

void Uart_Proc(void)
{
	if((uwTick - uwTick_Uart_Set_Point) < 5000)	return;
	uwTick_Uart_Set_Point = uwTick;
	
	sprintf((char * )TxDate, "Hello world");
	HAL_UART_Transmit(&huart1, (uint8_t * )TxDate, strlen(TxDate), 50);
	
	
	
}




//Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
	
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	HAL_UART_Transmit(&huart1, &RxDate, 1, 50);
	
	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)
	{
		Tim_Count++;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM1_IC_T = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
			PWM1_Duty = (float)PWM1_IC_t / PWM1_IC_T;
		
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM1_IC_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
		}
	}
	else if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM2_IC_T = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
			PWM2_Duty = (float)PWM2_IC_t / PWM2_IC_T;
		
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM2_IC_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_1, __HAL_TIM_GetCounter(htim) + 100);
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_2, __HAL_TIM_GetCounter(htim) + 500);
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
	
	
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
	
}


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
