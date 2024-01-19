#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"


#ifndef __DHT11_H__
#define __DHT11_H__


#define DHT11_PORT GPIOE
#define DHT11_PIN GPIO_PIN_1


class DHT11{
public:

	DHT11(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_HandleTypeDef* htim);

	void DHT11_Init();
	void DHT11_start();

	uint8_t DHT11_check();
	uint8_t DHT11_Read_Byte();
	uint8_t DHT11_Read_Sensor();
	int16_t readTemperature();


private:
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	TIM_HandleTypeDef* htim;

	void delayUs(int32_t us);
	void set_pin_output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	void set_pin_input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
};




#endif

