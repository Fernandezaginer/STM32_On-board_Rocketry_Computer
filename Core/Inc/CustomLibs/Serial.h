



#ifndef _SERIAL_H_
#define _SERIAL_H_




// Queue class
#define QUEUE_LENGTH 512

class Queue{
public:
	void add(uint8_t data);
	void add(uint8_t* data, uint8_t size);
	uint8_t read();
	int16_t available();
	uint8_t data[QUEUE_LENGTH] = {};
private:
	uint8_t start = 0;
	uint8_t end = 0;
};




// Hardware Serial Class
#define DMA_BUFFER_LENGHT 80

class HardwareSerial{
public:

	HardwareSerial(UART_HandleTypeDef* huart, USART_TypeDef* instance);

	// Call this function in DMA Callback:
	void process_data(UART_HandleTypeDef *huart, uint16_t Size);

	int16_t available();
	void write(uint8_t num);
	void write(uint8_t* buffer, uint8_t size);
	void print(int num);
	void print(float num);
	void print(const char* str);
	void println(int num);
	void println(float num);
	void println(const char* str);
	void println();
	uint8_t read();

	void set_dma_callback();
	void add_data(uint8_t* data, uint8_t size);
private:
	Queue queue;
	uint8_t dma_buffer[DMA_BUFFER_LENGHT] = {};
	UART_HandleTypeDef* huart;
	DMA_HandleTypeDef* hdma_usartx_rx;
	USART_TypeDef* instance;
};



#endif
