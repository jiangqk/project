#ifndef __DECODE_GPS_H_
#define __DECODE_GPS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "utils.h"

typedef struct gps_info{
//	int point_speed[5];
	int point_cnt;
	int max_speed;
	int direction;
	long start_time;
	long end_time;
	double longitude;
	double latitude;
	long long IMEI;	
}GPS_INFO;


int decode_gps(const char *data, GPS_INFO *gps_info);

#endif

