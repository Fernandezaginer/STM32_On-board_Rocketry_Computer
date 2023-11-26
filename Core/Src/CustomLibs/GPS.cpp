
#include "stdio.h"
#include <math.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include <malloc.h>
#include "CustomLibs/0-Utilities.h"
#include "CustomLibs/GPS.h"

#define TWO_PI M_PI*2
#define sq(n) (n*n)
#define radians(angleDegrees) (angleDegrees * M_PI / 180.0)
#define degrees(angleRadians) (angleRadians * 180.0 / M_PI)

unsigned long int millis() {
	return 0;
}

const float GPS_INVALID_F_ANGLE = 1000.0;
const float GPS_INVALID_F_ALTITUDE = 1000000.0;
const float GPS_INVALID_F_SPEED = -1.0;

enum B {
	_GPS_SENTENCE_GPGGA, _GPS_SENTENCE_GPRMC, _GPS_SENTENCE_OTHER
};

// properties
unsigned long _time, _new_time;
unsigned long _date, _new_date;
long _latitude, _new_latitude;
long _longitude, _new_longitude;
long _altitude, _new_altitude;
unsigned long _speed, _new_speed;
unsigned long _course, _new_course;
unsigned long _hdop, _new_hdop;
unsigned short _numsats, _new_numsats;

unsigned long _last_time_fix, _new_time_fix;
unsigned long _last_position_fix, _new_position_fix;

// parsing state variables
byte _parity;
bool _is_checksum_term;
char _term[15];
byte _sentence_type;
byte _term_number;
byte _term_offset;
bool _gps_data_good;


// internal utilities
int from_hex(char a);
unsigned long parse_decimal();
unsigned long parse_degrees();
bool term_complete();
bool gpsisdigit(char c) {
	return c >= '0' && c <= '9';
}
long gpsatol(const char *str);
int gpsstrcmp(const char *str1, const char *str2);

#define _GPRMC_TERM   "GPRMC"
#define _GPGGA_TERM   "GPGGA"

bool GPS::encode(int8_t c) {
	bool valid_sentence = false;

	switch (c) {
	case ',':
		_parity ^= c;
		break;
	case '\r':
		break;
	case '\n':
		break;
	case '*':
		if (_term_offset < sizeof(_term)) {
			_term[_term_offset] = 0;
			valid_sentence = term_complete();
		}
		++_term_number;
		_term_offset = 0;
		_is_checksum_term = c == '*';
		return valid_sentence;
		break;

	case '$':
		_term_number = _term_offset = 0;
		_parity = 0;
		_sentence_type = _GPS_SENTENCE_OTHER;
		_is_checksum_term = false;
		_gps_data_good = false;
		return valid_sentence;
		break;
	}

	// ordinary characters
	if (_term_offset < sizeof(_term) - 1) {
		_term[_term_offset++] = c;
	}
	if (!_is_checksum_term) {
		_parity ^= c;
	}

	return valid_sentence;
}

void GPS::TinyGPS() {
	_time = GPS_INVALID_TIME;
	_date = GPS_INVALID_DATE;
	_latitude = GPS_INVALID_ANGLE;
	_longitude = GPS_INVALID_ANGLE;
	_altitude = GPS_INVALID_ALTITUDE;
	_speed = GPS_INVALID_SPEED;
	_course = GPS_INVALID_ANGLE;
	_hdop = GPS_INVALID_HDOP;
	_numsats = GPS_INVALID_SATELLITES;
	_last_time_fix = GPS_INVALID_FIX_TIME;
	_last_position_fix = GPS_INVALID_FIX_TIME;
	_parity = 0;
	_is_checksum_term = false;
	_sentence_type = _GPS_SENTENCE_OTHER;
	_term_number = 0;
	_term_offset = 0;
	_gps_data_good = false;
	_term[0] = '\0';
}

int GPS::from_hex(char a) {
	if (a >= 'A' && a <= 'F')
		return a - 'A' + 10;
	else if (a >= 'a' && a <= 'f')
		return a - 'a' + 10;
	else
		return a - '0';
}

unsigned long GPS::parse_decimal() {
	char *p = _term;
	bool isneg = *p == '-';
	if (isneg)
		++p;
	unsigned long ret = 100UL * gpsatol(p);
	while (gpsisdigit(*p))
		++p;
	if (*p == '.') {
		if (gpsisdigit(p[1])) {
			ret += 10 * (p[1] - '0');
			if (gpsisdigit(p[2]))
				ret += p[2] - '0';
		}
	}
	return isneg ? -ret : ret;
}

