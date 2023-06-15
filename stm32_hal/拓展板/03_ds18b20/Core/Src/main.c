#include "main.h"
#include "led\led.h"
#include "key\key.h"
#include "lcd\lcd.h"
//#include "usart\uart.h"
//#include "iic\i2c.h"
#include "adc\adc.h"
//#include "tim\tim.h"
//#include "rtc\rtc.h"
#include "seg\seg.h"
#include "ds18b20\ds18b20_hal.h"
#include "string.h"
#include "stdio.h"

void SystemClock_Config(void);


void Key_Proc(void);
void LCD_Proc(void);

uint32_t				uwTick_LCD_Set_Point;
uint32_t				uwTick_Key_Set_Point;

uint8_t 				Key_Val;
uint8_t 				Key_Down;
uint8_t 				Key_Up;
uint8_t 				Key_Old;

char						str[21];

uint8_t				i;

int main(void)
{

	//系统初始化
  HAL_Init();
  SystemClock_Config();
	
	//外设初始化
  LED_Init();
	Key_Init();
	LCD_Init();
	SEG_Init();
	ADC2_Init();
	ds18b20_init_x();
	
	//LCD
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
  while (1)
  {
		Key_Proc();
		LCD_Proc();
  }

}



void Key_Proc(void)
{
	if((uwTick - uwTick_Key_Set_Point) < 100)	
		return;
	uwTick_Key_Set_Point = uwTick;
	
//	Key_Val = Key_Scan();
	Key_Val = Read_Btn();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	if(Key_Val == 1)
	{

	}
	if(Key_Up == 2)
	{

		LED_Disp(0x00);
	}
	if(Key_Up == 3)
	{
		LED_Disp(0xFF);

	}	
	if(Key_Up == 4)
	{
	}
}



void LCD_Proc(void)
{
	if((uwTick - uwTick_LCD_Set_Point) < 100)		return;
	uwTick_LCD_Set_Point = uwTick;

	i++;
	Seg_Display_Num(i);
	
	sprintf(str, "ADC_KEY:%4d, %d", Get_Btn(), Read_Btn());
	LCD_DisplayStringLine(Line0, (unsigned char *)str);

	sprintf(str, "Temp:%.3f", ds18b20_read() / 16.0);
	LCD_DisplayStringLine(Line1, (unsigned char *)str);
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
