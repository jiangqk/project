#include "decode_gps.h"


int decode_gps(const char *data, GPS_INFO *gps_info)
{
	int i;
	int speed_max_idx = 0;
	int speed_max_val = -1;
	cJSON *arr = NULL;
	cJSON *son = NULL;
	cJSON *obj = cJSON_Parse(data);


	if (NULL == obj) {
		x_printf(E, "Failed to create cJSON object!");
		return -1;
	}

	/*parse speed*/
	son = cJSON_GetObjectItem(obj, "speed");
	if (NULL == son){
		x_printf(E, "data has no speed!");
		goto fail;
	}
	int all = cJSON_GetArraySize(son);
	if (all <= 0 || all > 15) {
		x_printf(E, "Failed to decode GPS, the array number of speed is not right!\n");
		goto fail;
	}
	all = MIN(all, 5);
	gps_info->point_cnt = all;
	arr = cJSON_GetArrayItem(son, 0);
	gps_info->max_speed = arr->valueint;
	for (i = 0; i < all; i++){
		arr = cJSON_GetArrayItem(son, i);
		int speed = arr->valueint;
		
		if (speed > gps_info->max_speed){
			speed_max_idx = i;	
			gps_info->max_speed = speed;
		}
		x_printf(D, "\t%d\n", speed);
	}

	/*parse direction*/
	son = cJSON_GetObjectItem(obj, "direction");
	if (NULL == son){
		x_printf(E, "data has no direction!");
		goto fail;
	}
	arr = cJSON_GetArrayItem(son, speed_max_idx);
	gps_info->direction = arr->valuedouble;

	/*parse longitude*/
	son = cJSON_GetObjectItem(obj, "longitude");
	if (NULL == son){
		x_printf(E, "data has no longitude!");
		goto fail;
	}
	arr = cJSON_GetArrayItem(son, speed_max_idx);
	gps_info->longitude = arr->valuedouble;

	/*parse latitude*/
	son = cJSON_GetObjectItem(obj, "latitude");
	if (NULL == son){
		x_printf(E, "data has no latitude!");
		goto fail;
	}
	arr = cJSON_GetArrayItem(son, speed_max_idx);
	gps_info->latitude = arr->valuedouble;

	/*parse GPSTime*/
	son = cJSON_GetObjectItem(obj, "GPSTime");
	if (NULL == son){
		x_printf(E, "data has no GPSTime!");
		goto fail;
	}
	arr = cJSON_GetArrayItem(son, gps_info->point_cnt - 1);
	gps_info->start_time = arr->valueint;
	
	arr = cJSON_GetArrayItem(son, 0);
	gps_info->end_time = arr->valueint;


	/*parse IMEI*/
	son = cJSON_GetObjectItem(obj, "IMEI");
	if (NULL == son){
		x_printf(E, "data has no IMEI!");
		goto fail;
	}
	gps_info->IMEI = atoll(son->valuestring);


	cJSON_Delete(obj);

        x_printf(D, "<=====================================================================>");
        x_printf(D, "direction:%d, longitude:%lf, latitude:%lf, start_time:%ld, end_time:%ld, IMEI:%ld",
			gps_info->direction, gps_info->longitude, gps_info->latitude,
			 gps_info->start_time, gps_info->end_time, gps_info->IMEI);
	return 0;


fail:	
	cJSON_Delete(obj);
	return -1;
}