// Parse a string in the form ddmm.mmmmmmm...
unsigned long GPS::parse_degrees() {
	char *p;
	unsigned long left_of_decimal = gpsatol(_term);
	unsigned long hundred1000ths_of_minute = (left_of_decimal % 100UL)
			* 100000UL;
	for (p = _term; gpsisdigit(*p); ++p)
		;
	if (*p == '.') {
		unsigned long mult = 10000;
		while (gpsisdigit(*++p)) {
			hundred1000ths_of_minute += mult * (*p - '0');
			mult /= 10;
		}
	}
	return (left_of_decimal / 100) * 1000000
			+ (hundred1000ths_of_minute + 3) / 6;
}

#define COMBINE(sentence_type, term_number) (((unsigned)(sentence_type) << 5) | term_number)

// Processes a just-completed term
// Returns true if new sentence has just passed checksum test and is validated
bool GPS::term_complete(){
  if (_is_checksum_term)
  {
    byte checksum = 16 * from_hex(_term[0]) + from_hex(_term[1]);
    if (checksum == _parity)
    {
      if (_gps_data_good)
      {

        _last_time_fix = _new_time_fix;
        _last_position_fix = _new_position_fix;

        switch(_sentence_type)
        {
        case _GPS_SENTENCE_GPRMC:
          _time      = _new_time;
          _date      = _new_date;
          _latitude  = _new_latitude;
          _longitude = _new_longitude;
          _speed     = _new_speed;
          _course    = _new_course;
          break;
        case _GPS_SENTENCE_GPGGA:
          _altitude  = _new_altitude;
          _time      = _new_time;
          _latitude  = _new_latitude;
          _longitude = _new_longitude;
          _numsats   = _new_numsats;
          _hdop      = _new_hdop;
          break;
        }

        return true;
      }
    }


    return false;
  }

  // the first term determines the sentence type
  if (_term_number == 0)
  {
    if (!gpsstrcmp(_term, _GPRMC_TERM))
      _sentence_type = _GPS_SENTENCE_GPRMC;
    else if (!gpsstrcmp(_term, _GPGGA_TERM))
      _sentence_type = _GPS_SENTENCE_GPGGA;
    else
      _sentence_type = _GPS_SENTENCE_OTHER;
    return false;
  }

  if (_sentence_type != _GPS_SENTENCE_OTHER && _term[0])
    switch(COMBINE(_sentence_type, _term_number))
  {
    case COMBINE(_GPS_SENTENCE_GPRMC, 1): // Time in both sentences
    case COMBINE(_GPS_SENTENCE_GPGGA, 1):
      _new_time = parse_decimal();
      _new_time_fix = millis();
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 2): // GPRMC validity
      _gps_data_good = _term[0] == 'A';
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 3): // Latitude
    case COMBINE(_GPS_SENTENCE_GPGGA, 2):
      _new_latitude = parse_degrees();
      _new_position_fix = millis();
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 4): // N/S
    case COMBINE(_GPS_SENTENCE_GPGGA, 3):
      if (_term[0] == 'S')
        _new_latitude = -_new_latitude;
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 5): // Longitude
    case COMBINE(_GPS_SENTENCE_GPGGA, 4):
      _new_longitude = parse_degrees();
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 6): // E/W
    case COMBINE(_GPS_SENTENCE_GPGGA, 5):
      if (_term[0] == 'W')
        _new_longitude = -_new_longitude;
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 7): // Speed (GPRMC)
      _new_speed = parse_decimal();
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 8): // Course (GPRMC)
      _new_course = parse_decimal();
      break;
    case COMBINE(_GPS_SENTENCE_GPRMC, 9): // Date (GPRMC)
      _new_date = gpsatol(_term);
      break;
    case COMBINE(_GPS_SENTENCE_GPGGA, 6): // Fix data (GPGGA)
      _gps_data_good = _term[0] > '0';
      break;
    case COMBINE(_GPS_SENTENCE_GPGGA, 7): // Satellites used (GPGGA)
      _new_numsats = (unsigned char)atoi(_term);
      break;
    case COMBINE(_GPS_SENTENCE_GPGGA, 8): // HDOP
      _new_hdop = parse_decimal();
      break;
    case COMBINE(_GPS_SENTENCE_GPGGA, 9): // Altitude (GPGGA)
      _new_altitude = parse_decimal();
      break;
  }

  return false;
}

long GPS::gpsatol(const char *str) {
	long ret = 0;
	while (gpsisdigit(*str))
		ret = 10 * ret + *str++ - '0';
	return ret;
}

int GPS::gpsstrcmp(const char *str1, const char *str2) {
	while (*str1 && *str1 == *str2)
		++str1, ++str2;
	return *str1;
}

