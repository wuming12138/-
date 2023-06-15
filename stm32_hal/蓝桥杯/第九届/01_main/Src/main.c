#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "lcd\lcd.h"
#include "tim\tim.h"
#include "iic\i2c_hal.h"


#define			Standby			0
#define			Running			1
#define			Setting			2
#define			Pause				3


typedef struct
{
	uint8_t hours;
	uint8_t mins;
	uint8_t Secs;
	
}Timetypedef;

Timetypedef						sTime[5] = {{0,0,10},{0,0,30},{0,1,0},{0,2,0},{0,5,0}};
Timetypedef						Time_Ctrl;
uint8_t								Time_Disp[3];
uint8_t								pTime_Disp;

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);

//Point
uint32_t							uwTick_Key_Set_Point;
uint32_t							uwTick_Key2_Tim_Point;
uint32_t							uwTick_Key3_Tim_Point;
uint32_t							uwTick_Key4_Tim_Point;
uint32_t							uwTick_Lcd_Set_Point;
uint32_t							uwTick_Led_Set_Point;

//Key
uint8_t								Key_Val;
uint8_t								Key_Down;
uint8_t								Key_Up;
uint8_t								Key_Old;

//Lcd
char									LCD_ShowStr[21];


//main
uint8_t								pTime;
uint8_t								Mode = Standby;
uint8_t								pTime_Disp_S;
uint8_t								uwLED;
int main(void)
{

  HAL_Init();
	SystemClock_Config();
	Key_Led_Init();
	LCD_Init();
	TIM3_PWM_Init();
	TIM6_Init();
	I2CInit();
	
	//LCD
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//PWM
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
	

	
	IIC_AT24C02_Read((uint8_t * )&sTime[0], 0, 3);
	IIC_AT24C02_Read((uint8_t * )&sTime[1], 3, 3);
	IIC_AT24C02_Read((uint8_t * )&sTime[2], 6, 3);
	IIC_AT24C02_Read((uint8_t * )&sTime[3], 9, 3);
	IIC_AT24C02_Read((uint8_t * )&sTime[4], 12, 3);
	
  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Led_Proc();
  }

}

