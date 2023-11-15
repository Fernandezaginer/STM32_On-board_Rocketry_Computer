
#include "stm32f4xx.h"
#include "0-Utilities.h"


#ifndef _SERVO_H_
#define _SERVO_H_

#define FREQ_PRESCALER 1000000
#define FREQ_SERVO 50
#define ARR_REG 36000


typedef struct{
	TIM_HandleTypeDef* tim_handle;
	uint16_t tim_chanel;
}SERVO_PIN;


class EEPROM{
public:
	void Setup();
	void Check();
	void Clear();
	void Write();
	void Read();
private:

};


#endif

















