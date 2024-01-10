


#include <CustomLibs/FileHandling.h>
#include "mainProgram.h"
#include "fatfs.h"
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Servo.h"
#include "CustomLibs/EEPROM.h"
#include "CustomLibs/PRESION.h"
#include "CustomLibs/Serial.h"
#include "CustomLibs/GPS.h"
#include "CustomLibs/LM35.h"
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
#define TIEMPO_APOGEO_ESTIMADO  13000
#define TIEMPO_VUELO_ESTIMADO   40000
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

bool despegue = false;
bool caida = false;

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


//              EEPROM + SD                                          SOLO SD
// [4]  [4]  [4]  [4]  [4]  [4]   [1]    [1]       [4]      [4]  [4]  [2]  [2]  [2]  [4]   [4]
// LAT  LON  ALT  PRE  Az   TIME  PARAC  SATS      ALT_GPS  Ax   Ay   Gx   Gy   Gz   TEMP  HUM
// F    F    F    F    F    U32   U8     U8

// EEPROM: 26
// SD:     52


// Lecturas:
uint8_t* data = (uint8_t*)malloc(52*sizeof(uint8_t));


float Altitud_BMP = 0.0;
uint32_t* p_flight_time;






//----------------------------------------------------------
//                       Funciones
//----------------------------------------------------------

void cierre_paracaidas();
void apertura_paracaidas();
bool init_modulos();
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


	// Inicializar el actuador del paracaidas
	cierre_paracaidas();


	// Inicializar sensores:
	if(!init_modulos()){
		while(true){
			// Parpadear led error
		}
	}


	// Test UART 1

	for(int i = 0; i < 30; i++){
		char str[50] = {};
		float val = 23.1;
		sprintf(str, "Lectura de valor: \r\n");
		HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 10);
		HAL_Delay(1000);
	}


	// Test unitario LM35:
	printDebug("Temperatura: ");
	printDebugFloat(LM_Temp_Read(&hadc1));



	// Test unitario SD
//	Mount_SD("");
//	Format_SD();
//	Create_File("FILEA.TXT");
//	Create_File("FILEB.TXT");
//	Unmount_SD("");

	// Test unitario GPS OK
//	for(int i = 0; i < 20; i++){
//		HAL_Delay(1000);
//		printDebug("\nLAT: ");
//		printDebug(GPS.dec_latitude);
//		printDebug("\nLON: ");
//		printDebug(GPS.dec_longitude);
//	}

	//  Buscar direcciones I2C
//	I2C_Scan(&hi2c2);
//	I2C_Scan(&hi2c3);

	// Test unitario EEPROM OK
//	if (eeprom.Setup() == true){
//		printDebug("\nEEPROM OK");
//	}
//	else{
//		printDebug("\nEEPROM NOT FOUND");
//	}

	// Test unitario BMP OK
//	printDebug("\nTemperatura: ");
//	printDebugFloat(s_presion.getTemperature());
//	printDebug("\nPresion: ");
//	printDebugFloat(s_presion.getPressure());
//	printDebug("\nAltitud: ");
//	printDebugFloat(s_presion.getAltitude());

	// Test unitario Servo OK
//	for(int i = 0; i < 5; i++){
//		HAL_Delay(2000);
//		apertura_paracaidas();
//		HAL_Delay(2000);
//		cierre_paracaidas();
//	}




	// Configurar EEPROM:
	eeprom.ConfigUnitSave(5,1,0,0,0,2,0);
	eeprom.ConfigPointerSave(data);
	eeprom.ConfigPointerTime(&(p_flight_time[20]));   // verificar funcionamiento
	eeprom.ConfigSpace(0.6, 0.4, TIEMPO_APOGEO_ESTIMADO, TIEMPO_VUELO_ESTIMADO);



	// Espera a obtener cobertura GPS
	while(GPS.dec_latitude == 0.0 || GPS.dec_longitude == 0.0){
		HAL_Delay(1000);
		printDebug("Esperando senal GPS ...");
	}

	// Listo para despegue



}






//-------------------------------------------------
//                   LOOP
//-------------------------------------------------



void loop(){


	// DESPEGUE DEL COHETE E INICIO DE LA GRABACIÓN
	if(COND_DESPEGUE_1 && COND_DESPEGUE_2 && COND_DESPEGUE_3 && !inicio_grabacion){
		t_inicio = HAL_GetTick();
		despegue = true;
	}

	// CONTROL DEL PARACAIDAS
	if (Altitud_BMP > alt_max && despegue && (FLIGHT_TIME > T_MIN_PARACAIDAS)) {
		alt_max = Altitud_BMP;  // Obtener la altura maxima del vuelo
	}
	if(COND_APERTURA_1 && COND_APERTURA_2){
		apertura_paracaidas();
		despegue = false;
		caida = true;
	}


	*p_flight_time = FLIGHT_TIME;
	read_save_data();


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

bool init_modulos(){
	bool test_ok = true;
	if(!eeprom.Setup()){
		printDebug("[ERROR] Inicializacion memoria EEPROM\n\n");
		test_ok = false;
	}
	if(!s_presion.init()){
		printDebug("[ERROR] Inicializacion sesnor PRESION\n\n");
		test_ok = false;
	}
	if(test_ok){
		printDebug("INICIALIZACION DE TODOS LOS SENSORES CORRECTA\n\n");
	}
	return test_ok;
}


void read_save_data(){


//              EEPROM + SD                                          SOLO SD
// [4]  [4]  [4]  [4]  [4]  [4]   [1]    [1]       [4]      [4]  [4]  [2]  [2]  [2]  [4]   [4]
// LAT  LON  ALT  PRE  Az   TIME  PARAC  SATS      ALT_GPS  Ax   Ay   Gx   Gy   Gz   TEMP  HUM
// F    F    F    F    F    U32   U8     U8



// saveUint32(float val, data[0])
// saveFloat(float val, data[4])
// saveFloat(float val, data[8])
// saveUint8(float val, data[12])
// saveFloat(float val, data[16])
// saveFloat(float val, data[20])
// saveFloat(float val, data[24])
// saveInt16(float val, data[25])
// saveUint8(float val, data[26])
// saveFloat(float val, data[30])
// saveFloat(float val, data[34])
// saveFloat(float val, data[38])
// saveInt16(float val, data[40])
// saveInt16(float val, data[42])
// saveFloat(float val, data[44])
// saveFloat(float val, data[48])


	// EEPROM write
	eeprom.loop(despegue, caida);

	// SD write
	//...
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
  savedata((uint8_t*)&val, dir, 4);
}

void saveUint8(uint8_t val, uint8_t* dir){
  savedata(&val, dir, 1);
}

void saveInt8(int8_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 1);
}

void saveUint16(uint16_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 2);
}

void saveInt16(int16_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 2);
}

void saveUint32(uint32_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 4);
}

void saveInt32(uint32_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 4);
}

