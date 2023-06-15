#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "iic\i2c_hal.h"
#include "uart\uart.h"
#include "adc\adc.h"
#include "rtc\rtc.h"

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);
void UART_Proc(void);

//uwTick
uint32_t 						uwTick_Key_Set_Point;
uint32_t 						uwTick_Lcd_Set_Point;
uint32_t 						uwTick_Led_Set_Point;
uint32_t						uwTick_Uart_Set_Point;
uint32_t						uwTick_Dodge_Set_Point;
uint32_t						UART_Tim;

//Key
uint8_t							Key_Val;
uint8_t							Key_Down;
uint8_t							Key_Up;
uint8_t							Key_Old;

//lcd
char								LCD_ShowString[21];

//UART
uint8_t							Receive_Date[10];
uint8_t							UART_State_Flag = 0;
char								UART_ShowString[21];
uint8_t							Rx_Buf[100];
uint8_t							pRx_Buf;
uint8_t							Rx_Buffer;

//IIC
uint8_t							AT24C02_Write;
uint8_t							AT24C02_Read;

//RTC
RTC_TimeTypeDef 		sTime = {0};
RTC_DateTypeDef 		sDate = {0};



//main
float								k = 0.1;
float								V1;
uint8_t							LED_State_Flag = 1;
uint8_t							LED_ON_Flag = 0;
uint8_t							Mode = 0;
uint8_t							Toggle;

uint8_t							Alarm_Disp[3];
uint8_t							Alarm_Ctrl[3];
uint8_t							pAlarm = 0;
uint8_t							Alarm_Disp_Select_Flag;
uint8_t							UART_Flag = 0;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
	
	LCD_Init();
	Key_Led_Init();
	I2CInit();
	UART_Init();
	ADC2_Init();
	RTC_Init();
	
	//LCD
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//UART
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	

	
	
  while (1)
  {
		Key_Proc();
		Led_Proc();
		Lcd_Proc();
		UART_Proc();
  }

}

//Proc
void Key_Proc(void)
{
	if((uwTick - uwTick_Key_Set_Point) < 100)		return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	if(Mode == 0)
	{
		if(Key_Up == 1)
		{
			LED_State_Flag++;
			LED_State_Flag %= 2;
			
		}
		if(Key_Up == 2)
		{
			Mode = 1;
			
			
			LCD_DisplayStringLine(Line2, "                 ");
			LCD_DisplayStringLine(Line3, "                 ");
			LCD_DisplayStringLine(Line4, "                 ");
			LCD_DisplayStringLine(Line5, "                 ");
			LCD_DisplayStringLine(Line6, "                 ");
			
		}
	}
	else if(Mode == 1)
	{
		if(Key_Up == 1)
		{
			LED_State_Flag++;
			LED_State_Flag %= 2;
			
		}
		if(Key_Up == 2)
		{
			Mode = 0;
			UART_Flag = 0;
			Alarm_Ctrl[0] = Alarm_Disp[0];
			Alarm_Ctrl[1] = Alarm_Disp[1];
			Alarm_Ctrl[2] = Alarm_Disp[2];
			LCD_DisplayStringLine(Line2, "                 ");
			LCD_DisplayStringLine(Line3, "                 ");
			LCD_DisplayStringLine(Line4, "                 ");
			LCD_DisplayStringLine(Line5, "                 ");
			LCD_DisplayStringLine(Line6, "                 ");
			
		}
		if(Key_Up == 3)
		{
			pAlarm++;
			pAlarm%=3;
		}
		if(Key_Val == 4)
		{

			Alarm_Disp[pAlarm]++;
			if(pAlarm == 0 && Alarm_Disp[pAlarm] == 24)
			{
				Alarm_Disp[pAlarm] = 0;
			}
			else if(Alarm_Disp[pAlarm] == 60)
			{
				Alarm_Disp[pAlarm] = 0;
			}
		}
	}
	
}


