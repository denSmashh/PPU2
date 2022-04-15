/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "main_loop.h"
#include "lcd.h"
#include "menu.h"
#include "stdio.h"
#include "buzzer.h"



#define USB_UART_TX_Pin      GPIO_PIN_9
#define USB_UART_RX_Pin      GPIO_PIN_10
#define USB_UART_Port        GPIOA

#define LPUART_RS485_TX_Pin  GPIO_PIN_10
#define LPUART_RS485_RX_Pin  GPIO_PIN_11
#define LPUART_RS485_DE_Pin  GPIO_PIN_1
#define LPUART_RS485_Port    GPIOB

#define ADC1_IN0_vrX_Pin     GPIO_PIN_0
#define ADC1_IN1_vrY_Pin     GPIO_PIN_1
#define ADC1_Port GPIOA

#define LCD_RS_Pin 			 		 GPIO_PIN_3
#define LCD_D4_Pin 			 		 GPIO_PIN_4
#define LCD_D5_Pin 			 		 GPIO_PIN_5
#define LCD_D6_Pin 			 		 GPIO_PIN_6
#define LCD_D7_Pin 			 		 GPIO_PIN_7
#define LCD_Data_Port 	 		 GPIOA

#define SET0_BUTTON_Pin  		 GPIO_PIN_12 // кнопки SET0 и MODE поменяны местами, так как запаяны наооборот
#define SET0_BUTTON_Port 		 GPIOB

#define MODE_BUTTON_Pin  		 GPIO_PIN_8 
#define MODE_BUTTON_Port 		 GPIOA

#define JOYSTICK_BUTTON_Pin  GPIO_PIN_2
#define JOYSTICK_BUTTON_Port GPIOA

#define LCD_BL_Pin      		 GPIO_PIN_0
#define LCD_Enable_Pin       GPIO_PIN_2
#define LCD_BL_EN_Port       GPIOB 

#define LED_ROTATOR_Pin      GPIO_PIN_13
#define LED_PC_Pin 			     GPIO_PIN_14
#define LEDS_Port  			     GPIOC
 
#define BUZZER_Pin		     	 GPIO_PIN_15
#define BUZZER_Port 		     GPIOA
 
void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
