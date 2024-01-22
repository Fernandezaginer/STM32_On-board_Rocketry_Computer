


#include "mainProgram.h"
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Servo.h"
#include "CustomLibs/EEPROM.h"
#include "CustomLibs/PRESION.h"
#include "CustomLibs/telemetria.h"
#include "CustomLibs/Serial.h"
#include "CustomLibs/GPS.h"
#include "CustomLibs/LM35.h"
#include "CustomLibs/DHT11.h"
#include "CustomLibs/BUZZ.h"
#include "CustomLibs/LED.h"
#include "CustomLibs/acc.h"
#include "CustomLibs/SD.h"
#include "CustomLibs/acc.h"
#include "stdio.h"
#include "math.h"
#include "string.h"



extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
//extern SD_HandleTypeDef hsd;




//-------------------------------------------------
//              Parámetros Cohete
//-------------------------------------------------
#define ACC_START             2.0          // g
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


// Variables de estado
bool error_init_modulos = false;
bool espera_gps = true;
bool despegue = false;
bool caida = false;

// Aceleraciones MPU6050:
float acc[3] = {};

#define COND_DESPEGUE (abs(acc[2]) < ACC_START)





//-------------------------------------------------
//            Sensores y actuadores
//-------------------------------------------------
Servo paracaidas;
SERVO_PIN servo_pin = {&htim1, TIM_CHANNEL_1};
Bmp280 s_presion = *(new Bmp280(&hi2c2));
EEPROM eeprom = *(new EEPROM(&hi2c3));
MPU6050 accelerometro = *(new MPU6050(&hi2c2));
telemetria_uart telemetria = *(new telemetria_uart(&huart2));
//DHT11 dht11 = *(new DHT11(GPIOE, GPIO_PIN_1, &htim4));

extern GPS_t GPS;



//              SOLO TELM                                           EEPROM + TELM
//    [4]      [4]  [4]  [4]  [4]  [4]  [4]   [4]     [4]  [4]  [4]  [4]  [4]  [4]   [1]    [1]
//    ALT_GPS  Ax   Ay   Gx   Gy   Gz   TEMP  HUM     LAT  LON  ALT  PRE  Az   TIME  PARAC  SATS
//    F        F    F    F    F    F    F     F       F    F    F    F    F    U32   U8     U8


// Lecturas:
#define SIZE_DATA 58
uint8_t* data = (uint8_t*)malloc(SIZE_DATA*sizeof(uint8_t));


float Altitud_BMP = 0.0;
uint32_t* p_flight_time;










//----------------------------------------------------------
//                       Funciones
//----------------------------------------------------------

void read_data();
void cierre_paracaidas();
void apertura_paracaidas();
bool init_modulos();
void read_save_data();
void test_unitarios();
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


	//test_unitarios();

	// Inicializar el actuador del paracaidas
	cierre_paracaidas();
	HAL_Delay(1000);
	LED_Init(&htim4);

	// Configurar EEPROM:
	eeprom.Setup();
	eeprom.ConfigUnitSave(5,1,0,0,0,2,0);
	eeprom.ConfigPointerSave(&data[32]);
	eeprom.ConfigPointerTime(p_flight_time);
	eeprom.ConfigSpace(0.6, 0.4, TIEMPO_APOGEO_ESTIMADO, TIEMPO_VUELO_ESTIMADO);

	// Read eeprom:
	//eeprom.PrintDebug();

	// Configurar la telemetría:
	telemetria.config_p_data_estados(&error_init_modulos, &espera_gps, &despegue, &caida);
	telemetria.config_pdata(data);

	// clear pdata:
	for (int i = 0; i < SIZE_DATA; i++){
		data[i] = 0;
	}

	// -------------------------------------------
	//       FASE 1 Inicializar sensores:
	// -------------------------------------------

	if(!init_modulos()){
		error_init_modulos = true;
		while(true){
			printDebug("[Error de inicializacion]");
			telemetria.loop();
			HAL_Delay(100);
			LED_Toggle(&htim4);
		}
	}



	// -------------------------------------------
	//       FASE 2    Señal GPS:
	// -------------------------------------------

	printDebug("Esperando senal GPS ...\n");
	while(GPS.dec_latitude == 0.0 || GPS.dec_longitude == 0.0){
		read_data();
		telemetria.loop();
	}




	// -------------------------------------------
	//       FASE 3   Esperar a despegue:
	// -------------------------------------------


	// Listo para despegue, esperar a detectar aceleración
	espera_gps = false;
	printDebug("Senal GPS OK ...\n");
	printDebug("Esperando a tetectar aceleración de despegue ...\n");
	while(COND_DESPEGUE){
		read_data();
		telemetria.loop();
		LED_Toggle(&htim4);
	}
	printDebug("Despegue\n");
	LED_On(&htim4);
	t_inicio = HAL_GetTick();
	despegue = true;

}