//Proc
void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 250)		return;
	uwTick_Led_Set_Point = uwTick;
	
	if(Mode == Running)
	{
		uwLED ^= 0x01;
		HAL_TIM_Base_Start_IT(&htim6);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	}
	else
	{
		uwLED = 0;
		HAL_TIM_Base_Stop_IT(&htim6);
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
	}
	
	LED_Disp(uwLED);
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
		pTime++;
		pTime %= 5;
		
	}
	if(Key_Down == 2)
	{
		uwTick_Key2_Tim_Point = uwTick;
	}
	if(Key_Up == 2)
	{
		if(uwTick - uwTick_Key2_Tim_Point < 800)
		{
			if(Mode == Setting)
			{
				pTime_Disp++;
				pTime_Disp%=3;
			}
			Mode = Setting;
		}
		else if(uwTick - uwTick_Key2_Tim_Point >= 800) 
		{
			if(Mode == Setting)
			{
				sTime[pTime].hours = Time_Disp[0];
				sTime[pTime].mins = Time_Disp[1];
				sTime[pTime].Secs = Time_Disp[2];
				IIC_AT24C02_Write((uint8_t * )&sTime[pTime], pTime*3, 3);
				Mode = Standby;
			}
		}
	}
	if(Key_Down == 3)
	{
		Time_Disp[pTime_Disp]++;
		uwTick_Key3_Tim_Point = uwTick;
		if(pTime_Disp == 0)
		{
			if(Time_Disp[pTime_Disp] > 23)
			{
				Time_Disp[pTime_Disp] = 0;
			}
		}
		else
		{
			if(Time_Disp[pTime_Disp] > 59)
			{
				Time_Disp[pTime_Disp] = 0;
			}
		}
	}
	if(Key_Val == 3)
	{
		if(uwTick - uwTick_Key3_Tim_Point > 800)
		{
			Time_Disp[pTime_Disp]++;
		}
		if(pTime_Disp == 0)
		{
			if(Time_Disp[pTime_Disp] > 23)
			{
				Time_Disp[pTime_Disp] = 0;
			}
		}
		else
		{
			if(Time_Disp[pTime_Disp] > 60)
			{
				Time_Disp[pTime_Disp] = 0;
			}
		}
	}
	if(Key_Down == 4)
	{
		uwTick_Key4_Tim_Point = uwTick;
	}
	if(Key_Up == 4)
	{
		if(uwTick - uwTick_Key4_Tim_Point < 800)
		{
			if(Mode == Running)
			{
				Mode = Pause;
			}
			else if(Mode == Pause)
			{
				Mode = Running;
			}
			else if(Mode == Standby)
			{
				Mode = Running;
				Time_Ctrl.mins = sTime[pTime].mins;
				Time_Ctrl.Secs = sTime[pTime].Secs;
				Time_Ctrl.hours = sTime[pTime].hours;
			}
			else if(Mode == Setting)
			{
				Mode = Running;
				Time_Ctrl.hours = Time_Disp[0];
				Time_Ctrl.mins = Time_Disp[1];
				Time_Ctrl.Secs = Time_Disp[2];
			}
				
		}
		else if(uwTick - uwTick_Key4_Tim_Point >= 800) 
		{
			Mode = Standby;
		}
	}
}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	
	if(Mode == Standby)
	{
		sprintf(LCD_ShowStr, "   No %d", pTime+1);
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
		sprintf(LCD_ShowStr, "      %02d:%02d:%02d    ", sTime[pTime].hours, sTime[pTime].mins, sTime[pTime].Secs);
		LCD_DisplayStringLine(Line5,(unsigned char *)LCD_ShowStr);	
		
		sprintf(LCD_ShowStr, "      Standby");
		LCD_DisplayStringLine(Line8,(unsigned char *)LCD_ShowStr);
	}
	else if(Mode == Setting)
	{
		pTime_Disp_S++;
		pTime_Disp_S%=2;
		sprintf(LCD_ShowStr, "   No %d", pTime+1);
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
		
		sprintf(LCD_ShowStr, "      %02d:%02d:%02d     ", Time_Disp[0], Time_Disp[1], Time_Disp[2]);
		if(pTime_Disp == 0 && pTime_Disp_S == 1)
		{
			LCD_ShowStr[6] = ' ';
			LCD_ShowStr[7] = ' ';
		}
		else if(pTime_Disp == 1 && pTime_Disp_S == 1)
		{
			LCD_ShowStr[9] = ' ';
			LCD_ShowStr[10] = ' ';
		}
		else if(pTime_Disp == 2 && pTime_Disp_S == 1)
		{
			LCD_ShowStr[12] = ' ';
			LCD_ShowStr[13] = ' ';
		}
		LCD_DisplayStringLine(Line5,(unsigned char *)LCD_ShowStr);	
		
		sprintf(LCD_ShowStr, "      Setting");
		LCD_DisplayStringLine(Line8,(unsigned char *)LCD_ShowStr);
	}
	else if(Mode == Running || Mode == Pause)
	{
		if(Time_Ctrl.Secs > 200)
		{
			Time_Ctrl.Secs = 59;
			Time_Ctrl.mins--;
			if(Time_Ctrl.mins > 200)
			{
				Time_Ctrl.mins = 59;
				Time_Ctrl.hours--;
			}
		}
		
		sprintf(LCD_ShowStr, "   No %d", pTime+1);
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
		sprintf(LCD_ShowStr, "      %02d:%02d:%02d     ", Time_Ctrl.hours, Time_Ctrl.mins, Time_Ctrl.Secs);
		LCD_DisplayStringLine(Line5,(unsigned char *)LCD_ShowStr);	

		
		if(Mode == Running)
		{
			sprintf(LCD_ShowStr, "      Running");
			LCD_DisplayStringLine(Line8,(unsigned char *)LCD_ShowStr);
		}
		else
		{
			sprintf(LCD_ShowStr, "      Pause");
			LCD_DisplayStringLine(Line8,(unsigned char *)LCD_ShowStr);
		}
		if(Time_Ctrl.Secs == 0 && Time_Ctrl.mins == 0 && Time_Ctrl.hours == 0)
		{
			Mode = Standby;
			HAL_TIM_Base_Stop_IT(&htim6);
		}
		
	}
}


//CallBack
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	Time_Ctrl.Secs--;
	
}
















/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
