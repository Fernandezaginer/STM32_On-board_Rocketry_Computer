#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "BUZZ.h"

void BUZZ_Init(TIM_HandleTypeDef* htim){
  	HAL_TIM_OC_Start(htim, TIM_CHANNEL_2);

  	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 60000);
  }
