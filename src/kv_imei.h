#ifndef __KV_IMEI_H_
#define __KV_IMEI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "libkv.h"
#include "decode_gps.h"
#include "calculate.h"

#define MAX_ROAD_NAME           64*2
#define MAX_CROSS_BEGIN         128
#define MAX_CROSS_END           128
#define MAX_GRID_ID             128
#define MAX_NEXT_SEGMENT_ID     32


//int get_IMEI_road(long IMEI, long roadID, REDIS_IMEI *redis_IMEI, GPS_INFO *gps_info);

int get_IMEI_from_kv(long IMEI, KV_IMEI *kv_IMEI);
//int get_redis_IMEI(long IMEI, REDIS_IMEI *redis_IMEI);

int set_IMEI_to_kv(GPS_INFO *gps_info, long roadID, KV_IMEI *kv_IMEI, int flag);
//int set_redis_IMEI(GPS_INFO *gps_info, long roadID, REDIS_IMEI *redis_IMEI, int flag);

//int same_road(GPS_INFO *gps_info, long roadID, REDIS_IMEI *redis_IMEI, REDIS_RAODID *redis_roadID);

//int change_road(GPS_INFO *gps_info, long roadID, REDIS_IMEI *redis_IMEI, REDIS_ROADID *redis_roadID);

#endif
