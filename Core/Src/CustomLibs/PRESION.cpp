

#include "stm32f4xx.h"
#include "CustomLibs/PRESION.h"
#include "math.h"
#include "CustomLibs/0-Utilities.h"


Bmp280::Bmp280(I2C_HandleTypeDef *hi2c_){
	this->hi2c = hi2c_;
	calib_data = {};
}


bool Bmp280::init(){

	// Verify I2C address
	if(I2C_Is_Attached(hi2c, BMP280_ADDRESS) == 0){
		init_ok = 0;
		return 0;
	}

	// Verify device
	if(I2C_Read_SReg(hi2c, BMP280_ADDRESS, BMP280_REGISTER_CHIPID) != 0x58U){
		init_ok = 0;
		return 0;
	}

	// Configurate
	I2C_Write_Reg(this->hi2c, BMP280_ADDRESS, BMP280_REGISTER_CONTROL, 0B00011100);
	I2C_Write_Reg(this->hi2c, BMP280_ADDRESS, BMP280_REGISTER_CONFIG, 0B01000111);

	// Read data factory calibration
    calib_data.dig_T1 = read16_LE(BMP280_REGISTER_DIG_T1);
    calib_data.dig_T2 = readS16_LE(BMP280_REGISTER_DIG_T2);
    calib_data.dig_T3 = readS16_LE(BMP280_REGISTER_DIG_T3);

    calib_data.dig_P1 = read16_LE(BMP280_REGISTER_DIG_P1);
    calib_data.dig_P2 = readS16_LE(BMP280_REGISTER_DIG_P2);
    calib_data.dig_P3 = readS16_LE(BMP280_REGISTER_DIG_P3);
    calib_data.dig_P4 = readS16_LE(BMP280_REGISTER_DIG_P4);
    calib_data.dig_P5 = readS16_LE(BMP280_REGISTER_DIG_P5);
    calib_data.dig_P6 = readS16_LE(BMP280_REGISTER_DIG_P6);
    calib_data.dig_P7 = readS16_LE(BMP280_REGISTER_DIG_P7);
    calib_data.dig_P8 = readS16_LE(BMP280_REGISTER_DIG_P8);
    calib_data.dig_P9 = readS16_LE(BMP280_REGISTER_DIG_P9);

    // Sampling time
	I2C_Write_Reg(this->hi2c, BMP280_ADDRESS, BMP280_REGISTER_CONFIG, 0B00011100);

    // Normal mode
	I2C_Write_Reg(this->hi2c, BMP280_ADDRESS, BMP280_REGISTER_CONTROL, 0B01000111);

	init_ok = 1;
	return 1;
}



float Bmp280::getTemperature() {

    // Not initialized
    if(init_ok == 0){
    	return NAN;
    }

    int32_t adc_T = read24(BMP280_REGISTER_TEMPDATA);

    int32_t var1;
    int32_t var2;
    adc_T >>= 4;

	var1 = ((((adc_T >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * ((int32_t)calib_data.dig_T2)) >> 11;

	var2 = (((((adc_T >> 4) - ((int32_t)calib_data.dig_T1)) * ((adc_T >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) * ((int32_t)calib_data.dig_T3)) >> 14;


    t_fine = var1 + var2;

    float T = (t_fine * 5 + 128) >> 8;
    return T / 100;
}


float Bmp280::getPressure() {
    int64_t var1, var2, p;

    // Not initialized
    if(init_ok == 0){
    	return NAN;
    }

    // Must be done first to get the t_fine variable set up
    getTemperature();

    int32_t adc_P = read24(BMP280_REGISTER_PRESSUREDATA);
    adc_P >>= 4;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_data.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_data.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3) >> 8) + ((var1 * (int64_t)calib_data.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    return (float)p / 256;
}


float Bmp280::getAltitude(){
    float altitude;
    float pressure = this->getPressure();
    pressure /= 100.0;
    altitude = 44330 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));
    return altitude;
}



uint16_t Bmp280::read16_LE(uint8_t reg) {
    uint16_t temp = read16(reg);
    return (temp >> 8) | (temp << 8);
}

int16_t Bmp280::readS16_LE(uint8_t reg) {
    return (int16_t)read16_LE(reg);
}

uint16_t Bmp280::read16(uint8_t reg) {
    uint8_t buffer[2];
	I2C_Read_Reg(this->hi2c, BMP280_ADDRESS, reg, buffer, 2);
	return uint16_t(buffer[0]) << 8 | uint16_t(buffer[1]);
}



uint32_t Bmp280::read24(uint8_t reg) {
    uint8_t buffer[3];
	buffer[0] = uint8_t(reg);
	I2C_Read_Reg(this->hi2c, BMP280_ADDRESS, reg, buffer, 3);
    return uint32_t(buffer[0]) << 16 | uint32_t(buffer[1]) << 8 |
        uint32_t(buffer[2]);
}
