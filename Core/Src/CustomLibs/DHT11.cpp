#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "CustomLibs/DHT11.h"



void set_pin_output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void set_pin_input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void delayUs(int32_t us, TIM_HandleTypeDef* htim){
	/*DELAY EN MICROSEGUNDOS*/
	__HAL_TIM_SET_COUNTER(htim, 0);
	while(__HAL_TIM_GET_COUNTER(htim) < us);
}

void DHT11_Init(TIM_HandleTypeDef* htim){
	HAL_TIM_Base_Start(htim);
	HAL_TIM_Base_Start_IT(htim);
}

void DHT11_start(TIM_HandleTypeDef* htim){
	/*PROTOCOLO INICIO DE SENSOR*/
	set_pin_output(DHT11_PORT, DHT11_PIN);
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
	delayUs(18000, htim);
	HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
	delayUs(20, htim);
	set_pin_input(DHT11_PORT, DHT11_PIN);
}

uint8_t DHT11_check(TIM_HandleTypeDef* htim){
	uint8_t respuesta = 0;

	delayUs(40, htim);
	if(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))){
		delayUs(80, htim);
		if(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)) respuesta = 1;
		else respuesta = -1;
	}

	while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));

	return respuesta;
}

uint8_t DHT11_Read_Byte(TIM_HandleTypeDef* htim){
	uint8_t byte, j;

	for(j=0; j<8; j++){
		while(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)));
		delayUs(40, htim);
		if(!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))){
			byte &= ~(1<<(7-j)); //escribe 0
		}
		else byte |= (1<<(7-j)); //escribe 1
		while(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));
	}
	return byte;
}

uint8_t DHT11_Read_Sensor(TIM_HandleTypeDef* htim){
	uint8_t byte1Rh = 0, byte2Rh = 0, byte1Temp = 0, byte2Temp = 0, sum = 0, presence=0;
	delayUs(50000, htim);
	DHT11_start(htim);
	presence = DHT11_check(htim);
	byte1Rh = DHT11_Read_Byte(htim);
	byte2Rh = DHT11_Read_Byte(htim);
	byte1Temp = DHT11_Read_Byte(htim);
	byte2Temp = DHT11_Read_Byte(htim);
	sum = DHT11_Read_Byte(htim);
	return byte1Rh;
}
