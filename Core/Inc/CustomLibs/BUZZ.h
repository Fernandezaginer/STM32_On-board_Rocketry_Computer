#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"


#ifndef __BUZZ_H__
#define __BUZZ_H__


//TIM3 CHANNEL_2 -> OUTPUT COMPARE SET ON MATCH; ¿1 MIN?
void BUZZ_Init(TIM_HandleTypeDef* htim);

#endif

