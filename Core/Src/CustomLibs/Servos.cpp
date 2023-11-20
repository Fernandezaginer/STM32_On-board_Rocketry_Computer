// "Servo.cpp"
// Library for servomotors
// 4ALLBOT - Prueba Test PCB Servomotores NOV 2022

#include "CustomLibs/Servo.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_tim.h"


//----------------------------------------------------------
//                     class Servo
//----------------------------------------------------------



void Servo::setup(SERVO_PIN servo_pin){
	this->tim_handle = servo_pin.tim_handle;
	this->tim_chanel = servo_pin.tim_chanel;
	//__HAL_TIM_SET_AUTORELOAD(this->tim_handle, ARR_REG);
}

bool Servo::attach(){
	HAL_TIM_PWM_Start(tim_handle, tim_chanel);
	return true;
}



void Servo::write(int ang){
	float duty = 0.025 + (((float)ang)/180.0)*0.1;
	//float duty = 0.05 + (((float)ang)/180.0)*0.05;
	duty = 1 - duty;
	__HAL_TIM_SET_COMPARE(this->tim_handle, this->tim_chanel, (uint16_t)(ARR_REG * duty));
}


void Servo::write(float ang){
	float duty = 0.025 + ((angle)/180.0)*0.1;
	duty = 1 - duty;
	__HAL_TIM_SET_COMPARE(this->tim_handle, this->tim_chanel, (uint16_t)(ARR_REG * duty));
}


uint8_t Servo::read(){
	return angle;
}


void Servo::detach(){
	HAL_TIM_PWM_Stop(tim_handle, tim_chanel);
}



