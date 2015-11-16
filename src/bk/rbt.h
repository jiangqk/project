/*
 *  rbt.h
 *
 *  Created on: Jun 20, 2014
 *      Author: buyuanyuan
 *      Fixed By: chenzutao
 */

#ifndef RBT_H_
#define RBT_H_

#include "relation.h"

typedef enum Color {
	RED = 0,
        BLACK = 1
} Color;

typedef struct rbt_road {
	long roadID;
	Color color;
	ROAD_INFO *road;
	PERSON *person;
	SPEED *speed;
	struct rbt_road *left, *right, *parent;
} RBT_ROAD;

typedef struct rbt_imei {
	long long IMEI;
	Color color;
	int speed;
	long time;
	ROAD_INFO *road;
	struct rbt_imei *left, *right, *parent;
} RBT_IMEI;

//rbt_road.c
int insert_road_info(RBT_ROAD **root, ROAD_INFO *road, int speed, long time, long long IMEI, int point_speed[], int point_cnt);

SPEED *get_speed_info(RBT_ROAD *root, long roadID);

PERSON *get_road_person(RBT_ROAD *root, long roadID);

char *get_redis_speed_info_value(RBT_ROAD *root, long roadID);

char *get_redis_active_user_value(RBT_ROAD *root, long roadID);

int road_delete_imei(RBT_ROAD *root, long roadID, long long IMEI);

int road_delete(RBT_ROAD **root);

//rbt_imei.c
int imei_insert(RBT_IMEI **root, ROAD_INFO *road, long long IMEI, int speed, long time);

ROAD_INFO *get_imei_road(RBT_IMEI *root, long long IMEI);

char *get_redis_road_info_value(RBT_IMEI *root, long long IMEI);

int imei_destory(RBT_IMEI **root);

#endif /* RBT_H_ */
