#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "CustomLibs/DHT11.h"


DHT11::DHT11(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* htim){
	this->GPIOx = GPIOx;
	this->GPIO_Pin = GPIO_Pin;
	this->htim = htim;
}

void DHT11::set_pin_output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11::set_pin_input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11::delayUs(int32_t us){
	/*DELAY EN MICROSEGUNDOS*/
	__HAL_TIM_SET_COUNTER(htim, 0);
	while(__HAL_TIM_GET_COUNTER(htim) < us);
}

void DHT11::DHT11_Init(){
	HAL_TIM_Base_Start(htim);
	HAL_TIM_Base_Start_IT(htim);
}

void DHT11::DHT11_start(){
	/*PROTOCOLO INICIO DE SENSOR*/
	set_pin_output(this->GPIOx, this->GPIO_Pin);
	HAL_GPIO_WritePin(this->GPIOx, this->GPIO_Pin, GPIO_PIN_RESET);
	delayUs(18000);
	HAL_GPIO_WritePin(this->GPIOx, this->GPIO_Pin, GPIO_PIN_SET);
	delayUs(40);
	set_pin_input(this->GPIOx, this->GPIO_Pin);
}

uint8_t DHT11::DHT11_check(){
	uint8_t respuesta = 0;

	delayUs(40);
	if(!(HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin))){
		this->delayUs(80);
		if(HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin)) respuesta = 1;
		else respuesta = -1;
	}

	while(HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin));

	return respuesta;
}

uint8_t DHT11::DHT11_Read_Byte() {
	uint8_t byte, j;

	for (j = 0; j < 8; j++) {
		while (HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 0) {
			this->delayUs(30);
			if(HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 1) {
					byte |= (1<<(7-j)); //escribe 1
			}
			while (HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 1){}
		}
	}

	return byte;
}

uint8_t DHT11::DHT11_Read_Sensor(){
	uint8_t byte1Rh = 0, byte2Rh = 0, byte1Temp = 0, byte2Temp = 0, sum = 0, presence=0;
	this->delayUs(50000);
	this->DHT11_start();
	presence = this->DHT11_check();
	byte1Rh = this->DHT11_Read_Byte();
	byte2Rh = this->DHT11_Read_Byte();
	byte1Temp = this->DHT11_Read_Byte();
	byte2Temp = this->DHT11_Read_Byte();
	sum = this->DHT11_Read_Byte();
	return byte1Rh;
}





int16_t DHT11::readTemperature(){
  HAL_Delay(150);
  uint8_t data[5] = {0, 0, 0, 0, 0};

  DHT11_start();
  unsigned long timeout_start = HAL_GetTick();
  while (HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 1){
    if (HAL_GetTick() - timeout_start > 300){
      return -1; //DHT11::ERROR_TIMEOUT;
    }
  }

  if (HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 0){
    delayUs(80);
    if (HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin) == 1){
      delayUs(80);
      for (int i = 0; i < 5; i++){
        data[i] = this->DHT11_Read_Byte();
        if (data[i] == 253){
          return -1; //DHT11::ERROR_TIMEOUT;
        }
      }
      if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)){
        return data[2];
      }
    }
  }
  return -1; //DHT11::ERROR_CHECKSUM;
}
