#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "adc\adc.h"
#include "lcd\lcd.h"
#include "tim\tim.h"


void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
	
//Point
uint32_t								uwTick_Key_Set_Point;
uint32_t								uwTick_Lcd_Set_Point;

//Key
uint8_t									Key_Val;
uint8_t									Key_Down;
uint8_t									Key_Up;
uint8_t									Key_Old;

//LCD
char										LCD_ShowStr[21];

//main
float										Volt;
float										Volt_Old;
float										Volt_Max = 3.0;
float										Volt_Min = 1.0;
uint32_t								Time;
uint8_t									Mode;
uint8_t									Time_Start_Flag;
uint8_t									uwLED;

int main(void)
{

  HAL_Init();
	SystemClock_Config();
	
	Key_Led_Init();
	ADC2_Init();
	LCD_Init();
	TIM6_Init();

	//lcd
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	

  while (1)
  {
		
		Key_Proc();
		Lcd_Proc();
  }
}

//Proc
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
		if(Mode == 0)
		{
			Mode = 1;
			
		}
		else if(Mode == 1)
		{
			Mode = 0;
			if(Volt_Max >= Volt_Min + 1)
			{
				uwLED &= ~0x02;
			}
			else
			{
				uwLED |= 0x02;
				Volt_Max = 3.0;
				Volt_Min = 1.0;
			}
		}
	}
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			Volt_Max+=0.1f;
			if(Volt_Max > 3.3f)
			{
				Volt_Max = 0;
			}
		}
		if(Key_Down == 3)
		{
			Volt_Min+=0.1f;
			if(Volt_Min > 3.3f)
			{
				Volt_Min = 0;
			}
		}
	}
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	if(Time_Start_Flag == 0)
	{
		HAL_TIM_Base_Stop_IT(&htim6);
		uwLED &= ~0x01;
	}
	if(Time_Start_Flag == 1)
	{
		HAL_TIM_Base_Start_IT(&htim6);
		Time = 0;
		Time_Start_Flag = 2;
	}
	if(Time_Start_Flag == 2)
	{
		uwLED |= 0x01;
	}
	
	LED_Disp(uwLED);
	if(Mode == 0)
	{
		Volt_Old = Volt;
		Volt = ADC_GetValue()*3.3/4095;
		if(Volt >= Volt_Min - 0.05f && Volt <= Volt_Min + 0.05f  && Volt > Volt_Old)
		{
			Time_Start_Flag = 1;
		}
		else if(Volt >= Volt_Max - 0.05f && Volt <= Volt_Max + 0.05f && Volt > Volt_Old)
		{
			Time_Start_Flag = 0;
		}
		sprintf(LCD_ShowStr, "      Data       ");
		LCD_DisplayStringLine(Line0,(unsigned char *)LCD_ShowStr);	
		
		sprintf(LCD_ShowStr, " V:%.2fV         ", Volt);
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
		sprintf(LCD_ShowStr, " T:%ds           ", Time);
		LCD_DisplayStringLine(Line3,(unsigned char *)LCD_ShowStr);
	}
	else if(Mode == 1)
	{
		
		sprintf(LCD_ShowStr, "      Para       ");
		LCD_DisplayStringLine(Line0,(unsigned char *)LCD_ShowStr);	
		
		sprintf(LCD_ShowStr, " Vmax:%.1fV      ", Volt_Max);
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
		sprintf(LCD_ShowStr, " Vmin:%.1fV      ", Volt_Min);
		LCD_DisplayStringLine(Line3,(unsigned char *)LCD_ShowStr);
	}
}



//Callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	Time++;
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
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
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
