


#include "CustomLibs/0-Utilities.h"
#include "stdio.h"
#include "stm32f4xx.h"


#ifndef _TELEMETRIA_H_
#define _TELEMETRIA_H_

#define PERIOD_SEND 500

class telemetria_uart{
public:
	telemetria_uart(UART_HandleTypeDef* huart);
	void config_p_data_estados(bool* init_error, bool* wait_gps, bool* despegue, bool* caida);
	void config_pdata(uint8_t* p_d);
	void loop();

private:
	char* get_estado_str();

	float g_f(uint8_t* pdata);
	uint32_t g_u32(uint8_t* pdata);
	uint8_t g_u8(uint8_t* pdata);

	uint8_t* p_d;
	UART_HandleTypeDef* huart;
	bool* init_error;
	bool* wait_gps;
	bool* despegue;
	bool* caida;
	uint32_t last_tic = 0;
};




#endif




