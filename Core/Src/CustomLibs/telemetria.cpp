


#include "CustomLibs/telemetria.h"
#include "stdio.h"
#include "string.h"


telemetria_uart::telemetria_uart(UART_HandleTypeDef* huart){
	this->huart = huart;
}


void telemetria_uart::config_p_data_estados(bool* init_error, bool* wait_gps, bool* despegue, bool* caida){
	this->init_error = init_error;
	this->wait_gps = wait_gps;
	this->despegue = despegue;
	this->caida = caida;
}

void telemetria_uart::config_pdata(uint8_t* p_d){
	this->p_d = p_d;
}

void telemetria_uart::loop(){

	if(HAL_GetTick() > (last_tic + PERIOD_SEND)){
		last_tic = HAL_GetTick();
		char data1[100] = {};
		char data2[100] = {};
		char data3[100] = {};

		sprintf(data1,
				"  %.1f \t %.2f \t %.2f \t %.2f \t %.2f",
				g_f(&p_d[0]), g_f(&p_d[4]), g_f(&p_d[8]), g_f(&p_d[12]), g_f(&p_d[16]));

		sprintf(data2,
				" \t %.2f \t %.2f \t %.2f \t %.6f \t %.6f",
				 g_f(&p_d[20]), g_f(&p_d[24]), g_f(&p_d[28]), g_f(&p_d[32]), g_f(&p_d[36]));


		sprintf(data3,
				" \t %.2fm \t %.1fPa \t %.2f \t %dms \t %d \t %d\r\n",
				 g_f(&p_d[40]), g_f(&p_d[44]), g_f(&p_d[48]), g_u32(&p_d[52]), g_u8(&p_d[56]),
				 g_u8(&p_d[57]));

		HAL_UART_Transmit(huart, (uint8_t*)this->get_estado_str(), strlen(this->get_estado_str()), 50);
		HAL_UART_Transmit(huart, (uint8_t*)data1, strlen(data1), HAL_MAX_DELAY);
		HAL_UART_Transmit(huart, (uint8_t*)data2, strlen(data2), HAL_MAX_DELAY);
		HAL_UART_Transmit(huart, (uint8_t*)data3, strlen(data3), HAL_MAX_DELAY);
	}
}



char* telemetria_uart::get_estado_str(){
	char* estados_str[5] = {"SENSOR INITIALIZATION ERROR", "WAITING GPS SIGNAL", "READY FOR TAKEOFF", "LIFT OFF", "LANDING"};

	if(*init_error){
		return estados_str[0];
	}
	if(*wait_gps){
		return estados_str[1];
	}
	if(!(*despegue) && !(*caida)){
		return estados_str[2];
	}
	if(*despegue){
		return estados_str[3];
	}
	if(*caida){
		return estados_str[4];
	}
	return estados_str[4];
}



float telemetria_uart::g_f(uint8_t* pdata){
  uint32_t mem_aux = 0;
  mem_aux |= pdata[0];
  mem_aux |= (uint32_t)(pdata[1]) << 8;
  mem_aux |= (uint32_t)(pdata[2]) << 16;
  mem_aux |= (uint32_t)(pdata[3]) << 24;
  return (float)(*((float*)&mem_aux));
}

uint32_t telemetria_uart::g_u32(uint8_t* pdata){
	uint32_t mem_aux = 0;
	mem_aux |= pdata[0];
	mem_aux |= (uint32_t)(pdata[1]) << 8;
	mem_aux |= (uint32_t)(pdata[2]) << 16;
	mem_aux |= (uint32_t)(pdata[3]) << 24;
	return mem_aux;
}

uint8_t telemetria_uart::g_u8(uint8_t* pdata){
	return *pdata;
}










