/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"


UART_HandleTypeDef huart1;
UART_HandleTypeDef hlpuart1;
I2C_HandleTypeDef hi2c2;
ADC_HandleTypeDef hadc1;
ADC_ChannelConfTypeDef RegularChannel;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
IWDG_HandleTypeDef hiwdg;


void SystemClock_Config(void);
static void GPIO_Init(void);
static void USB_UART_Init(void);
static void LPUART_RS485(void);
static void ADC_Init(void);
static void TIM2_Init(void);
static void TIM6_Init(void);
static void IWDG_Init(void);


int main(void)
{
	HAL_Init();
  SystemClock_Config();
  GPIO_Init();
	USB_UART_Init();
	LPUART_RS485();
	ADC_Init();
	TIM2_Init();
	TIM6_Init();
	
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
	
	LCD_Init();
	LCD_Clear();	
	init_menu();
	
	IWDG_Init();
	
	loop();

}


void SystemClock_Config(void) // тактовая частота - 32 MHz
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_8;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
 
	
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

	
}


static void GPIO_Init(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
	
	//тактирование портов
  __HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_InitStruct;
		
	/* USART1 for USB 
		GPIO Configuration
     PA9     ------> USB_UART_TX_Pin
     PA10     ------> USB_UART_RX_Pin
  */
    GPIO_InitStruct.Pin = USB_UART_TX_Pin  |USB_UART_RX_Pin  ;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
    HAL_GPIO_Init(USB_UART_Port, &GPIO_InitStruct);
			
		/*USART2 for RS485
		GPIO Configuration
     PB10     ------> LPUART_RS485_TX_Pin 
     PB11     ------> LPUART_RS485_RX_Pin
		 PB1      ------> LPUART_RS485_DE_Pin (Hardware Flow Control)
    */
		GPIO_InitStruct.Pin = LPUART_RS485_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_LPUART1;
    HAL_GPIO_Init(LPUART_RS485_Port, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = LPUART_RS485_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_LPUART1;
    HAL_GPIO_Init(LPUART_RS485_Port, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = LPUART_RS485_DE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //GPIO_InitStruct.Alternate = GPIO_AF4_LPUART1;
    HAL_GPIO_Init(LPUART_RS485_Port, &GPIO_InitStruct);
	
		/*
		ADC1 GPIO Configuration
		PA0    ------>ADC1_IN0_vrX_Pin 
		PA1    ------>ADC1_IN1_vrY_Pin 
		*/
		GPIO_InitStruct.Pin = ADC1_IN0_vrX_Pin|ADC1_IN1_vrY_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(ADC1_Port, &GPIO_InitStruct);
		
		/*
		LCD1602 GPIO Config(RS,D4,D5,D6,D7,BL,EN) and (LCD_BL_Pin,LCD_Enable)
		*/
		GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(LCD_Data_Port, &GPIO_InitStruct);
		HAL_GPIO_WritePin(LCD_Data_Port,LCD_RS_Pin|LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin,GPIO_PIN_RESET);
		
		//LCD1602 GPIO Config(LCD_BL_Pin,LCD_Enable_Pin)
		GPIO_InitStruct.Pin = LCD_BL_Pin|LCD_Enable_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(LCD_BL_EN_Port, &GPIO_InitStruct);		
		HAL_GPIO_WritePin(LCD_BL_EN_Port,LCD_Enable_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LCD_BL_EN_Port,LCD_BL_Pin,GPIO_PIN_SET); // backlight ON
		
		// Configure user leds (LED_PC_Pin, LED_ROTATOR_Pin)
		GPIO_InitStruct.Pin = LED_PC_Pin|LED_ROTATOR_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(LEDS_Port, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = SET0_BUTTON_Pin;
		//GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		//GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(SET0_BUTTON_Port, &GPIO_InitStruct);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_SetPriority(EXTI4_15_IRQn,0,0);	
		
		GPIO_InitStruct.Pin = MODE_BUTTON_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(MODE_BUTTON_Port, &GPIO_InitStruct);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_SetPriority(EXTI4_15_IRQn,0,0);
		
		GPIO_InitStruct.Pin = JOYSTICK_BUTTON_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(JOYSTICK_BUTTON_Port, &GPIO_InitStruct);
		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
		HAL_NVIC_SetPriority(EXTI2_3_IRQn,0,0);
		
		GPIO_InitStruct.Pin = BUZZER_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
		GPIO_InitStruct.Alternate = GPIO_AF5_TIM2;
		HAL_GPIO_Init(BUZZER_Port, &GPIO_InitStruct);

}

static void USB_UART_Init(void)
{
  __HAL_RCC_USART1_CLK_ENABLE();

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
	
		HAL_NVIC_SetPriority(USART1_IRQn,0,0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void LPUART_RS485(void)
{
	__HAL_RCC_LPUART1_CLK_ENABLE();
	
	hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 38400;
	hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
	hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OverSampling = UART_OVERSAMPLING_16;
	
	if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }

	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
	HAL_NVIC_SetPriority(LPUART1_IRQn,0,0);
		
}


static void ADC_Init(void)
{
		__HAL_RCC_ADC1_CLK_ENABLE();
	
		hadc1.Instance = ADC1;  
		hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1; // 16 MHz, тактируемся от HSI
		//hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2; // 16 MHz
		hadc1.Init.Resolution = ADC_RESOLUTION_8B;
		hadc1.Init.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
		hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
		//hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
		hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
		hadc1.Init.ContinuousConvMode = DISABLE;
		hadc1.Init.DiscontinuousConvMode = DISABLE;
		hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
		hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
		hadc1.Init.DMAContinuousRequests = DISABLE;
		hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
		hadc1.Init.OversamplingMode = DISABLE;
		hadc1.Init.LowPowerAutoWait = ENABLE; //позволяет начинать новое преобразование после функции HAL_ADC_GetValue()
		hadc1.Init.LowPowerAutoPowerOff = DISABLE;
		hadc1.Init.LowPowerFrequencyMode = DISABLE;

		if( HAL_ADC_Init(&hadc1) != HAL_OK)
		{
		 Error_Handler();
		}
		
		//Configure Channel parameters
		RegularChannel.Channel = ADC_CHANNEL_0;
		RegularChannel.Rank = ADC_RANK_CHANNEL_NUMBER;
		if(HAL_ADC_ConfigChannel(&hadc1,&RegularChannel) != HAL_OK) 
		{
		 Error_Handler();
		}
		
	
		RegularChannel.Channel = ADC_CHANNEL_1;
		if(HAL_ADC_ConfigChannel(&hadc1,&RegularChannel) != HAL_OK) 
		{
		 Error_Handler();
		}
	
}

static void TIM2_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
	
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	htim2.Instance = TIM2;
  htim2.Init.Prescaler = 320 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 25 - 1;  											//frequency buzzer ~ 4 kHz 
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
	
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }

	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
	
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
	
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 13 - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
	
}

static void TIM6_Init(void)				
{
	__HAL_RCC_TIM6_CLK_ENABLE();
	
	htim6.Instance = TIM6;
  htim6.Init.Prescaler = 32000 - 1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 600 - 1;  				// в мс		
  htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
		
	HAL_NVIC_EnableIRQ(TIM6_IRQn);
	HAL_NVIC_SetPriority(TIM6_IRQn,0,0);
}

static void IWDG_Init(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 2999;  // ~ 3 c
	hiwdg.Init.Window = 2999;
	HAL_IWDG_Init(&hiwdg);
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
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
