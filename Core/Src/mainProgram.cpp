

#include <mainProgram.h>
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Servo.h"


//-------------------------------------------------
//              Parámetros Cohete
//-------------------------------------------------
#define ACC_START             3.0          // g
#define T_MIN_PARACAIDAS      5000         // ms
#define T_MAX_PARACAIDAS      15000        // ms
#define DIF_ALTURA_APERTURA   20.0         // m
#define DIF_ALTURA_ALARMA     200.0        // m
#define T_MAX_ALARMA          30000        // ms
// Apogeo estimado: 1500 m
// Tiempo estimado: 13 s


//-------------------------------------------------
//         Control apertura paracaidas
//-------------------------------------------------
#define COND_APERTURA_1 (Altitud_BMP < (alt_max - DIF_ALTURA_APERTURA)) && (FLIGHT_TIME > T_MIN_PARACAIDAS)
#define COND_APERTURA_2 (FLIGHT_TIME > T_MAX_PARACAIDAS) && (fin_paracaidas == false)
#define FLIGHT_TIME (HAL_GetTick() - t_inicio)
float alt_max = 0.0;
uint32_t t_inicio = 0;
bool start = false;
bool fin_paracaidas = false;
bool fin_alarma = false;



#define COND_DESPEGUE_1 true
#define COND_DESPEGUE_2 true
#define COND_DESPEGUE_3 true



//-------------------------------------------------
//            Sensores y actuadores
//-------------------------------------------------
Servo paracaidas;
SERVO_PIN servo_pin = {};






// Mediciones sensores:
float Altitud_BMP = 0.0;


void setup(){

	//test_modulos();

}


void loop(){


	// DESPEGUE DEL COHETE E INICIO DE LA GRABACIÓN
	if(COND_DESPEGUE_1 && COND_DESPEGUE_2 && COND_DESPEGUE_3){

	}



	// CONTROL DEL PARACAIDAS
	if (Altitud_BMP > alt_max && (FLIGHT_TIME > T_MIN_PARACAIDAS)) {
		alt_max = Altitud_BMP;  // Obtener la altura maxima del vuelo
    }
	if(COND_APERTURA_1 && COND_APERTURA_2){
		apertura_paracaidas();
	}



}




// -------------------------------------------------------
//              FUNCIONES
// -------------------------------------------------------

void apertura_paracaidas(){
	fin_paracaidas = false;
}



void cierre_paracaidas(){

}


void test_modulos(){

}


void read_save_data(){


}





