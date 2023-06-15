#include "main.h"
#include "uart\uart.h"
#include "led\led.h"
#include "adc\adc.h"
#include "lcd\lcd.h"
#include "iic\i2c_hal.h"
#include "string.h"
#include "stdio.h"


void SystemClock_Config(void);

void Key_Proc(void);
void Uart_Proc(void);
void Lcd_Proc(void);
void Led_Proc(void);

uint32_t									uwTick_Key_Set_Point;
uint32_t									uwTick_Lcd_Set_Point;
uint32_t									uwTick_Uart_Set_Point;
uint32_t									uwTick_Led_Set_Point;

uint32_t									Key_Time;

//Lcd
char 											Lcd_ShowStr[21];

//Key
uint8_t										Key_Val;
uint8_t										Key_Down;
uint8_t										Key_Up;
uint8_t										Key_Old;

//IIC
uint32_t									Read32;

float											Read_float;
typedef union
{
	float f;
	uint8_t	u8[4];
}f_Typedef;

f_Typedef									F;

//Main
uint8_t										Mode;
float											Weight;
uint8_t										Price_NO;
float											Unit_price[3] = {0.2, 0.3, 0.4};
float											Total_price[3];
uint8_t										pPrice;
uint8_t										Uart_Flag;


int main(void)
{

  HAL_Init();
  SystemClock_Config();
	
	MX_GPIO_Init();
	LCD_Init();
	UART_Init();
	ADC2_Init();
	I2CInit();
	
	//LCD
	LCD_Clear(Black);
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	


	AT24C02_Read(F.u8, 0, 4);
	Unit_price[0] = F.f;
	AT24C02_Read(F.u8, 4, 4);
	Unit_price[1] = F.f;
	AT24C02_Read(F.u8, 8, 4);
	Unit_price[2] = F.f;		
	
//	Read32 = ((F.u8[0]) | (F.u8[1] << 8) | (F.u8[2] << 16) | (F.u8[3] << 24));
	
	
	/*F.u8[0] == 205(CD)
	 *F.u8[1] == 204(CC)
	 *F.u8[2] == 76(4C)
	 *F.u8[3] == 61(3E)
	 */
	 
	Read32 = 0x3E4CCCCD;
	Read_float = *((float * )(&Read32));
	
  while (1)
  {
		Key_Proc();
		Uart_Proc();
		Lcd_Proc();
		Led_Proc();
  }

}

void Key_Proc(void)
{
	if(uwTick - uwTick_Key_Set_Point < 100)		return;
	uwTick_Key_Set_Point = uwTick;

	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	
	if(Key_Down == 1)
	{
		Mode++;
		Mode %= 2;
		LCD_Clear(Black);
		
		if(Mode == 0)
		{
			Uart_Flag = 2;
			AT24C02_Write((uint8_t * )&Unit_price[0], 0, 4);
			AT24C02_Write((uint8_t * )&Unit_price[1], 4, 4);
			AT24C02_Write((uint8_t * )&Unit_price[2], 8, 4);
		}
	}
	if(Mode == 1)
	{
		if(Key_Val == 2 && uwTick - Key_Time > 800 && Key_Time != 0)
		{
			Unit_price[pPrice] += 0.01f;
		}
		if(Key_Down == 2)
		{
			Key_Time = uwTick;
			Unit_price[pPrice] += 0.01f;
		}	
		if(Key_Up == 2)
		{
			Key_Time = 0;
		}
		if(Unit_price[pPrice] > 10)
			Unit_price[pPrice] = 10;		
		
		if(Key_Val == 3 && uwTick - Key_Time > 800 && Key_Time != 0)
		{
			Unit_price[pPrice] -= 0.01f;
		}
		if(Key_Down == 3)
		{
			Key_Time = uwTick;
			Unit_price[pPrice] -= 0.01f;
		}	
		if(Key_Up == 3)
		{
			Key_Time = 0;
		}
		if(Unit_price[pPrice] < 0)
			Unit_price[pPrice] = 0;
		
		if(Key_Down == 4)
		{
			pPrice++;
			pPrice %= 3;
		}	
	}
	if(Mode == 0)
	{
		if(Key_Down == 5)
		{
			Price_NO = 0;
		}	
		if(Key_Down == 6)
		{
			Price_NO = 1;
		}	
		if(Key_Down == 7)
		{
			Price_NO = 2;
		}	
		if(Key_Down == 8)
		{
			Uart_Flag = 1;
		}		
	}


}

