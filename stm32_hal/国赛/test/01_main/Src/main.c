#include "main.h"
#include "iic\i2c_hal.h"
#include "lcd\lcd.h"
#include "uart\uart.h"
#include "tim\tim.h"
#include "rtc\rtc.h"
#include "adc\adc.h"
#include "key_led\key_led.h"
#include "stdio.h"
#include "string.h"


void SystemClock_Config(void);

void Key_Proc(void);
void Lcd_Proc(void);
void Uart_Proc(void);

uint32_t										uwTick_Key_Set_Point;
uint32_t										uwTick_Lcd_Set_Point;
uint32_t										uwTick_Uart_Set_Point;

//Key
uint8_t											Key_Val;
uint8_t											Key_Down;
uint8_t											Key_Up;
uint8_t											Key_Old;

//Lcd
char												LCD_ShowStr[21];

//Tim
uint16_t										TIM2_T;
uint16_t										TIM2_t;
float												TIM2_Duty;
uint16_t										TIM3_T;
uint16_t										TIM3_t;
float												TIM3_Duty;

//RTC
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

//IIC
uint8_t											W[3] = {18, 17, 15};
uint8_t											R[3];
int8_t											xx, yy, zz;

//Uart
uint8_t											Rx_Buffer;
uint8_t											pRx_Buf;
uint8_t											Rx_Buf[50];
uint8_t											Uart_Flag;
uint32_t										Uart_Time;



int main(void)
{
  HAL_Init();
  SystemClock_Config();
	
	MX_GPIO_Init();
	LCD_Init();
	RTC_Init();
	ADC1_Init();
	ADC2_Init();
	TIM2_Init();
	TIM3_Init();
	TIM4_Init();	
	TIM15_Init();
	I2CInit();
	UART_Init();
	
	
	//lcd
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
//	IIC_AT24C02_Write(W, 0, 5);
//	IIC_AT24C02_Read(R, 0, 5);

	LIS302_Config();
	
	HAL_UART_Receive_IT(&huart1, &Rx_Buffer, 1);
	
	//TIM
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);	

	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
	
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim15, TIM_CHANNEL_1);
  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Uart_Proc();
  }

}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)		return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Val ^ Key_Old);
	Key_Up = ~Key_Val & (Key_Val ^ Key_Old);
	Key_Old = Key_Val;
	
	if(Key_Down == 1)
	{
		LED_Disp(0x88);
	}
	if(Key_Down == 2)
	{
		LED_Disp(0x00);
	}
	if(Key_Down == 3)
	{
		
	}
	if(Key_Down == 4)
	{
		
	}	
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	
	LIS302_Output(&xx, &yy, &zz);
	
	sprintf(LCD_ShowStr, "%d %d %d", R[0], R[1], R[2]);
	LCD_DisplayStringLine(Line0, (uint8_t * )LCD_ShowStr);
	sprintf(LCD_ShowStr, "R37:%4d   %.2f  ", ADC2_GetValue(), ADC2_GetValue() * 3.3 /4095);
	LCD_DisplayStringLine(Line1, (uint8_t * )LCD_ShowStr);
	sprintf(LCD_ShowStr, "R38:%4d   %.2f  ", ADC1_GetValue(), ADC1_GetValue() * 3.3 /4095);
	LCD_DisplayStringLine(Line2, (uint8_t * )LCD_ShowStr);	
	sprintf(LCD_ShowStr, "R39:%6d   %.2f  ", 1000000 / TIM3_T, TIM3_Duty);
	LCD_DisplayStringLine(Line3, (uint8_t * )LCD_ShowStr);		
	sprintf(LCD_ShowStr, "R40:%6d   %.2f  ", 1000000 / TIM2_T, TIM2_Duty);
	LCD_DisplayStringLine(Line4, (uint8_t * )LCD_ShowStr);	
	sprintf(LCD_ShowStr, "  %1.2f  ", xx * 18.0 / 1000);
	LCD_DisplayStringLine(Line5, (uint8_t * )LCD_ShowStr);
	sprintf(LCD_ShowStr, "  %1.2f  ", yy * 18.0 / 1000);
	LCD_DisplayStringLine(Line6, (uint8_t * )LCD_ShowStr);
	sprintf(LCD_ShowStr, "  %1.2f  ", zz * 18.0 / 1000);
	LCD_DisplayStringLine(Line7, (uint8_t * )LCD_ShowStr);	
	
	sprintf(LCD_ShowStr, "%02d-%02d-%02d   ", sTime.Hours, sTime.Minutes, sTime.Seconds);
	LCD_DisplayStringLine(Line9, (uint8_t * )LCD_ShowStr);	
}	

void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 200)		return;
	uwTick_Uart_Set_Point = uwTick;
	
	if(uwTick - Uart_Time >= 200 && uwTick - Uart_Time < 400)
	{
		if(Rx_Buf[0] == 'A' && Rx_Buf[1] == 'T')
		{
			printf("OK\r\n");
		}
		else
		{
			printf("No\r\n");
		}
		
		Uart_Flag = 0;
		pRx_Buf = 0;
	}
}		


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			TIM2_T = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			TIM2_Duty = (float)TIM2_t / TIM2_T;
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			TIM2_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		}
	}
	if(htim->Instance == TIM3)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			TIM3_T = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			TIM3_Duty = (float)TIM3_t / TIM3_T;
		}
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			TIM3_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		}
	}	
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM4)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_1, __HAL_TIM_GetCounter(htim) + 500);
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(pRx_Buf == 0)
	{
		Uart_Flag = 1;
		Uart_Time = uwTick;
	}
	if(Uart_Flag == 1)
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
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
	__HAL_RCC_GPIOF_CLK_ENABLE();
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
