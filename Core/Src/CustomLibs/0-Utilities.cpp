

#include "stdio.h"
#include "stm32f4xx.h"
#include "CustomLibs/0-Utilities.h"
#include <malloc.h>





//----------------------------------------------------------
//                    GPIO UTILITIES
//----------------------------------------------------------


GPIO_PIN::GPIO_PIN(GPIO_TypeDef* pin_port, uint16_t pin_num){
	this->pin_port = pin_port;
	this->pin_num = pin_num;
}
void GPIO_PIN::setHigh(){
	HAL_GPIO_WritePin(pin_port, pin_num, GPIO_PIN_SET);
}

void GPIO_PIN::setLow(){
	HAL_GPIO_WritePin(pin_port, pin_num, GPIO_PIN_RESET);
}

bool GPIO_PIN::read(){
	return HAL_GPIO_ReadPin(pin_port, pin_num);
}






//----------------------------------------------------------
//                    I2C  UTILITIES
//----------------------------------------------------------


void I2C_Scan(I2C_HandleTypeDef *hi2c) {
	printf("Test");
	printDebug("Scan I2C: \r\n");
	uint8_t ret;
	for (int i = 1; i < 128; i++) {
		ret = HAL_I2C_IsDeviceReady(hi2c, (uint16_t) (i << 1), 3, 5);
		if (ret != HAL_OK) { // No ACK

		} else if (ret == HAL_OK) {
			printDebug("Found: (0x");
 	    	if(i < 16){
 	    		printDebug("0");
 	    	}
			printDebugIntHex(i);
	    	printDebug(")HEX (7bit-address)\n");

		}
	}
	printDebug("Done! \r\n\r\n");

}

int I2C_Is_Attached(I2C_HandleTypeDef *hi2c, uint8_t add) {
	int ret = HAL_I2C_IsDeviceReady(hi2c, (uint16_t) (add << 1), 3, 5);
	if (ret != HAL_OK) { // No ACK
		return 0;
	} else { // ACK ok
		return 1;
	}
}

void I2C_Write_Reg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add,
	uint8_t reg_add, uint8_t val) {
	uint8_t data[2] = { };
	data[0] = reg_add;
	data[1] = val;
	HAL_I2C_Master_Transmit(hi2c, i2c_7bit_add << 1, data, 2, 10);
}

void I2C_Write_MReg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add,
	uint8_t reg_add, uint8_t *val, uint8_t nbytes) {
	uint8_t *data;
	data = (uint8_t*) malloc((nbytes + 1) * sizeof(uint8_t));
	data[0] = reg_add;
	for (int i = 0; i < nbytes; i++) {
		data[1 + i] = val[i];
	}
	HAL_I2C_Master_Transmit(hi2c, i2c_7bit_add << 1, data, nbytes + 1, 10);
	free(data);
}

void I2C_Read_Reg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add,
	uint8_t reg_add, uint8_t *buffer, uint16_t size) {
	HAL_I2C_Master_Transmit(hi2c, i2c_7bit_add << 1, &reg_add, 1, 10);
	HAL_I2C_Master_Receive(hi2c, i2c_7bit_add << 1, buffer, size, 10);
}

uint8_t I2C_Read_SReg(I2C_HandleTypeDef *hi2c, uint8_t i2c_7bit_add,
	uint8_t reg_add) {
	uint8_t buffer;
	HAL_I2C_Master_Transmit(hi2c, i2c_7bit_add << 1, &reg_add, 1, 10);
	HAL_I2C_Master_Receive(hi2c, i2c_7bit_add << 1, &buffer, 1, 10);
	return buffer;
}






//----------------------------------------------------------
//                    SPI UTILITIES
//----------------------------------------------------------

//void SPI_Write_Reg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t val) {
//	uint8_t data[2] = {};
//	cs->setLow();
//	data[0] = reg_add & 0B01111111;
//	data[1] = val;
//	HAL_SPI_Transmit(hspi, data, 2, 100);
//	cs->setHigh();
//}
//void SPI_Write_MReg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t *val, uint8_t nbytes) {
//	uint8_t *data;
//	cs->setLow();
//	data = (uint8_t*) malloc((nbytes + 1) * sizeof(uint8_t));
//	data[0] = reg_add & 0b01111111;
//	for (int i = 0; i < nbytes; i++) {
//		data[1 + i] = val[i];
//	}
//	HAL_SPI_Transmit(hspi, data, nbytes + 1, 100);
//	cs->setHigh();
//	free(data);
//}
//void SPI_Read_Reg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add, uint8_t *buffer, uint16_t size) {
//	cs->setLow();
//	uint8_t reg = reg_add | 0B10000000;
//	HAL_SPI_Transmit(hspi, &reg, 1, 100);
//	HAL_SPI_Receive(hspi, buffer, size, 100);
//	cs->setHigh();
//}
//uint8_t SPI_Read_SReg(SPI_HandleTypeDef *hspi, GPIO_PIN *cs, uint8_t reg_add) {
//	cs->setLow();
//	uint8_t buffer;
//	uint8_t reg = reg_add | 0B10000000;
//	HAL_SPI_Transmit(hspi, &reg, 1, 100);
//	HAL_SPI_Receive(hspi, &buffer, 1, 100);
//	cs->setHigh();
//	return buffer;
//}







//----------------------------------------------------------
//                 DEBUG  SWD  TRACE
//----------------------------------------------------------


int _write(int file, char *ptr, int len) {
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		ITM_SendChar(*ptr++);
	}
	return len;
}
void printDebug(char *__restrict info, ...) {
	int i = 0;
	while (info[i] != '\0') {
		i++;
	}
	_write(0, info, i);

}
void printDebugInt(int aux) {
	char str[10];
	sprintf(str, "%d", aux);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	_write(0, str, i);
}
void printDebugIntHex(int aux) {
	char str[10];
	sprintf(str, "%X", aux);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	_write(0, str, i);
}
void printDebugFloat(float aux) {
	char str[30];
	sprintf(str, "%.5e", aux);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	_write(0, str, i);
}

