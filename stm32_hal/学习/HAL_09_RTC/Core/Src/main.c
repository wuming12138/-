#include "main.h"
#include "led\led.h"
#include "key\key.h"
#include "lcd\lcd.h"
#include "usart\uart.h"
#include "iic\i2c.h"
#include "adc\adc.h"
#include "tim\tim.h"
#include "rtc\rtc.h"
#include "string.h"
#include "stdio.h"

void SystemClock_Config(void);


void Key_Proc(void);
void LCD_Proc(void);
void USART1_Proc(void);
	
//Proc
uint32_t				uwTick_Key_Set_Point = 0;
uint32_t				uwTick_LCD_Set_Point = 0;
uint32_t				uwTick_USART1_Set_Point = 0;

//KEY
uint8_t					Key_Val;
uint8_t					Key_Down;
uint8_t					Key_Up;
uint8_t					Key_Old;

//LCD
char						str[21] = {0};
uint32_t				i = 13254;

//UART
char						TxDate[50] = {0};
unsigned char		RxDate;
uint32_t				count = 0;

//IIC
unsigned char		IIC_DATE1[5] = {1,2,3,4,5};
unsigned char		IIC_DATE2[4] = {0};
unsigned char		MCP4017_Date;
unsigned int *	pDate = &i; 

//TIM
uint16_t				PWM1_IC;
uint16_t				PWM2_IC;
uint16_t				PWM1_IC_t;
uint16_t				PWM2_IC_t;
float						PWM1_Proportion;
float						PWM2_Proportion;
uint8_t					PWM15_Duty1;

//RTC
RTC_DateTypeDef	RTC_Date;
RTC_TimeTypeDef	RTC_Time;


int main(void)
{

	//系统初始化
  HAL_Init();
  SystemClock_Config();
	
	//外设初始化
  LED_Init();
	Key_Init();
	USART1_Init();
	LCD_Init();
	I2CInit();
	ADC1_Init(); 
	ADC2_Init(); 
	TIM3_PWM_IC_Init();
	TIM2_PWM_IC_Init();
	TIM15_PWM_Init();
	TIM4_OC_Init();
	RTC_Init();
	
	
	//UART
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//IIC
	I2C_MCP4017_Write(0x0D);
	MCP4017_Date = I2C_MCP4017_Read();
	I2C_24C02_Read(IIC_DATE2, 0, 4);
	
	//TIM
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);
	
	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_2);
	
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_2);
	
	//RTC
	HAL_RTC_Init(&hrtc);
	
	
  while (1)
  {
		Key_Proc();
		LCD_Proc();
//		USART1_Proc();
  }

}



void Key_Proc(void)
{
	if((uwTick - uwTick_Key_Set_Point) < 100)	
		return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	if(Key_Val == 1)
	{
		PWM15_Duty1++;
		if(PWM15_Duty1>=100)
		{
			PWM15_Duty1 = 0;
		}
		__HAL_TIM_SetCompare(&htim15, TIM_CHANNEL_1, PWM15_Duty1);
	}
	if(Key_Up == 2)
	{

		LED_Disp(0x00);
	}
	if(Key_Up == 3)
	{
		LED_Disp(0xFF);

		I2C_24C02_Write((unsigned char * )pDate, 0, 4);
	}	
	if(Key_Up == 4)
	{
		LED_Disp(0x00);
		I2C_24C02_Read(IIC_DATE2, 0, 4);
		i = ((unsigned int)IIC_DATE2[0]) | ((unsigned int)IIC_DATE2[1]<<8) | ((unsigned int)IIC_DATE2[2]<<16) | ((unsigned int)IIC_DATE2[3]<<24);
	}	
}



void LCD_Proc(void)
{
	if((uwTick - uwTick_LCD_Set_Point) < 100)		return;
	uwTick_LCD_Set_Point = uwTick;

	i++;
	
	HAL_RTC_GetTime(&hrtc, &RTC_Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &RTC_Date, RTC_FORMAT_BIN);
	
	
	sprintf(str, " %02d:%02d:%02d", RTC_Time.Hours, RTC_Time.Minutes, RTC_Time.Seconds);
	LCD_DisplayStringLine(Line0, (unsigned char *)str);
	sprintf(str, " The i is %-3d", (int)i);
	LCD_DisplayStringLine(Line1, (unsigned char *)str);
	sprintf(str, " %x,%x,%x,%x", IIC_DATE2[0],IIC_DATE2[1],IIC_DATE2[2],IIC_DATE2[3]);
	LCD_DisplayStringLine(Line2, (unsigned char *)str);
	sprintf(str, " 4017:%.3f", 0.7874 * MCP4017_Date);
	LCD_DisplayStringLine(Line3, (unsigned char *)str);
	sprintf(str, " R37:%4d  %3.1fv", GetADC2(), (GetADC2()/4095.0)*3.3);
	LCD_DisplayStringLine(Line4, (unsigned char *)str);	
	sprintf(str, " R38:%4d  %3.1fv", GetADC1(), (GetADC1()/4095.0)*3.3);
	LCD_DisplayStringLine(Line5, (unsigned char *)str);
	sprintf(str, " R39:%06dhz,%02.1f%%", (unsigned int)(1000000 / PWM1_IC), PWM1_Proportion*100);
	LCD_DisplayStringLine(Line6, (unsigned char *)str);	
	sprintf(str, " R40:%06dhz,%02.1f%%", (unsigned int)(1000000 / PWM2_IC), PWM2_Proportion*100);
	LCD_DisplayStringLine(Line7, (unsigned char *)str);	
	sprintf(str, " PA2:%d", PWM15_Duty1);
	LCD_DisplayStringLine(Line8, (unsigned char *)str);	
}



void USART1_Proc(void)
{
	if((uwTick - uwTick_USART1_Set_Point) < 1000)		return;
	uwTick_USART1_Set_Point = uwTick;
	
//	sprintf(TxDate, "%04d:hello world\r\n", count); 
//	HAL_UART_Transmit(&huart1, (uint8_t * )TxDate, strlen(TxDate), 50);
//	if(++count == 10000)
//	{
//		count = 0;
//	}

	
	HAL_UART_Transmit(&huart1, &RxDate, 1, 50);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	LED_Disp(0xFF);
	HAL_Delay(300);
	LED_Disp(0x00);
	
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	HAL_UART_Transmit(&huart1, &RxDate, 1, 50);
	
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
  {
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM1_IC = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
			PWM1_Proportion = (float)PWM1_IC_t/PWM1_IC;
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			PWM1_IC_t = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) + 1;
			
		}
	}
	else if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			PWM2_IC = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1) + 1;
			PWM2_Proportion = (float)PWM2_IC_t/PWM2_IC;

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
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_1, (__HAL_TIM_GetCounter(htim) + 100));
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			__HAL_TIM_SetCompare(htim, TIM_CHANNEL_2, (__HAL_TIM_GetCounter(htim) + 500));
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
