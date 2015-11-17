#ifndef __CALCULATE_H_
#define __CALCULATE_H_

#include <stdio.h>
#include <stdlib.h>
#include <ev.h>

#include "decode_gps.h"
#include "rr_cfg.h"
#include "match_road.h"
/*

typedef struct kv_imei{
	int count;
	int max_speed_num;
	int max_speed;
	long str_time;
	long end_time;
	long roadID;
}KV_IMEI;

typedef struct kv_roadID{
        long IMEI;
        int max_speed;
        int avg_speed;
        long end_time;  
        long used_time;
}KV_ROADID;

*/
int calculate(GPS_INFO *gps_info, ROAD_INFO *road_info, struct ev_loop *loop);

#endif
		

