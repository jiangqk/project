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
#include "json.h"
#include "relation.h"

int match_road(struct ev_loop *loop, GPS_INFO *gps_info);
int get_roadId(char *http_data,ROAD_INFO **road_info,
		 int direction);

#endif


