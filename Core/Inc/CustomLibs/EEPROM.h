

#include "stdio.h"
#include "stm32f4xx.h"
#include "0-Utilities.h"


#ifndef _EEPROM_H_
#define _EEPROM_H_

#define EEPROM_N_BYTES 65536     // 24FC512
#define EEPROM_I2C_ADDRESS 0x50


class EEPROM{
public:

	EEPROM(I2C_HandleTypeDef* hi2c);

	// Configuración de la EEPROM I2C
	bool Setup();



	// Funcion de configuracion de la unidad de escritura (numero de datos a guardar)
	void ConfigUnitSave(uint8_t n_float, uint8_t n_uint32_t, uint8_t n_int32_t, uint8_t n_uint16_t, uint8_t n_int16_t, uint8_t n_uint8_t, uint8_t n_int8_t);

	// Funcion para definir el puntero de guardado de datos
	void ConfigPointerSave(uint8_t* pointer);

	// Funcion para configurar la lectura del tiempo
	void ConfigPointerTime(uint32_t* flight_time);

	// Funcion para definir la velocidad de escritura en cada etapa
	void ConfigSpace(float despegue, float aterrizaje, uint32_t t_vuelo, uint32_t t_caida);


	// Funcion para el guardado de datos si procede
	void loop(bool despegue, bool caida);



	// -----------  API -----------------

	void PrintDebug();

	void Sd_Save();


private:

	// Numero de variables de escritura (según tipo)
	uint8_t n_float;
	uint8_t n_uint32_t;
	uint8_t n_int32_t;
	uint8_t n_uint16_t;
	uint8_t n_int16_t;
	uint8_t n_uint8_t;
	uint8_t n_int8_t;
	uint8_t n_bytes = 0;   // Total en bytes

	uint8_t* pointer;              // Puntero de las variables de escritura
	uint32_t* flight_time = 0;     // Tiempo de vuelo del cohete

	// Tiempos y numero de unidades
	uint32_t n_despegue;
	uint32_t t_despegue;
	uint32_t intervalo_despegue;
	uint32_t n_aterrizaje;
	uint32_t t_aterrizaje;

	// Celda de escritura en la eeprom
	uint32_t n_ud_esctiras;        // Numero de unidades de informacion escriras
	uint32_t n_ud_maximas;         // Numero de unidades maximas de escritura

	I2C_HandleTypeDef* hi2c;

	void writeEEPROM_Page(uint16_t address, uint8_t *val, uint8_t tam);
	void float_to_4byte(float* var, uint8_t* aux);
	void _4byte_to_float(uint8_t* aux, float *out);
	void uint16_to_2byte(uint16_t dato_in, uint8_t* dir_dato_out);

};











#endif

















