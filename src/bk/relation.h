#ifndef __RELATION_H_
#define __RELATION_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#define HIGHWAY 0
#define FASTWAY 10
#define HIGHWAY_CONGESTION_SPEED 10
#define HIGHWAY_SLOWLY_SPEED 25
#define MAX_ROAD_NAME           64*2
#define MAX_CROSS_SECTION       128*2
#define MAX_CROSS_BEGIN         128
#define MAX_CROSS_END           128
#define MAX_GRID_ID             128
#define MAX_NEXT_SEGMENT_ID     32
#define NORMAL_WAY_CONGESTION_SPEED 5
#define NORMAL_WAY_SLOWLY_SPEED 20
#define ROAD_LEVEL(_level) ( ( ((_level) == HIGHWAY ) || ((_level) == FASTWAY) ) ? 1 : 0 )
#define CONGESTION_SPEED(_level, _speed) ( ( ROAD_LEVEL(_level) && ((_speed) <= HIGHWAY_CONGESTION_SPEED)) ? (HIGHWAY_CONGESTION_SPEED) : (NORMAL_WAY_CONGESTION_SPEED) )
#define SLOWLY_SPEED(_level, _speed)  ( ( (!ROAD_LEVEL(_level)) && ((_speed) <= HIGHWAY_SLOWLY_SPEED) && ((_speed) >= HIGHWAY_CONGESTION_SPEED) ) ? (HIGHWAY_SLOWLY_SPEED) : (NORMAL_WAY_SLOWLY_SPEED) )


typedef struct point {
        int total_points;
        int congestion_points;
        int slowly_points;
        int congestion_trough;
        int slowly_trough;
        bool congestion_flag;
        bool slowly_flag;
}POINT;

typedef struct person {
        int cnt;
        int max_speed;
        long sum_speed;
        long time;
        long long IMEI;
        POINT points;
        struct person *next;
} PERSON;

typedef struct speed {
        int max_speed;
        int average_speed;
        int last_speed;
        long time;
        POINT points;
} SPEED;


int person_insert(PERSON **person, int speed, long time, long long IMEI, int level, 
		int point_speed[], int point_cnt);

char *get_person_imei(PERSON *person);

int person_delete(PERSON **person, long long IMEI);

int person_speed(PERSON *person, SPEED **speed);

char *get_road_speed(SPEED *speed, ROAD_INFO *road);

int person_destory(PERSON *person);

int traffic_insert(ROAD_INFO **road, ROAD_INFO *road_info, SPEED *speed);

int road_insert(ROAD_INFO **road, ROAD_INFO *road_info);

char *get_road_id(ROAD_INFO *road);

int road_destory(ROAD_INFO *road);

#endif
