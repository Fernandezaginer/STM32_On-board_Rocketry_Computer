


#include "CustomLibs/0-utilities.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_tim.h"


#ifndef _SERVO_H_
#define _SERVO_H_



#define FREQ_PRESCALER 1000000
#define FREQ_SERVO 50
#define ARR_REG 36000


typedef struct{
	TIM_HandleTypeDef* tim_handle;
	uint16_t tim_chanel;
}SERVO_PIN;


class Servo{
public:
	void setup(SERVO_PIN);
	bool attach();
	void write(int);
	void write(float);
	uint8_t read();
	void detach();
private:
	TIM_HandleTypeDef* tim_handle;
	uint8_t tim_chanel;
	uint8_t angle;
};


#endif



