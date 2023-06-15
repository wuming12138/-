#include "main.h"
#include "led\led.h"
#include "key\key.h"
#include "lcd\lcd.h"
#include "usart\uart.h"
#include "tim6\tim6.h"
#include "buz\buz.h"
#include "string.h"
#include "stdio.h"

void SystemClock_Config(void);


void Key_Proc(void);
void LCD_Proc1(void);
void LCD_Proc2(void);
void LCD_Proc3(void);
void USART1_Proc(void);
void Buz_Proc(void);
uint8_t Day_Judge(void);	

uint32_t				uwTick_Key_Set_Point = 0;
uint32_t				uwTick_LCD_Set_Point = 0;
uint32_t				uwTick_Buz_Set_Point = 0;


uint8_t					Key_Val;
uint8_t					Key_Down;
uint8_t					Key_Up;
uint8_t					Key_Old;


char						str1[21] = {0};
char						str2[21] = {0};
char						str3[21] = {0};
char						str4[21] = {0};
char						Alarm[50] = {0};
uint8_t					Clock[6] = {23, 3, 14, 17, 32, 21};
uint8_t					Clock_Compare[6] = {0};
uint8_t					pClock = 0;

char						TxDate[50] = {0};
unsigned char		RxDate;

uint8_t					Mode = 1;

uint8_t					pLine = 0;
uint8_t					i = 0;

uint8_t					BuzFlag;
uint8_t					BuzTimCount;

void Mycpy(uint8_t * dest, uint8_t * rsc, uint8_t Num)
{
	while(Num--)
	{
		*dest = *rsc;
		dest++;
		rsc++;
	}
}

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
	TIM6_Init();
	Buz_Init();
	
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	HAL_TIM_Base_Start_IT(&htim6);
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);

	
  while (1)
  {
		Key_Proc();
		LCD_Proc1();
		LCD_Proc2();
		LCD_Proc3();
		Buz_Proc();
  }

}



void Key_Proc(void)
{
	if((uwTick - uwTick_Key_Set_Point) < 150)	
		return;
	uwTick_Key_Set_Point = uwTick;
	
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
	
	
	//模式一
	if(Mode == 1)
	{
		if(Key_Down == 1)
		{
			HAL_TIM_Base_Stop(&htim6);
			Mode = 2;
			pClock = 3;
		}
		if(Key_Down == 2)
		{
			HAL_TIM_Base_Stop(&htim6);
			Mode = 3;
			pClock = 3;
			
			
			
			Mycpy(Clock_Compare, Clock, 6);
		}

	}
	
	//模式二
	else if(Mode == 2)
	{
		if(Key_Down == 1)
		{
			Mode = 1;
			
			HAL_TIM_Base_Start(&htim6);

		}
		if(Key_Down == 2)
		{
			pClock++;
			pClock %= 6;
		}
		if(Key_Val == 3)
		{
			Clock[pClock]++;
			if(Clock[5] >= 60)
			{
				Clock[5]--;
			}
			else if(Clock[4] >= 60)
			{
				Clock[4]--;
			}
			else if(Clock[3] >= 24)
			{
				Clock[3]--;
			}
			else if(Clock[2] > Day_Judge())
			{
				Clock[2]--;
			}
			else if(Clock[1] > 12)
			{
				Clock[1]--;
			}
		}
		if(Key_Val == 4)
		{
			Clock[pClock]--;
			if(Clock[pClock] > 200)
			{
				Clock[pClock] = 0;
			}
		}
	}
	
	//模式三
	else if(Mode == 3)
	{
		if(Key_Down == 1)
		{
			Mode = 1;
			HAL_TIM_Base_Start(&htim6);

		}
		if(Key_Down == 2)
		{
			pClock++;
			pClock %= 6;
		}
		if(Key_Val == 3)
		{
			Clock_Compare[pClock]++;
			if(Clock_Compare[5] >= 60)
			{
				Clock_Compare[5]--;
			}
			else if(Clock_Compare[4] >= 60)
			{
				Clock_Compare[4]--;
			}
			else if(Clock_Compare[3] >= 24)
			{
				Clock_Compare[3]--;
			}
			else if(Clock_Compare[2] > Day_Judge())
			{
				Clock_Compare[2]--;
			}
			else if(Clock_Compare[1] > 12)
			{
				Clock_Compare[1]--;
			}
		}
		if(Key_Val == 4)
		{
			Clock_Compare[pClock]--;
			if(Clock_Compare[pClock] > 200)
			{
				Clock_Compare[pClock] = 0;
			}
		}
	}
}




uint8_t Day_Judge(void)
{
	if( Clock[1] == 1 && Clock[1] == 3 && Clock[1] == 5 && 
			Clock[1] == 7 && Clock[1] == 8 && Clock[1] == 10 && Clock[1] == 12)
	{
		return 31;
	}
	else if(Clock[1] == 2)
	{
		return 28;
	}
	else
	{
		return 30;
	}
}
void Clock_Carry(void)
{
	if(Clock[5] >= 60){
		Clock[5] = 0;
		Clock[4]++;
		if(Clock[4] >= 60){
			Clock[4] = 0;
			Clock[3]++;
			if(Clock[3] >= 24){
				Clock[3] = 0;
				Clock[2]++;
				if(Clock[2] > Day_Judge()){
					Clock[2] = 1;
					Clock[1]++;
					if(Clock[1] > 12){
						Clock[1] = 1;
						Clock[0]++;
					}
				}
			}
		}
	}
}



