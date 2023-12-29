#include "acc.h"
#include "stm32f4xx_hal.h"

void MPU6050_Init(I2C_HandleTypeDef* hi2c)
{
    uint8_t check, data;
    //Comprueba funcionamiento
    HAL_I2C_Mem_Read(hi2c,MPU6050_ADDR,WHO_AM_I_REG,1,&check,1,100);
    //Configura registros
    if(check == 104)
    {
    	//Configuracion acelerometro
        data = 0x18;	//0x00 rango +-2g; 0x18 rango +-16g
        HAL_I2C_Mem_Write(hi2c,MPU6050_ADDR,ACCEL_CONFIG_REG,1,&data,1,50);
        HAL_Delay(50);

        //data = 0x00;
        //HAL_I2C_Mem_Write(&hi2c2,MPU6050_ADDR,GYRO_CONFIG_REG,1,&data,1,50);
        //HAL_Delay(50);

        //Configuracion gestion de energia
        data = 0;
        HAL_I2C_Mem_Write(hi2c,MPU6050_ADDR,PWR_MGMT_1_REG,1,&data,1,50);
        HAL_Delay(50);
    }
}

void MPU6050_Read_Accel(I2C_HandleTypeDef* hi2c, float* accel)
{
    uint8_t recData[6];
    int16_t Accel_X_RAW,Accel_Y_RAW,Accel_Z_RAW;

    //Lee los registros del acc
    HAL_I2C_Mem_Read(hi2c,MPU6050_ADDR,ACCEL_XOUT_H_REG,I2C_MEMADD_SIZE_8BIT,recData,6,100);
    HAL_Delay(50);

    //Junta las componentes
    Accel_X_RAW = (int16_t)(recData[0] << 8 | recData[1]);
    Accel_Y_RAW = (int16_t)(recData[2] << 8 | recData[3]);
    Accel_Z_RAW = (int16_t)(recData[4] << 8 | recData[5]);

    //Conversion de datos para lectura
    //16384 para rango 2g
    //2048 para rango 16g
    accel[0] = (float)(Accel_X_RAW / 2048.0);
    accel[1] = (float)(Accel_Y_RAW / 2048.0);
    accel[2] = (float)(Accel_Z_RAW / 2048.0);
}
