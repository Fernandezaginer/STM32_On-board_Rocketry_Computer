

#include "CustomLibs/0-utilities.h"
#include "stm32f4xx.h"


#ifndef _SD_H_
#define _SD_H_


class SD_SDIO{
public:

	SD_SDIO(SD_HandleTypeDef* hsd);
	void open_file(char* file_name);
	void write();
	void close_file();

private:
	SD_HandleTypeDef* hsd;
};


#endif


