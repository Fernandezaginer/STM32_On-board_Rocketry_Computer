

#include "stm32f4xx.h"
#include "CustomLibs/Serial.h"
#include "CustomLibs/0-Utilities.h"
#include "stdio.h"
#include <cstring>







// -----------    Queue    --------------------

void Queue::add(uint8_t data_add){
	if((end + 1) < QUEUE_LENGTH){
		data[end] = data_add;
		end++;
	}
	else{
		data[end] = data_add;
		end = 0;
	}
}

void Queue::add(uint8_t* data, uint8_t size){
	for(int i = 0; i < size; i++){
		this->add(data[i]);
		//printDebug((char*)&data[i]);
	}
}

uint8_t Queue::read(){
	if(this->available() > 0){
		if(start + 1 < QUEUE_LENGTH){
			start++;
			return data[start-1];
		}
		else{
			start = 0;
			return data[QUEUE_LENGTH - 1];
		}
	}
	else{
		return 0;
	}
}


int16_t Queue::available(){
	if(end > start){
		return (int16_t)(end - start);
	}
	if(end < start){
		return (int16_t)(QUEUE_LENGTH - ((int16_t)(start - end)));
	}
	else{
		return -1;
	}
}













// -----------    HardwareSerial    ----------------

HardwareSerial::HardwareSerial(UART_HandleTypeDef* huart, USART_TypeDef* instance){
	this->huart = huart;
	this->instance = instance;
	this->set_dma_callback();
}

int16_t HardwareSerial::available(){
	return this->queue.available();
}

void HardwareSerial::write(uint8_t num){
	HAL_UART_Transmit(this->huart, &num, 1, 10);
}

void HardwareSerial::write(uint8_t* buffer, uint8_t size){
	HAL_UART_Transmit(this->huart, buffer, size, 10);
}

void HardwareSerial::print(int num){
	char str[10];
	sprintf(str, "%d", num);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	HAL_UART_Transmit(this->huart, (uint8_t*)str, i, 10);

}

void HardwareSerial::print(float num){
	char str[10];
	sprintf(str, "%f", num);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	HAL_UART_Transmit(this->huart, (uint8_t*)str, i, 10);
}


void HardwareSerial::print(const char* str){
	HAL_UART_Transmit(this->huart, (uint8_t*)str, strlen(str), 10);
}


void HardwareSerial::println(int num){
	char str[10];
	sprintf(str, "%d", num);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	HAL_UART_Transmit(this->huart, (uint8_t*)str, i, 10);
	uint8_t aux[2] = {'\r', '\n'};
	HAL_UART_Transmit(this->huart, aux, sizeof(aux), 50);
}

void HardwareSerial::println(float num){
	char str[10];
	sprintf(str, "%f", num);
	int i = 0;
	while (str[i] != '\0') {
		i++;
	}
	HAL_UART_Transmit(this->huart, (uint8_t*)str, i, 10);
	uint8_t aux[2] = {'\r', '\n'};
	HAL_UART_Transmit(this->huart, aux, sizeof(aux), 50);
}

void HardwareSerial::println(const char* str){
	HAL_UART_Transmit(this->huart, (uint8_t*)str, strlen(str), 50);
	uint8_t aux[2] = {'\r', '\n'};
	HAL_UART_Transmit(this->huart, aux, sizeof(aux), 50);
}

void HardwareSerial::println(){
	uint8_t aux[2] = {'\r', '\n'};
	HAL_UART_Transmit(this->huart, aux, sizeof(aux), 50);
}


uint8_t HardwareSerial::read(){
	return this->queue.read();
}


void HardwareSerial::process_data(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart->Instance == this->instance){
		this->add_data(dma_buffer, Size);
		this->set_dma_callback();
	}
}


void HardwareSerial::add_data(uint8_t* data, uint8_t size){
	this->queue.add(data, size);
}

void HardwareSerial::set_dma_callback(){
	//HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*) dma_buffer, DMA_BUFFER_LENGHT);
	//__HAL_DMA_DISABLE_IT(hdma_usartx_rx, DMA_IT_HT);
}

