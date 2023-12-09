


#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/Serial.h"
#include "stdio.h"
#include "stm32f4xx.h"

#ifndef __GPS_H__
#define __GPS_H__

extern UART_HandleTypeDef huart1;
#define	GPS_USART	&huart1

#define GPSBUFSIZE  256

#define c1 "$PUBX,40,GLL,0,0,0,0*5C"
#define c2 "$PUBX,40,ZDA,0,0,0,0*44"
#define c3 "$PUBX,40,VTG,0,0,0,0*5E"
#define c4 "$PUBX,40,GSV,0,0,0,0*59"
#define c5 "$PUBX,40,GSA,0,0,0,0*4E"
#define c6 "$PUBX,40,RMC,0,0,0,0*47"
#define c7 "\xB5\x62\x06\x08\x06\x00\xC8\x00\x01\x00\x01\x00\xDE\x6A"


typedef struct{

    float dec_longitude;
    float dec_latitude;
    float altitude_ft;

    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    char ns, ew;
    int lock;
    int satelites;
    float hdop;
    float msl_altitude;
    char msl_units;

    char rmc_status;
    float speed_k;
    float course_d;
    int date;

    char gll_status;

    float course_t;
    char course_t_unit;
    float course_m;
    char course_m_unit;
    char speed_k_unit;
    float speed_km;
    char speed_km_unit;
} GPS_t;



void GPS_Init();
void GSP_USBPrint(char *data);
void GPS_print_val(char *data, int value);
void GPS_UART_CallBack();
int GPS_validate(char *nmeastr);
void GPS_parse(char *GPSstrParse);
float GPS_nmea_to_dec(float deg_coord, char nsew);


#endif
