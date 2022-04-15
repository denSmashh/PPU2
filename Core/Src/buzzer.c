#include "buzzer.h"

extern TIM_HandleTypeDef htim2;


void buzzer(int duration) 
{	
	 HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	 HAL_Delay(duration); 
	 HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}
