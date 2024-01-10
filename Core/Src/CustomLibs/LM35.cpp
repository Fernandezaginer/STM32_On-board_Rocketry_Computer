#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "CustomLibs/LM35.h"

float LM_Temp_Read(ADC_HandleTypeDef* hadc){
	uint32_t ADC_val;
	float temp;
	HAL_ADC_Start(hadc);
	if(HAL_ADC_PollForConversion(hadc, 5)==HAL_OK){
		ADC_val=HAL_ADC_GetValue(hadc);
		temp = ((ADC_val * 3.0)/4095.0 - 0.5)*100.0;
	}
	HAL_ADC_Stop(hadc);
	return temp;
}