void Lcd_Proc(void)
{
	if((uwTick - uwTick_Lcd_Set_Point) < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	if(Mode == 0)
	{
		k = IIC_24C02_ReadDate(0) / 10.0;
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		V1 = ADC_GetValue()*3.3/4095;
		
		if(V1 > (float)3.3*k)
		{
			LED_ON_Flag = 1;
		}
		else
		{
			LED_ON_Flag = 0;
		}
		 
		sprintf(LCD_ShowString, "   V1:%.2fV         ", V1);
		LCD_DisplayStringLine(Line2, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "   k:%.1f           ", k);
		LCD_DisplayStringLine(Line3, (unsigned char *)LCD_ShowString);
		if(LED_State_Flag == 1)
		{
			sprintf(LCD_ShowString, "   LED:ON         ");
			LCD_DisplayStringLine(Line4, (unsigned char *)LCD_ShowString);
		}
		else
		{
			sprintf(LCD_ShowString, "   LED:OFF        ");
			LCD_DisplayStringLine(Line4, (unsigned char *)LCD_ShowString);
		}
		sprintf(LCD_ShowString, "   T:%02d-%02d-%02d ", sTime.Hours, sTime.Minutes, sTime.Seconds);
		LCD_DisplayStringLine(Line5, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "   %d %d %d %d      ", Receive_Date[0], Receive_Date[1], Receive_Date[2], UART_State_Flag);
		LCD_DisplayStringLine(Line6, (unsigned char *)LCD_ShowString);
	}
	else if(Mode == 1)
	{
		V1 = ADC_GetValue()*3.3/4095;
		if(V1 > (float)3.3*k)
		{
			LED_ON_Flag = 1;
		}
		else
		{
			LED_ON_Flag = 0;
		}
		sprintf(LCD_ShowString, "     Setting: ");
		LCD_DisplayStringLine(Line2, (unsigned char *)LCD_ShowString);
		sprintf(LCD_ShowString, "     %02d-%02d-%02d", Alarm_Disp[0], Alarm_Disp[1], Alarm_Disp[2]);
		if((uwTick - uwTick_Dodge_Set_Point) > 500)
		{
			uwTick_Dodge_Set_Point = uwTick;
			Alarm_Disp_Select_Flag++;
			Alarm_Disp_Select_Flag %= 2;
		}
		if(pAlarm == 0 && Alarm_Disp_Select_Flag == 1)
		{
			LCD_ShowString[5] = ' ';
			LCD_ShowString[6] = ' ';
		}
		else if(pAlarm == 1 && Alarm_Disp_Select_Flag == 1)
		{
			LCD_ShowString[8] = ' ';
			LCD_ShowString[9] = ' ';
		}
		else if(pAlarm == 2 && Alarm_Disp_Select_Flag == 1)
		{
			LCD_ShowString[11] = ' ';
			LCD_ShowString[12] = ' ';
		}
		LCD_DisplayStringLine(Line4, (unsigned char *)LCD_ShowString);
		
	}
}


void Led_Proc(void)
{
	if((uwTick - uwTick_Led_Set_Point) < 200)		return;
	uwTick_Led_Set_Point = uwTick;
	
	if(LED_ON_Flag == 1 && LED_State_Flag == 1)
	{
		Toggle++;
		Toggle %= 2;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_All, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, Toggle);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	}
	else
	{
		LED_Disp(0x00);
	}
}


void UART_Proc(void)
{
	if((uwTick - uwTick_Uart_Set_Point) < 100)		return;
	uwTick_Uart_Set_Point = uwTick;
	
	if((uwTick - UART_Tim) <= 600 && (uwTick - UART_Tim) >=200)
	{
		if(pRx_Buf == 6)
		{
			if(Rx_Buf[0] == 0x6B && Rx_Buf[1] == 0x30 && Rx_Buf[2] == 0x2E && 
				 Rx_Buf[3] > 0x30 && Rx_Buf[3] < 0x3a && Rx_Buf[4] == 0x5C && Rx_Buf[5] == 0x6E)
			{
				HAL_UART_Transmit(&huart1, "OK", 2, 50);
				k = (Rx_Buf[3] - 48)/10.0;
				AT24C02_Write = Rx_Buf[3] - 48;
				IIC_24C02_WriteDate(AT24C02_Write, 0);
			}
		}
		UART_State_Flag = 0;
		pRx_Buf = 0;
		
	}
	if(Alarm_Ctrl[0] == sTime.Hours && Alarm_Ctrl[1] == sTime.Minutes 
		&& Alarm_Ctrl[2] == sTime.Seconds && UART_Flag == 0)
	{
		UART_Flag = 1;
		sprintf(UART_ShowString, "%.2f+%.1f+%d%d%d", V1, k, sTime.Hours, sTime.Minutes, sTime.Seconds);
		HAL_UART_Transmit(&huart1, (uint8_t *)UART_ShowString, 21, 50);
	}
}

//Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	UART_State_Flag = 1;
//	HAL_UART_Receive_IT(&huart1, Receive_Date, 6);
	if(Rx_Buffer == 0x6B && pRx_Buf == 0)
	{
		UART_Tim = uwTick;
		UART_State_Flag = 1;
	}
	if(UART_State_Flag == 1)
	{
		Rx_Buf[pRx_Buf] = Rx_Buffer;
		pRx_Buf++;
	}
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */



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
