


#include "stdio.h"
#include "stm32f4xx.h"


void I2C_Scan(I2C_HandleTypeDef *hi2c);

int I2C_Is_Attached(I2C_HandleTypeDef *hi2c, uint8_t add);

void I2C_Write_Reg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add, uint8_t reg_add, uint8_t val);
void I2C_Write_MReg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add, uint8_t reg_add, uint8_t *val, uint8_t nbytes);
void I2C_Read_Reg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add, uint8_t reg_add, uint8_t *buffer, uint16_t size);
uint8_t I2C_Read_SReg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add, uint8_t reg_add);


//void SPI_Write_Reg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t val);
//void SPI_Write_MReg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t *val, uint8_t nbytes);
//void SPI_Read_Reg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t *buffer, uint16_t size);
//uint8_t SPI_Read_SReg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add);