/* static */
float GPS::distance_between(float lat1, float long1, float lat2, float long2) {
	// returns distance in meters between two positions, both specified
	// as signed decimal-degrees latitude and longitude. Uses great-circle
	// distance computation for hypothetical sphere of radius 6372795 meters.
	// Because Earth is no exact sphere, rounding errors may be up to 0.5%.
	// Courtesy of Maarten Lamers
	float delta = radians(long1-long2);
	float sdlong = sin(delta);
	float cdlong = cos(delta);
	lat1 = radians(lat1);
	lat2 = radians(lat2);
	float slat1 = sin(lat1);
	float clat1 = cos(lat1);
	float slat2 = sin(lat2);
	float clat2 = cos(lat2);
	delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
	delta = sq(delta);
	delta += sq(clat2 * sdlong);
	delta = sqrt(delta);
	float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
	delta = atan2(delta, denom);
	return delta * 6372795;
}

float GPS::course_to(float lat1, float long1, float lat2, float long2) {
	// returns course in degrees (North=0, West=270) from position 1 to position 2,
	// both specified as signed decimal-degrees latitude and longitude.
	// Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
	// Courtesy of Maarten Lamers
	float dlon = radians(long2-long1);
	lat1 = radians(lat1);
	lat2 = radians(lat2);
	float a1 = sin(dlon) * cos(lat2);
	float a2 = sin(lat1) * cos(lat2) * cos(dlon);
	a2 = cos(lat1) * sin(lat2) - a2;
	a2 = atan2(a1, a2);
	if (a2 < 0.0) {
		a2 += TWO_PI;
	}
	return degrees(a2);
}

// lat/long in MILLIONTHs of a degree and age of fix in milliseconds
// (note: versions 12 and earlier gave this value in 100,000ths of a degree.
void GPS::get_position(float *latitude, float *longitude,
		unsigned long *fix_age) {
	if (latitude)
		*latitude = ((float) _latitude) * 0.000001;
	if (longitude)
		*longitude = ((float) _longitude) * 0.000001;
	if (fix_age)
		*fix_age =
				_last_position_fix == GPS_INVALID_FIX_TIME ?
						GPS_INVALID_AGE : millis() - _last_position_fix;
}

// date as ddmmyy, time as hhmmsscc, and age in milliseconds
void GPS::get_datetime(unsigned long *date, unsigned long *time,
		unsigned long *age) {
	if (date)
		*date = _date;
	if (time)
		*time = _time;
	if (age)
		*age = _last_time_fix == GPS_INVALID_FIX_TIME ?
				GPS_INVALID_AGE : millis() - _last_time_fix;
}

void GPS::f_get_position(float *latitude, float *longitude,
		unsigned long *fix_age) {
	long lat, lon;
	get_position((float*) &lat, (float*) &lon, (unsigned long*) fix_age);
	*latitude =
			lat == GPS_INVALID_ANGLE ? GPS_INVALID_F_ANGLE : (lat / 1000000.0);
	*longitude =
			lat == GPS_INVALID_ANGLE ? GPS_INVALID_F_ANGLE : (lon / 1000000.0);
}

void GPS::crack_datetime(int *year, byte *month, byte *day,
byte *hour, byte *minute, byte *second, byte *hundredths, unsigned long *age) {
	unsigned long date, time;
	get_datetime(&date, &time, age);
	if (year) {
		*year = date % 100;
		*year += *year > 80 ? 1900 : 2000;
	}
	if (month)
		*month = (date / 100) % 100;
	if (day)
		*day = date / 10000;
	if (hour)
		*hour = time / 1000000;
	if (minute)
		*minute = (time / 10000) % 100;
	if (second)
		*second = (time / 100) % 100;
	if (hundredths)
		*hundredths = time % 100;
}

float GPS::f_altitude() {
	return _altitude == GPS_INVALID_ALTITUDE ?
			GPS_INVALID_F_ALTITUDE : _altitude / 100.0;
}

float GPS::f_course() {
	return _course == GPS_INVALID_ANGLE ? GPS_INVALID_F_ANGLE : _course / 100.0;
}

float GPS::f_speed_knots() {
	return _speed == GPS_INVALID_SPEED ? GPS_INVALID_F_SPEED : _speed / 100.0;
}

float GPS::f_speed_mph() {
	float sk = f_speed_knots();
	return sk == GPS_INVALID_F_SPEED ?
			GPS_INVALID_F_SPEED : _GPS_MPH_PER_KNOT * sk;
}

float GPS::f_speed_mps() {
	float sk = f_speed_knots();
	return sk == GPS_INVALID_F_SPEED ?
			GPS_INVALID_F_SPEED : _GPS_MPS_PER_KNOT * sk;
}

float GPS::f_speed_kmph() {
	float sk = f_speed_knots();
	return sk == GPS_INVALID_F_SPEED ?
			GPS_INVALID_F_SPEED : _GPS_KMPH_PER_KNOT * sk;
}

long GPS::altitude() {
	return _altitude;
}

unsigned long GPS::course() {
	return _course;
}

unsigned long GPS::speed() {
	return _speed;
}

unsigned short GPS::satellites() {
	return _numsats;
}

unsigned long GPS::hdop() {
	return _hdop;
}