void Lcd_Proc(void)
{
	if(uwTick - uwTick_Lcd_Set_Point < 100)		return;
	uwTick_Lcd_Set_Point = uwTick;

	Weight = Get_AO1() * 10.0 / 4095; 
	Total_price[Price_NO] = Unit_price[Price_NO] * Weight;
	if(Mode == 0)
	{
		sprintf(Lcd_ShowStr, "       Main    ");
		LCD_DisplayStringLine(Line1, (uint8_t * )Lcd_ShowStr);
		sprintf(Lcd_ShowStr, "  NO:%d  ", Price_NO + 1);
		LCD_DisplayStringLine(Line3, (uint8_t * )Lcd_ShowStr);
		sprintf(Lcd_ShowStr, "  Unit:%.2f  ", Unit_price[Price_NO]);
		LCD_DisplayStringLine(Line4, (uint8_t * )Lcd_ShowStr);
		sprintf(Lcd_ShowStr, "  Weight:%.2fkg  ", Weight);
		LCD_DisplayStringLine(Line5, (uint8_t * )Lcd_ShowStr);
		sprintf(Lcd_ShowStr, "  Total:%.2f  ", Total_price[Price_NO]);
		LCD_DisplayStringLine(Line6, (uint8_t * )Lcd_ShowStr);

		sprintf(Lcd_ShowStr, "  %f ", Read_float);
		LCD_DisplayStringLine(Line7, (uint8_t * )Lcd_ShowStr);
//		sprintf(Lcd_ShowStr, "  %f ", F.f);
//		LCD_DisplayStringLine(Line8, (uint8_t * )Lcd_ShowStr);
//		sprintf(Lcd_ShowStr, "  %d %d %d %d ", F.u8[0], F.u8[1], F.u8[2], F.u8[3]);
//		LCD_DisplayStringLine(Line9, (uint8_t * )Lcd_ShowStr);

	}
	else if(Mode == 1)
	{
		sprintf(Lcd_ShowStr, "       Para    ");
		LCD_DisplayStringLine(Line1, (uint8_t * )Lcd_ShowStr);
		
		if(pPrice == 0)
			LCD_SetBackColor(Green);
		else
			LCD_SetBackColor(Black);
		sprintf(Lcd_ShowStr, "  NO1:%.2f  ", Unit_price[0]);
		LCD_DisplayStringLine(Line3, (uint8_t * )Lcd_ShowStr);
		
		if(pPrice == 1)
			LCD_SetBackColor(Green);
		else
			LCD_SetBackColor(Black);		
		sprintf(Lcd_ShowStr, "  NO2:%.2f  ", Unit_price[1]);
		LCD_DisplayStringLine(Line4, (uint8_t * )Lcd_ShowStr);
		
		if(pPrice == 2)
			LCD_SetBackColor(Green);
		else
			LCD_SetBackColor(Black);	
		sprintf(Lcd_ShowStr, "  NO3:%.2f  ", Unit_price[2]);
		LCD_DisplayStringLine(Line5, (uint8_t * )Lcd_ShowStr);

		LCD_SetBackColor(Black);
	}

}


void Uart_Proc(void)
{
	if(uwTick - uwTick_Uart_Set_Point < 200)		return;
	uwTick_Uart_Set_Point = uwTick;

	if(Uart_Flag)
	{
		if(Uart_Flag == 1)
		{
			printf("U.W.%d:%.2f\r\n", Price_NO + 1, Unit_price[Price_NO]);
			printf("G.W:%.2f\r\n", Weight);
			printf("Total:%.2f\r\n", Total_price[Price_NO]);		
		}
		else if(Uart_Flag == 2)
		{
			printf("U.W.1:%.2f\r\n", Unit_price[0]);
			printf("U.W.2:%.2f\r\n", Unit_price[1]);
			printf("U.W.3:%.2f\r\n", Unit_price[2]);
		
		}
		
		Uart_Flag = 0;
	}
	
}	


void Led_Proc(void)
{
	if(uwTick - uwTick_Led_Set_Point < 400)		return;
	uwTick_Led_Set_Point = uwTick;

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
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
