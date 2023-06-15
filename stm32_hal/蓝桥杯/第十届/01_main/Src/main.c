#include "main.h"
#include "stdio.h"
#include "string.h"
#include "key_led\key_led.h"
#include "adc\adc.h"
#include "lcd\lcd.h"
#include "iic\i2c_hal.h"

#define			Upper			0x97;
#define			Normal		0x98;
#define			Lower			0x99;

void SystemClock_Config(void);
void Key_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);

//Point
uint32_t						uwTick_Key_Set_Point;
uint32_t						uwTick_Lcd_Set_Point;
uint32_t						uwTick_Led_Set_Point;

//Key
uint8_t							Key_Val;
uint8_t							Key_Down;
uint8_t							Key_Up;
uint8_t							Key_Old;

//Lcd
char								LCD_ShowStr[21] = {0};

//main
uint8_t							Mode;
float								Volt;
float								Volt_Max = 2.4;
float								Volt_Min = 1.2;
uint8_t							Upper_LED = 0;
uint8_t							Lower_LED = 1;
uint8_t							uwLED;
uint8_t							Status;
uint8_t							pSelect;

int main(void)
{

  HAL_Init();
  SystemClock_Config();
	Key_Led_Init();
	ADC2_Init();
	LCD_Init();
	I2CInit();
	
	
	//LCD
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	
	//IIC
	Volt_Max = IIC_AT24C02_Read(0)/10.0;
	Volt_Min = IIC_AT24C02_Read(1)/10.0;
	Upper_LED = IIC_AT24C02_Read(2);
	Lower_LED = IIC_AT24C02_Read(3);

  while (1)
  {
		Key_Proc();
		Lcd_Proc();
		Led_Proc();
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
		Mode++;
		Mode%=2;
		if(Mode == 0)
		{
			IIC_AT24C02_Write((uint8_t)(Volt_Max*10), 0);
			IIC_AT24C02_Write((uint8_t)(Volt_Min*10), 1);
			IIC_AT24C02_Write((uint8_t)Upper_LED, 2);
			IIC_AT24C02_Write((uint8_t)Lower_LED, 3);
		}
	}
	if(Mode == 1)
	{
		if(Key_Down == 2)
		{
			pSelect++;
			pSelect%=4;
		}
		if(Key_Down == 3)
		{
			switch(pSelect)
			{
				case 0:
					Volt_Max+=0.3f;
					if(Volt_Max > 3.3f)
					{
						Volt_Max = 3.3f;
					}
					break;
				case 1:
					Volt_Min+=0.3f;
					if(Volt_Min > 3.3f)
					{
						Volt_Min = 3.3f;
					}
					break;
				case 2:
					Upper_LED+=1;
					Upper_LED%=8;
					if(Upper_LED == Lower_LED)
					{
						Upper_LED+=1;
						Upper_LED%=8;
					}
					break;
				case 3:
					Lower_LED+=1;
					Lower_LED%=8;
					if(Upper_LED == Lower_LED)
					{
						Lower_LED+=1;
						Lower_LED%=8;
					}
					break;
			}
		}
		if(Key_Down == 4)
		{
			switch(pSelect)
			{
				case 0:
					Volt_Max-=0.3f;
					if(Volt_Max < 0)
					{
						Volt_Max = 0;
					}
					break;
				case 1:
					Volt_Min-=0.3f;
					if(Volt_Min < 0)
					{
						Volt_Min = 0;
					}
					break;
				case 2:
					Upper_LED-=1;
					Upper_LED%=8;
					if(Upper_LED == Lower_LED)
					{
						Upper_LED-=1;
						Upper_LED%=8;
					}
					break;
				case 3:
					Lower_LED-=1;
					Lower_LED%=8;
					if(Upper_LED == Lower_LED)
					{
						Lower_LED-=1;
						Lower_LED%=8;
					}
					break;
			}
		}
		
	}

}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;
	if(Mode == 0)
	{
		Volt = ADC_GetValue()*3.3/4095;
		sprintf(LCD_ShowStr, "       Main           ");
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);	
			
		sprintf(LCD_ShowStr, "   Volt:%.2fV         ", Volt);
		LCD_DisplayStringLine(Line4,(unsigned char *)LCD_ShowStr);
		
		sprintf(LCD_ShowStr, "                      ");
		LCD_DisplayStringLine(Line3,(unsigned char *)LCD_ShowStr);
		sprintf(LCD_ShowStr, "                      ");
		LCD_DisplayStringLine(Line6,(unsigned char *)LCD_ShowStr);
		if(Volt > Volt_Max)
		{
			Status = Upper;
			sprintf(LCD_ShowStr, "   Status:Upper     ");
		}
		else if(Volt < Volt_Min)
		{
			Status = Lower;
			sprintf(LCD_ShowStr, "   Status:Lower     ");
		}
		else
		{
			Status = Normal;
			sprintf(LCD_ShowStr, "   Status:Normal    ");
		}
		LCD_DisplayStringLine(Line5,(unsigned char *)LCD_ShowStr);
	}
	else if(Mode == 1)
	{
		Status = Normal;
		
		sprintf(LCD_ShowStr, "       Setting      ");
		LCD_DisplayStringLine(Line2,(unsigned char *)LCD_ShowStr);
		
		if(pSelect == 0)
			LCD_SetBackColor(Yellow);
		else
			LCD_SetBackColor(Blue);
		sprintf(LCD_ShowStr, "   Max Volt:%.1fV    ", Volt_Max);
		LCD_DisplayStringLine(Line3,(unsigned char *)LCD_ShowStr);
		if(pSelect == 1)
			LCD_SetBackColor(Yellow);
		else
			LCD_SetBackColor(Blue);
		sprintf(LCD_ShowStr, "   Min Volt:%.1fV    ", Volt_Min);
		LCD_DisplayStringLine(Line4,(unsigned char *)LCD_ShowStr);
		if(pSelect == 2)
			LCD_SetBackColor(Yellow);
		else
			LCD_SetBackColor(Blue);
		sprintf(LCD_ShowStr, "   Upper:LD%d        ", Upper_LED + 1);
		LCD_DisplayStringLine(Line5,(unsigned char *)LCD_ShowStr);
		if(pSelect == 3)
			LCD_SetBackColor(Yellow);
		else
			LCD_SetBackColor(Blue);
		sprintf(LCD_ShowStr, "   Lower:LD%d        ", Lower_LED + 1);
		LCD_DisplayStringLine(Line6,(unsigned char *)LCD_ShowStr);
		LCD_SetBackColor(Blue);
	}
	
}


void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 200)		return;
	uwTick_Led_Set_Point = uwTick;
	
	if(Status == 0x97)
	{
		uwLED ^= (0x01<<Upper_LED);
	}
	else if(Status == 0x99)
	{
		uwLED ^= (0x01<<Lower_LED);
	}
	else if(Status == 0x98)
	{
		uwLED = 0;
	}
	LED_Disp(uwLED);
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
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
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
