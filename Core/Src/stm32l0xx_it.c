/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */


#include "main.h"
#include "stm32l0xx_it.h"

extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;
extern volatile uint8_t RxBf[1];

extern UART_HandleTypeDef hlpuart1;

void NMI_Handler(void)
{
 
  while (1)
  {
  }
	
}


void HardFault_Handler(void)
{
  
  while (1)
  {
    
  }
}


void SVC_Handler(void)
{
  
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  
}

void SysTick_Handler(void)
{
  
  HAL_IncTick();
  
}


void TIM6_IRQHandler(void)
{
	
	HAL_TIM_IRQHandler(&htim6);
}


void USART1_IRQHandler(void) 
{
	HAL_UART_Receive_IT(&huart1,RxBf,1);
	HAL_UART_IRQHandler(&huart1);
}

void LPUART1_IRQHandler(void) 
{
	HAL_UART_IRQHandler(&hlpuart1);
}


void EXTI4_15_IRQHandler(void)
{
	 	HAL_GPIO_EXTI_IRQHandler(MODE_BUTTON_Pin);
		HAL_GPIO_EXTI_IRQHandler(SET0_BUTTON_Pin);
}

void EXTI2_3_IRQHandler(void)
{
		HAL_GPIO_EXTI_IRQHandler(JOYSTICK_BUTTON_Pin);
}


/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
