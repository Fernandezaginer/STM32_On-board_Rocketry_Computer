

#ifndef _PRESION_H_
#define _PRESION_H_

#include "stm32f4xx.h"

#define BMP280_ADDRESS 0x76
#define BMP280_CHIPID 0x58

#define seaLevelhPa 1013.25


// Registers
enum {
    BMP280_REGISTER_DIG_T1 = 0x88,
    BMP280_REGISTER_DIG_T2 = 0x8A,
    BMP280_REGISTER_DIG_T3 = 0x8C,
    BMP280_REGISTER_DIG_P1 = 0x8E,
    BMP280_REGISTER_DIG_P2 = 0x90,
    BMP280_REGISTER_DIG_P3 = 0x92,
    BMP280_REGISTER_DIG_P4 = 0x94,
    BMP280_REGISTER_DIG_P5 = 0x96,
    BMP280_REGISTER_DIG_P6 = 0x98,
    BMP280_REGISTER_DIG_P7 = 0x9A,
    BMP280_REGISTER_DIG_P8 = 0x9C,
    BMP280_REGISTER_DIG_P9 = 0x9E,
    BMP280_REGISTER_CHIPID = 0xD0,
    BMP280_REGISTER_VERSION = 0xD1,
    BMP280_REGISTER_SOFTRESET = 0xE0,
    BMP280_REGISTER_CAL26 = 0xE1, /**< R calibration = 0xE1-0xF0 */
    BMP280_REGISTER_STATUS = 0xF3,
    BMP280_REGISTER_CONTROL = 0xF4,
    BMP280_REGISTER_CONFIG = 0xF5,
    BMP280_REGISTER_PRESSUREDATA = 0xF7,
    BMP280_REGISTER_TEMPDATA = 0xFA,
};



// Struct to hold calibration data.
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} bmp280_calib_data;



class Bmp280{
public:
	Bmp280(I2C_HandleTypeDef *hi2c_);
	bool init();
	float getTemperature();
	float getPressure();
	float getAltitude();
private:
	I2C_HandleTypeDef *hi2c;
	bool init_ok = 0;
	bmp280_calib_data calib_data;
    int32_t t_fine;
	uint16_t read16_LE(uint8_t reg);
	int16_t readS16_LE(uint8_t reg);
	uint16_t read16(uint8_t reg);
	uint32_t read24(uint8_t reg);
};

#endif

