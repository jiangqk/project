#ifndef __MATCH_ROAD_H_
#define __MATCH_ROAD_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <match.h>


#include "utils.h"
#include "pool_api.h"
#include "async_api.h"
#include "decode_gps.h"
//#include "kv_imei.h"
typedef struct road_info {
	long road_rootID;
	int segmentID;
//	int count;
//	long long roadID;
//	long length;
//	int cross_beginlevel;
//	int cross_endlevel;
//	 int county_code;
//	int direction; 
//	char road_name[MAX_ROAD_NAME]; 
//	char cross_begin[MAX_CROSS_BEGIN];
//	char cross_end[MAX_CROSS_END];
//	char gridID[MAX_GRID_ID]; 
//	char next_segmentID[MAX_NEXT_SEGMENT_ID]; 
//	int road_level; 
//	double start_lon; 
//	double start_lat;
//	 double end_lon; 
//	double end_lat;  
} ROAD_INFO;

int match_road(struct ev_loop *loop, GPS_INFO *gps_info);
int get_roadId(char *http_data,ROAD_INFO **road_info,
		 int direction);

#endif


