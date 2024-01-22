

#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/EEPROM.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include <malloc.h>



EEPROM::EEPROM(I2C_HandleTypeDef* hi2c){
	this->hi2c = hi2c;
}



bool EEPROM::Setup(){
	return I2C_Is_Attached(hi2c, EEPROM_I2C_ADDRESS);
}



void EEPROM::ConfigUnitSave(uint8_t n_float, uint8_t n_uint32_t, uint8_t n_int32_t, uint8_t n_uint16_t, uint8_t n_int16_t, uint8_t n_uint8_t, uint8_t n_int8_t){
	this->n_float = n_float;
	this->n_uint32_t = n_uint32_t;
	this->n_int32_t = n_int32_t;
	this->n_uint16_t = n_uint16_t;
	this->n_int16_t = n_int16_t;
	this->n_uint8_t = n_uint8_t;
	this->n_int8_t = n_int8_t;

	n_bytes = n_float*4 + n_uint32_t*4 + n_int32_t*4 + n_uint16_t*2 + n_int16_t*2 + n_int8_t + n_uint8_t;
}

void EEPROM::ConfigPointerSave(uint8_t* pointer){
	this->pointer = pointer;
}


void EEPROM::ConfigPointerTime(uint32_t* flight_time){
	this->flight_time = flight_time;
}


void EEPROM::ConfigSpace(float despegue, float aterrizaje, uint32_t t_vuelo, uint32_t t_caida){
	this->t_despegue = t_vuelo;
	this->t_aterrizaje = t_caida;

	// Calcular el número de unidades de escritura
	n_despegue = (uint32_t) ((EEPROM_N_BYTES/n_bytes) * despegue);
	n_aterrizaje = (uint32_t) ((EEPROM_N_BYTES/n_bytes) * aterrizaje);
	intervalo_despegue = t_despegue / n_despegue;
	//intervalo_aterrizaje =
	n_ud_maximas = (uint32_t)(EEPROM_N_BYTES/n_bytes);
}


void EEPROM::writeEEPROM_Page(uint16_t address, uint8_t *val, uint8_t tam) {
	uint8_t* pdata;
	pdata = (uint8_t *)malloc((tam+2)*sizeof(uint8_t));
	pdata[0] = (uint8_t)(address >> 8);
	pdata[1] = (uint8_t)(address & 0xFF);
	for (int i = 0; i < tam; i++){
		pdata[2+i] = val[i];
	}
	HAL_I2C_Master_Transmit(hi2c, (EEPROM_I2C_ADDRESS << 1), pdata, 2+tam, 10);
	free(pdata);
}



// Read EEPROM address
uint8_t EEPROM::readEEPROM(uint32_t address) {
  uint8_t rcvData = 0xFF;

  uint8_t buf[2] = {};
  buf[0] = (uint8_t)(address >> 8);       // MSB
  buf[1] = (uint8_t)(address & 0x00FF);   // LSB

  HAL_I2C_Master_Transmit(hi2c, (EEPROM_I2C_ADDRESS << 1), buf, 2, 10);
  HAL_I2C_Master_Receive(hi2c, (EEPROM_I2C_ADDRESS << 1)+1, &rcvData, 1, 10);
  return rcvData;
}


void EEPROM::readEEPROMpage(uint8_t* buffer, uint32_t address_start, uint8_t size){
	for(int i = 0; i < size; i++){
		buffer[i] = readEEPROM(address_start + i);
	}
}






// -----------  API -----------------



void EEPROM::loop(bool despegue, bool caida){
	if (despegue){

		// Realizar escritura si ha pasado el tiempo correspondiente
		// (La eeprom tiene 32 Kb, sirve para poner un límite)
		if(intervalo_despegue*n_ud_escritas < (*(this->flight_time))){
			// Escribir los valores

			if(n_ud_escritas+1<n_ud_maximas){
				writeEEPROM_Page((uint8_t)(n_ud_escritas*n_bytes), this->pointer, n_bytes);
				n_ud_escritas++;
			}
		}


	}
	else if(caida){

		// Realizar escritura si ha pasado el tiempo correspondiente
		// (La eeprom tiene 32 Kb, sirve para poner un límite)
		if(intervalo_despegue*n_ud_escritas < * flight_time){
			// Escribir los valores

			if(n_ud_escritas+1<n_ud_maximas){
				writeEEPROM_Page((uint8_t)(n_ud_escritas*n_bytes), this->pointer, n_bytes);
				n_ud_escritas++;
			}
		}

	}
}













// -----------  API -----------------

