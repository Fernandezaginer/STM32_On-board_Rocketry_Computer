

#include <mainProgram.h>
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Servo.h"
#include "CustomLibs/EEPROM.h"



extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern SD_HandleTypeDef hsd;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;




//-------------------------------------------------
//              Parámetros Cohete
//-------------------------------------------------
#define ACC_START             3.0          // g
#define T_MIN_PARACAIDAS      5000         // ms
#define T_MAX_PARACAIDAS      15000        // ms
#define DIF_ALTURA_APERTURA   20.0         // m
#define DIF_ALTURA_ALARMA     200.0        // m
#define T_MAX_ALARMA          30000        // ms
// Apogeo estimado: 1500 m  (13 s)




//-------------------------------------------------
//         Control apertura paracaidas
//-------------------------------------------------
#define COND_APERTURA_1 	(Altitud_BMP < (alt_max - DIF_ALTURA_APERTURA)) && (FLIGHT_TIME > T_MIN_PARACAIDAS)
#define COND_APERTURA_2 	(FLIGHT_TIME > T_MAX_PARACAIDAS) && (fin_paracaidas == false)
#define FLIGHT_TIME 		(HAL_GetTick() - t_inicio)

uint32_t t_inicio = 0;
float alt_max = 0.0;
bool inicio_grabacion = false;
bool fin_paracaidas = false;
bool fin_alarma = false;


#define COND_DESPEGUE_1 true
#define COND_DESPEGUE_2 true
#define COND_DESPEGUE_3 true



//-------------------------------------------------
//            Sensores y actuadores
//-------------------------------------------------
Servo paracaidas;
SERVO_PIN servo_pin = {&htim1, TIM_CHANNEL_1};




// led error
// led ok




// Mediciones sensores:
float Altitud_BMP = 0.0;







void cierre_paracaidas();
void apertura_paracaidas();
bool test_modulos();
void read_save_data();


//-------------------------------------------------
//                     SETUP
//-------------------------------------------------

void setup(){


	// Cerrar el paracaidas
	cierre_paracaidas();


	// Comprobar el funcionamiento de los modulos
	if(test_modulos()){

	}
	else{

	}

	// Espera a obtener cobertura GPS




	// Listo para despegue



}







//-------------------------------------------------
//                   LOOP
//-------------------------------------------------

void loop(){


	// DESPEGUE DEL COHETE E INICIO DE LA GRABACIÓN
	if(COND_DESPEGUE_1 && COND_DESPEGUE_2 && COND_DESPEGUE_3 && !inicio_grabacion){
		t_inicio = HAL_GetTick();
		inicio_grabacion = true;
	}

	// CONTROL DEL PARACAIDAS
	if (Altitud_BMP > alt_max && (FLIGHT_TIME > T_MIN_PARACAIDAS)) {
		alt_max = Altitud_BMP;  // Obtener la altura maxima del vuelo
    }
	if(COND_APERTURA_1 && COND_APERTURA_2){
		apertura_paracaidas();
	}


	// GRABACIÓN DE DATOS
	if(inicio_grabacion == true){
		read_save_data();
	}

}









// -------------------------------------------------------
//                       FUNCIONES
// -------------------------------------------------------



void cierre_paracaidas(){
	paracaidas.setup(servo_pin);
	paracaidas.attach();
	paracaidas.write(0);
}

void apertura_paracaidas(){
	paracaidas.write(180);
	fin_paracaidas = false;
}



bool test_modulos(){
	return true;
}


void read_save_data(){


}





