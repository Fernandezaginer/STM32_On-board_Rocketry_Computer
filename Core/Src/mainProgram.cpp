


#include <CustomLibs/FileHandling.h>
#include "mainProgram.h"
#include "fatfs.h"
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Servo.h"
#include "CustomLibs/EEPROM.h"
#include "CustomLibs/PRESION.h"
#include "CustomLibs/Serial.h"
#include "CustomLibs/GPS.h"
#include "CustomLibs/SD.h"



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
Bmp280 s_presion = *(new Bmp280(&hi2c2));
EEPROM eeprom = *(new EEPROM(&hi2c3));
extern GPS_t GPS;


// EEPROM  (28)
// [4]   [4]  [4]  [1]   [4]  [4]  [4]  [2]  [1]
// TIME  LAT  LON  SATS  ALT  PRE  Az   Gz   PARAC.
// SD  (48)
// [4]   [4]  [4]  [1]   [4]  [4]  [4]  [2]  [1]       [4]     [4]   [4]  [2]  [2]  [4]
// TIME  LAT  LON  SATS  ALT  PRE  Az   Gz   PARAC.    ALT_GPS  Ax   Ay   Gx   Gy   TEMP



// Lecturas:
uint8_t* data = (uint8_t*)malloc(48*sizeof(uint8_t));













// led error
// led ok

// Mediciones sensores:
float Altitud_BMP = 0.0;





//----------------------------------------------------------
//                       Funciones
//----------------------------------------------------------

void cierre_paracaidas();
void apertura_paracaidas();
bool test_modulos();
void read_save_data();
void savedata(uint8_t* dir_dato, uint8_t* dir, uint8_t size);
void saveFloat(float val, uint8_t* dir);
void saveUint8(uint8_t val, uint8_t* dir);
void saveInt8(int8_t val, uint8_t* dir);
void saveUint16(uint16_t val, uint8_t* dir);
void saveInt16(int16_t val, uint8_t* dir);
void saveUint32(uint32_t val, uint8_t* dir);
void saveInt32(uint32_t val, uint8_t* dir);




//----------------------------------------------------------
//                       Callbacks
//----------------------------------------------------------

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == &huart1) GPS_UART_CallBack();
}




//-------------------------------------------------
//                     SETUP
//-------------------------------------------------

void setup(){


	// Inicializar sensores:
	cierre_paracaidas();
	s_presion.init();
	GPS_Init();


	// Test unitario SD
//	Mount_SD("");
//	Format_SD();
//	Create_File("FILEA.TXT");
//	Create_File("FILEB.TXT");
//	Unmount_SD("");


	// Test unitario GPS OK
	for(int i = 0; i < 20; i++){
		HAL_Delay(1000);
		printDebug("\nLAT: ");
		printDebugFloat(GPS.dec_latitude);
		printDebug("\nLON: ");
		printDebugFloat(GPS.dec_longitude);
	}


	//  Buscar direcciones I2C
	I2C_Scan(&hi2c2);
	I2C_Scan(&hi2c3);


	// Test unitario EEPROM OK
	if (eeprom.Setup() == true){
		printDebug("\nEEPROM OK");
	}
	else{
		printDebug("\nEEPROM NOT FOUND");
	}


	// Test unitario BMP OK
	printDebug("\nTemperatura: ");
	printDebugFloat(s_presion.getTemperature());
	printDebug("\nPresion: ");
	printDebugFloat(s_presion.getPressure());
	printDebug("\nAltitud: ");
	printDebugFloat(s_presion.getAltitude());



	// Test unitario Servo OK
	for(int i = 0; i < 5; i++){
		HAL_Delay(2000);
		apertura_paracaidas();
		HAL_Delay(2000);
		cierre_paracaidas();
	}




	// Configurar EEPROM:








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
	paracaidas.setup(&servo_pin);
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

// [4]   [4]  [4]  [1]   [4]  [4]  [4]  [2]  [1]       [4]     [4]   [4]  [2]  [2]  [4]
// TIME  LAT  LON  SATS  ALT  PRE  Az   Gz   PARAC.    ALT_GPS  Ax   Ay   Gx   Gy   TEMP
// saveUint32(float val, data[0])
// saveFloat(float val, data[4])
// saveFloat(float val, data[8])
// saveUint8(float val, data[12])
// saveFloat(float val, data[13])
// saveFloat(float val, data[17])
// saveFloat(float val, data[21])
// saveInt16(float val, data[25])
// saveUint8(float val, data[27])
// saveFloat(float val, data[28])
// saveFloat(float val, data[32])
// saveFloat(float val, data[36])
// saveInt16(float val, data[40])
// saveInt16(float val, data[42])
// saveFloat(float val, data[44])

// EEPROM write


// SD write


}


void savedata(uint8_t* dir_dato, uint8_t* dir, uint8_t size) {
  uint8_t* puntero_dato = dir_dato;
  for (uint8_t i = 0; i < size; i++) {
    *dir = *(puntero_dato);
    puntero_dato++;
    dir++;
  }
}

void saveFloat(float val, uint8_t* dir){
  savedata(&val, dir, 4);
}

void saveUint8(uint8_t val, uint8_t* dir){
  savedata(&val, dir, 1);
}

void saveInt8(int8_t val, uint8_t* dir){
  savedata(&val, dir, 1);
}

void saveUint16(uint16_t val, uint8_t* dir){
  savedata(&val, dir, 2);
}

void saveInt16(int16_t val, uint8_t* dir){
  savedata(&val, dir, 2);
}

void saveUint32(uint32_t val, uint8_t* dir){
  savedata(&val, dir, 4);
}

void saveInt32(uint32_t val, uint8_t* dir){
  savedata(&val, dir, 4);
}