void LCD_Proc1(void)
{
	if(Mode == 1)
	{
		Clock_Carry();
		if((uwTick - uwTick_LCD_Set_Point) < 100)		return;
		uwTick_LCD_Set_Point = uwTick;
		
		
		LCD_SetBackColor(Blue);
		LCD_SetTextColor(White);
		
		sprintf(str1, "Mode%d", Mode);
		LCD_DisplayStringLine(Line0, (uint8_t * )str1);
		LCD_DisplayStringLine(Line2, "   Clock:          ");
		
		LCD_ShowChar(Line3, 7, '-');
		LCD_ShowChar(Line3, 10, '-');
		LCD_ShowChar(Line4, 5, ':');
		LCD_ShowChar(Line4, 8, ':');
		LCD_ShowNum(Line3, 3, 20, Blue);
		LCD_ShowNum(Line3, 5, Clock[0], Blue);
		LCD_ShowNum(Line3, 8, Clock[1], Blue);
		LCD_ShowNum(Line3, 11, Clock[2], Blue);
		LCD_ShowNum(Line4, 3, Clock[3], Blue);
		LCD_ShowNum(Line4, 6, Clock[4], Blue);
		LCD_ShowNum(Line4, 9, Clock[5], Blue);
		
		if(Clock[3] == Clock_Compare[3] && Clock[4] == Clock_Compare[4] && Clock[5] == Clock_Compare[5])
		{
			BuzFlag = 1;
			
		}
		
	}
}


void LCD_Proc2(void)
{
	if(Mode == 2)
	{
		if((uwTick - uwTick_LCD_Set_Point) < 100)		return;
		uwTick_LCD_Set_Point = uwTick;
		
		
		LCD_SetBackColor(Blue);
		LCD_SetTextColor(White);
		
		sprintf(str1, "Mode%d", Mode);
		LCD_DisplayStringLine(Line0, (uint8_t * )str1);
		LCD_DisplayStringLine(Line2, "   Clock Set:");
		
		LCD_ShowChar(Line3, 7, '-');
		LCD_ShowChar(Line3, 10, '-');
		LCD_ShowChar(Line4, 5, ':');
		LCD_ShowChar(Line4, 8, ':');
		LCD_ShowNum(Line3, 3, 20, Blue);


		if(pClock < 3)
		{
			pLine = Line3;
			LCD_ShowNum(pLine, 5+pClock*3, Clock[pClock], Black);
		}
		else
		{
			pLine = Line4;
			LCD_ShowNum(pLine, 3+(pClock-3)*3, Clock[pClock], Black);
		}
		pClock++;
		pClock%=6;
		for(i=0;i<5;i++)
		{
			if(pClock < 3)
			{
				pLine = Line3;
				LCD_ShowNum(pLine, 5+pClock*3, Clock[pClock], Blue);
			}
			else
			{
				pLine = Line4;
				LCD_ShowNum(pLine, 3+(pClock-3)*3, Clock[pClock], Blue);
			}
			pClock++;
			pClock%=6;
		}
	}
}

void LCD_Proc3(void)
{
	if(Mode == 3)
	{
		if((uwTick - uwTick_LCD_Set_Point) < 100)		return;
		uwTick_LCD_Set_Point = uwTick;
		
		
		LCD_SetBackColor(Blue);
		LCD_SetTextColor(White);
		
		sprintf(str1, "Mode%d", Mode);
		LCD_DisplayStringLine(Line0, (uint8_t * )str1);
		LCD_DisplayStringLine(Line2, "   Alarm Set:");
		
		LCD_ShowChar(Line3, 7, '-');
		LCD_ShowChar(Line3, 10, '-');
		LCD_ShowChar(Line4, 5, ':');
		LCD_ShowChar(Line4, 8, ':');
		LCD_ShowNum(Line3, 3, 20, Blue);


		if(pClock < 3)
		{
			pLine = Line3;
			LCD_ShowNum(pLine, 5+pClock*3, Clock_Compare[pClock], Black);
		}
		else
		{
			pLine = Line4;
			LCD_ShowNum(pLine, 3+(pClock-3)*3, Clock_Compare[pClock], Black);
		}
		pClock++;
		pClock%=6;
		for(i=0;i<5;i++)
		{
			if(pClock < 3)
			{
				pLine = Line3;
				LCD_ShowNum(pLine, 5+pClock*3, Clock_Compare[pClock], Blue);
			}
			else
			{
				pLine = Line4;
				LCD_ShowNum(pLine, 3+(pClock-3)*3, Clock_Compare[pClock], Blue);
			}
			pClock++;
			pClock%=6;
		}
	}
}


void Buz_Proc(void)
{
	if(BuzFlag == 1)
	{
		if((uwTick - uwTick_Buz_Set_Point) < 1000)		return;
		uwTick_Buz_Set_Point = uwTick;
		
		if(BuzTimCount == 0)
		{
			sprintf(Alarm, "当前时间: %2d:%2d:%2d", Clock[3], Clock[4], Clock[5]);
			HAL_UART_Transmit(&huart1, (uint8_t * )Alarm, strlen(Alarm), 50);
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		LED_Disp(0xFF);
		BuzTimCount++;
		if(BuzTimCount>=3)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
			LED_Disp(0x00);
			BuzTimCount = 0;
			BuzFlag = 0;
		}
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
	LED_Disp(0xFF);
	HAL_Delay(300);
	LED_Disp(0x00);
	
	HAL_UART_Receive_IT(&huart1, &RxDate, 1);
	HAL_UART_Transmit(&huart1, &RxDate, 1, 50);
	
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	Clock[5]++;
	HAL_TIM_Base_Start_IT(&htim6);
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
