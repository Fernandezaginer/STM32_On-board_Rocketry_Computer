#include "acc.h"
#include "stm32f4xx_hal.h"
#include "LED.h"

void LED_Init(TIM_HandleTypeDef* htim){
	HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);

	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
}

void LED_On(TIM_HandleTypeDef* htim){
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, PERIOD);
}

void LED_Toggle(TIM_HandleTypeDef* htim){
	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, PERIOD/2);
}
