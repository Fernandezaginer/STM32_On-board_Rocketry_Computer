

#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/EEPROM.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include <malloc.h>



bool EEPROM::Setup(){

	// Init I2C
}



void EEPROM::ConfigUnitSave(uint8_t n_float, uint8_t n_uint32_t, uint8_t n_int32_t, uint8_t n_uint16_t, uint8_t n_int16_t, uint8_t n_uint8_t, uint8_t n_int8_t){
	this->n_float = n_float;
	this->n_uint32_t = n_uint32_t;
	this->n_int32_t = n_int32_t;
	this->n_uint16_t = n_uint16_t;
	this->n_int16_t = n_int16_t;
	this->n_uint16_t = n_uint16_t;
	this->n_int16_t = n_int16_t;

	n_bytes = n_float*4 + n_uint32_t*4 + n_int32_t*4 + n_uint16_t*2 + n_int16_t*2 + n_int8_t + n_uint8_t;
}

void EEPROM::ConfigPointerSave(uint8_t* pointer){
	this->pointer = pointer;
}


void EEPROM::ConfigPointerTime(uint32_t* hal_tick){
	this->hal_tick = hal_tick;
}


void EEPROM::ConfigSpace(float despege, float aterrizaje, uint32_t t_vuelo, uint32_t t_caida){
	this->despege = despege;
	this->aterrizaje = aterrizaje;
	this->t_vuelo = t_vuelo;
	this->t_caida = t_caida;

	// Calcular el número de celdas


}



void EEPROM::loop(){

}












void EEPROM::writeEEPROM_Page(uint16_t address, uint8_t *val, uint8_t tam) {
	uint8_t* pdata;
	pdata = (uint8_t *)malloc((tam+2)*sizeof(uint8_t));
	pdata[0] = (uint8_t)(address >> 8);
	pdata[1] = (uint8_t)(address & 0xFF);
	for (int i = 0; i < tam; i++){
		pdata[2+i] = val[i];
	}
	HAL_I2C_Master_Transmit(hi2c, EEPROM_I2C_ADDRESS << 1, pdata, 2+tam, 10);
	free(pdata);
}




void EEPROM::float_to_4byte(float* var, uint8_t* aux) {
  uint8_t* p = (uint8_t*)var;
  for (char i = 3; i >= 0; i--) {
    *(aux + i) = *p;
    p++;
  }
}


// Conversión de los bytes a float
void EEPROM::_4byte_to_float(uint8_t* aux, float *out) {
  uint32_t mem_aux = 0;
  mem_aux |= aux[3];
  mem_aux |= (uint32_t)(aux[2]) << 8;
  mem_aux |= (uint32_t)(aux[1]) << 16;
  mem_aux |= (uint32_t)(aux[0]) << 24;
  *(out) = *((float*)&mem_aux);
}


// Guardar el uint16_t MSB, LSB
void EEPROM::uint16_to_2byte(uint16_t dato_in, uint8_t* dir_dato_out) {
  *(dir_dato_out) = (uint8_t)(dato_in >> 8);
  *(dir_dato_out + 1) = (uint8_t)dato_in;
}





// -----------  API -----------------

void EEPROM::PrintDebug(){

}






