void EEPROM::PrintDebug(){


	printDebug("Lectura de la memoria EEPROM por SWD:\n");
	printDebug("\n");
	printDebug("\n");
	printDebug("Contenido de la memoria:\n");
	printDebug("\nNº Floats:");
	printDebugInt((int)n_float);
	printDebug("\nNº Uint32:");
	printDebugInt((int)n_uint32_t);
	printDebug("\nNº Int32:");
	printDebugInt((int)n_int32_t);
	printDebug("\nNº Uint16:");
	printDebugInt((int)n_uint16_t);
	printDebug("\nNº Int16:");
	printDebugInt((int)n_int16_t);
	printDebug("\nNº Uint8:");
	printDebugInt((int)n_uint8_t);
	printDebug("\nNº Int8:");
	printDebugInt((int)n_int8_t);
	printDebug("\n\n\n");


	for(int i = 0; i < n_ud_maximas; i++){
		uint8_t* data_read;
		uint8_t dir_data;
		dir_data = 0;
		data_read = (uint8_t*)malloc((n_bytes) * sizeof(uint8_t));
		this->readEEPROMpage(data_read, i*n_bytes, n_bytes);

//		for(int j = 0; j < n_bytes; j++){
//			printDebugInt((int)data_read[j]);
//			printDebug("\n");
//		}


		// Print floats
		if (n_float > 0){
			for(int i = 0; i < n_float; i++){
				float val;
				this->_4byte_to_float(&(data_read[dir_data]), &val);
				printDebugFloat(val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print U32
		if (n_uint32_t > 0){
			for(int i = 0; i < n_uint32_t; i++){
				float val;
				void _4byte_to_U32(uint8_t* aux, uint32_t *out);
				this->_4byte_to_float(&(data_read[dir_data]), &val);
				printDebugFloat(val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print I32
		if (n_int32_t > 0){
			for(int i = 0; i < n_int32_t; i++){
				int32_t val;
				this->_4byte_to_I32(&(data_read[dir_data]), &val);
				printDebugFloat(val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print U16
		if (n_uint16_t > 0){
			for(int i = 0; i < n_uint16_t; i++){
				uint16_t val;
				this->_2byte_to_U16(&(data_read[dir_data]), &val);
				printDebugFloat(val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print I16
		if (n_int16_t > 0){
			for(int i = 0; i < n_int16_t; i++){
				int16_t val;
				this->_2byte_to_I16(&(data_read[dir_data]), &val);
				printDebugFloat(val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print U8
		if (n_uint8_t > 0){
			for(int i = 0; i < n_uint8_t; i++){
				uint8_t val;
				this->_byte_to_U8(&(data_read[dir_data]), &val);
				printDebugInt((int)val);
				printDebug("\t");
				dir_data=+4;
			}
		}


		// Print I8
		if (n_int8_t > 0){
			for(int i = 0; i < n_int8_t; i++){
				int8_t val;
				this->_byte_to_I8(&(data_read[dir_data]), &val);
				printDebugInt((int)val);
				printDebug("\t");
				dir_data=+4;
			}
		}

		free(data_read);


		// Print \n
		printDebug("\n");


	}

}



void EEPROM::Sd_Save(){

}






// ---------- CONVERT FNC -----------


// Conversión de los bytes a float
void EEPROM::_4byte_to_float(uint8_t* aux, float *out) {
  uint32_t mem_aux = 0;
  mem_aux |= aux[0];
  mem_aux |= (uint32_t)(aux[1]) << 8;
  mem_aux |= (uint32_t)(aux[2]) << 16;
  mem_aux |= (uint32_t)(aux[3]) << 24;
  *(out) = *((float*)&mem_aux);
}


void EEPROM::_4byte_to_U32(uint8_t* aux, uint32_t *out){
  uint32_t mem_aux = 0;
  mem_aux |= aux[0];
  mem_aux |= (uint32_t)(aux[1]) << 8;
  mem_aux |= (uint32_t)(aux[2]) << 16;
  mem_aux |= (uint32_t)(aux[3]) << 24;
  *(out) = mem_aux;
}


void EEPROM::_4byte_to_I32(uint8_t* aux, int32_t *out){
	int32_t mem_aux = 0;
	mem_aux |= aux[0];
	mem_aux |= (int32_t)(aux[1]) << 8;
	mem_aux |= (int32_t)(aux[2]) << 16;
	mem_aux |= (int32_t)(aux[3]) << 24;
	*(out) = mem_aux;
}


void EEPROM::_2byte_to_U16(uint8_t* aux, uint16_t *out){
	uint16_t mem_aux = 0;
	mem_aux |= aux[0];
	mem_aux |= (uint16_t)(aux[1]) << 8;
	*(out) = mem_aux;
}


void EEPROM::_2byte_to_I16(uint8_t* aux, int16_t *out){
	int16_t mem_aux = 0;
	mem_aux |= aux[0];
	mem_aux |= (int16_t)(aux[1]) << 8;
	*(out) = mem_aux;
}


void EEPROM::_byte_to_U8(uint8_t* aux, uint8_t *out){
	*(out) = *(aux);
}


void EEPROM::_byte_to_I8(uint8_t* aux, int8_t *out){
	*(out) = *(aux);
}







void EEPROM::float_to_4byte(float* var, uint8_t* aux) {
  uint8_t* p = (uint8_t*)var;
  for (char i = 3; i >= 0; i--) {
    *(aux + i) = *p;
    p++;
  }
}




