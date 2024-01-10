#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

/*
 *
 *
 * PENSADO PARA UNA FREQ DE 1Hz DEL TEMPORIZADOR PWM
 *
 *
 */


#ifndef __LED_H__
#define __LED_H__


#define PERIOD 500	//Ajustar en funcion de la configuración del timer

void LED_Init(TIM_HandleTypeDef* htim);

void LED_On(TIM_HandleTypeDef* htim);

void LED_Toggle(TIM_HandleTypeDef* htim);

#endif
