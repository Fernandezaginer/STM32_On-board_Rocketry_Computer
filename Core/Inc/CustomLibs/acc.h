#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#ifndef __ACC_H__
#define __ACC_H__

#define MPU6050_ADDR 0xD0	//Direccion dispositivo

//#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C	//Registro acelerometro
#define ACCEL_XOUT_H_REG 0x3B	//Registro componentes acc
//#define TEMP_OUT_H_REG 0x41
//#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B	//Registro gestion de energia
#define WHO_AM_I_REG 0X75	//"who am i" check


class MPU6050{
public:
	MPU6050(I2C_HandleTypeDef* hi2c);
	bool MPU6050_Init();

	//Tiene que recibir un vector de 3 floats (X, Y, Z)
	void MPU6050_Read_Accel(float* accel);

private:
	I2C_HandleTypeDef* hi2c;
};


#endif



