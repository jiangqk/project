#ifndef __KV_ROAD_H_
#define __KV_ROAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "libkv.h"
#include "decode_gps.h"
#include "calculate.h"

extern KV_ROADID *redis_roadID;
extern KV_IMEI *redis_IMEI;

int set_roadID_to_kv(KV_IMEI *kv_IMEI, KV_ROADID *kv_roadID, GPS_INFO *gps_info, long roadID);
#endif