//-------------------------------------------------
//                   LOOP
//-------------------------------------------------

uint32_t time = 0;
bool fp = 0;


void loop(){



	// CONTROL DEL PARACAIDAS
	if (Altitud_BMP > alt_max && despegue && (FLIGHT_TIME > T_MIN_PARACAIDAS)) {
		alt_max = Altitud_BMP;  // Obtener la altura maxima del vuelo
	}
	if(COND_APERTURA_1 || COND_APERTURA_2){
		if(fp == 0){
			fp = 1;
			BUZZ_Init(&htim3);
			printDebug("Apertura Paracaidas");
		}
		apertura_paracaidas();
		despegue = false;
		caida = true;
	}


	time = FLIGHT_TIME;
	p_flight_time = &time;
	read_save_data();
	telemetria.loop();
}









// -------------------------------------------------------
//                       FUNCIONES
// -------------------------------------------------------


void cierre_paracaidas(){
	paracaidas.setup(&servo_pin);
	paracaidas.attach();
	paracaidas.write(20);
}

void apertura_paracaidas(){
	paracaidas.write(80);
	fin_paracaidas = false;
}

bool init_modulos(){
	bool test_ok = true;
	accelerometro.MPU6050_Init();
	GPS_Init();
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



void read_data(){

	//              SOLO TELM                                           EEPROM + TELM
	//    [4]      [4]  [4]  [4]  [4]  [4]  [4]   [4]     [4]  [4]  [4]  [4]  [4]  [4]   [1]    [1]
	//    ALT_GPS  Ax   Ay   Gx   Gy   Gz   TEMP  HUM     LAT  LON  ALT  PRE  Az   TIME  PARAC  SATS
	//    F        F    F    F    F    F    F     F       F    F    F    F    F    U32   U8     U8

	accelerometro.MPU6050_Read_Accel(acc);

	 saveFloat(GPS.altitude_ft, (uint8_t*)&data[0]);
	 saveFloat(acc[0], (uint8_t*)&data[4]);
	 saveFloat(acc[1], (uint8_t*)&data[8]);
	 saveFloat(0.0, (uint8_t*)&data[12]);
	 saveFloat(0.0, (uint8_t*)&data[16]);
	 saveFloat(0.0, (uint8_t*)&data[20]);
	 saveFloat(LM_Temp_Read(&hadc1), (uint8_t*)&data[24]);
	 saveFloat(0.0, (uint8_t*)&data[28]);

	 saveFloat(GPS.dec_latitude, (uint8_t*)&data[32]);
	 saveFloat(GPS.dec_longitude, (uint8_t*)&data[36]);
	 saveFloat(s_presion.getAltitude(), (uint8_t*)&data[40]);
	 saveFloat(s_presion.getPressure(), (uint8_t*)&data[44]);
	 saveFloat(acc[2], (uint8_t*)&data[48]);
	 saveUint32(FLIGHT_TIME, (uint8_t*)&data[52]);
	 saveUint8((uint8_t)caida, (uint8_t*)&data[56]);
	 saveUint8((uint8_t)GPS.satelites, (uint8_t*)&data[57]);

}


void read_save_data(){

	read_data();

	// EEPROM write
	eeprom.loop(despegue, caida);

	// SD write
	//...
}



void test_unitarios(){


	// Test telemetría: OK
//	char data_[60] = {};
//	sprintf(data_, "Test Hola Mundo\n");
//	HAL_UART_Transmit(&huart2, (uint8_t*)data_, strlen(data_), HAL_MAX_DELAY);
//
//	sprintf(data_, "Lectura de temperatura: %.3f\n", LM_Temp_Read(&hadc1));
//	HAL_UART_Transmit(&huart2, (uint8_t*)data_, strlen(data_), HAL_MAX_DELAY);


	// Test DHT11, fallido
//	dht11.DHT11_Init();
//	int16_t temp;
//	temp = dht11.readTemperature();
//	dht11.DHT11_Read_Sensor();

	// Test unitario SD, fallido
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



}




// -------------------------------------------------------
//                       FUNCIONES
// -------------------------------------------------------


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

void Float(int16_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 2);
}

void saveUint32(uint32_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 4);
}

void saveInt32(uint32_t val, uint8_t* dir){
  savedata((uint8_t*)&val, dir, 4);
}

