


#include "CustomLibs/0-Utilities.h"
#include "stdio.h"
#include "stm32f4xx.h"


#ifndef _GPS_H_
#define _GPS_H_



#define byte unsigned char


#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001


enum A{
GPS_INVALID_AGE = 0xFFFFFFFF,      GPS_INVALID_ANGLE = 999999999,
GPS_INVALID_ALTITUDE = 999999999,  GPS_INVALID_DATE = 0,
GPS_INVALID_TIME = 0xFFFFFFFF,		 GPS_INVALID_SPEED = 999999999,
GPS_INVALID_FIX_TIME = 0xFFFFFFFF, GPS_INVALID_SATELLITES = 0xFF,
GPS_INVALID_HDOP = 0xFFFFFFFF
};


class GPS{
public:

  void TinyGPS();

  bool encode(int8_t c);

  void get_position(float *latitude, float *longitude, unsigned long *fix_age);
  void get_datetime(unsigned long *date, unsigned long *time, unsigned long *age);
  long altitude();
  unsigned long course();
  unsigned long speed();
  unsigned short satellites();

  unsigned long hdop();

  void f_get_position(float *latitude, float *longitude, unsigned long *fix_age);
  void crack_datetime(int *year, byte *month, byte *day,
	byte *hour, byte *minute, byte *second, byte *hundredths, unsigned long *fix_age);
  float f_altitude();
  float f_course();
  float f_speed_knots();
  float f_speed_mph();
  float f_speed_mps();
  float f_speed_kmph();

  float distance_between (float lat1, float long1, float lat2, float long2);
  float course_to (float lat1, float long1, float lat2, float long2);



private:
  int from_hex(char a);
  unsigned long parse_decimal();
  unsigned long parse_degrees();
  bool term_complete();
  long gpsatol(const char *str);
  int gpsstrcmp(const char *str1, const char *str2);

};






#endif


