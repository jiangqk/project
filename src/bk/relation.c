/**
 * Author       : chenzutao
 * date         : 2014-11-18
 * function     : data structure to store all people's average and max speed on one road
 **/

#include "relation.h"

int person_insert(PERSON **person, int speed, long time, long long IMEI, int level, int point_speed[], int point_cnt)
{
        if(NULL == *person)
        {
                *person = (PERSON *)calloc(sizeof(PERSON), 1);
                if(*person == NULL) {
                        x_printf(E,  "person person malloc error !\n");
                        return -1;
                }
                (*person)->IMEI = IMEI;
                (*person)->cnt = 1;
                (*person)->max_speed = speed;
                (*person)->sum_speed = speed;
                (*person)->time = time;
                (*person)->next = NULL;

                (*person)->points.total_points = point_cnt;
                (*person)->points.congestion_points = 0;
                (*person)->points.slowly_points = 0;
                (*person)->points.congestion_trough = 0;
                (*person)->points.slowly_trough = 0;
                int i = 0;
                for(i=0; i<point_cnt; i++)
                {
                        if(point_speed[i] <= CONGESTION_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.congestion_points++;
                                if( !(*person)->points.congestion_flag )
                                {
                                        (*person)->points.congestion_flag = true;
                                }
                        }
                        else if(point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.slowly_points++;
                                if( !(*person)->points.slowly_flag)
                                {
                                        (*person)->points.slowly_flag = true;
                                }
                        }

                        if( (*person)->points.congestion_flag && point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.congestion_flag = false;
                                (*person)->points.congestion_trough++;
                        }

                        if( (*person)->points.slowly_flag && point_speed[i] > SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.slowly_flag = false;
                                (*person)->points.slowly_trough++;
                        }
                }
                return 0;
        }

        PERSON *p = *person;

        while(p->next != NULL && p->IMEI != IMEI)
        {
                p = p->next;
        }

        if(p->IMEI != IMEI) {
                PERSON *q = (PERSON *)calloc(sizeof(PERSON), 1);
                if(q == NULL) {
                        x_printf(E,  "person q malloc error !\n");
                        return -1;
                }
                q->IMEI = IMEI;
                q->cnt = 1;
                q->max_speed = speed;
                q->sum_speed = speed;
                q->time = time;
                q->next = NULL;
                p->next = q;

                (*person)->points.total_points = point_cnt;
                (*person)->points.congestion_points = 0;
                (*person)->points.slowly_points = 0;
                (*person)->points.congestion_trough = 0;
                (*person)->points.slowly_trough = 0;
                int i = 0;
                for(i=0; i<point_cnt; i++)
                {
                        if(point_speed[i] <= CONGESTION_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.congestion_points++;
                                if( !(*person)->points.congestion_flag )
                                {
                                        (*person)->points.congestion_flag = true;
                                }
                        }
                        else if(point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.slowly_points++;
                                if( !(*person)->points.slowly_flag)
                                {
                                        (*person)->points.slowly_flag = true;
                                }
                        }

                        if( (*person)->points.congestion_flag && point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.congestion_flag = false;
                                (*person)->points.congestion_trough++;
                        }

                        if( (*person)->points.slowly_flag && point_speed[i] > SLOWLY_SPEED(level, point_speed[i]))
                        {
                                (*person)->points.slowly_flag = false;
                                (*person)->points.slowly_trough++;
                        }
                }

                return 0;
        }

        p->cnt ++;
        if(p->max_speed < speed) {
                p->max_speed = speed;
        }
        p->sum_speed += speed;
        p->time = time;

        int i = 0;
        for(i=0; i<point_cnt; i++)
        {
                if(point_speed[i] <= CONGESTION_SPEED(level, point_speed[i]))
                {
                        (*person)->points.congestion_points++;
                        if( !(*person)->points.congestion_flag )
                        {
                                (*person)->points.congestion_flag = true;
                        }
                }
                else if(point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                {
                        (*person)->points.slowly_points++;
                        if( !(*person)->points.slowly_flag)
                        {
                                (*person)->points.slowly_flag = true;
                        }
                }

                if( (*person)->points.congestion_flag && point_speed[i] > CONGESTION_SPEED(level, point_speed[i]) && point_speed[i] <= SLOWLY_SPEED(level, point_speed[i]))
                {
                        (*person)->points.congestion_flag = false;
                        (*person)->points.congestion_trough++;
                }

                if( (*person)->points.slowly_flag && point_speed[i] > SLOWLY_SPEED(level, point_speed[i]))
                {
                        (*person)->points.slowly_flag = false;
                        (*person)->points.slowly_trough++;
                }
        }

        return 0;
}

char *get_person_imei(PERSON *person)
{
	if(NULL == person) {
		return NULL;
	}

	PERSON *p = person;
	char *ret = NULL, *res2;
	char res[16];

	while(p != NULL) {
		sprintf(res, "%lld", p->IMEI);
		if(ret == NULL) {
			ret = (char *)calloc(strlen(res) + 1, 1);
			if(ret == NULL) {
                	        x_printf(E,  "get redis active user value malloc error !\n");
        	                return NULL;
	                }
			strcpy(ret, res);
		} else {
			res2 = ret;
			ret = (char *)malloc(strlen(res) + strlen(res2) + 2);
			if(ret == NULL) {
                                x_printf(E,  "get redis active user value malloc error !\n");
                                return NULL;
                        }
			sprintf(ret, "%s:%s", res2, res);
			free(res2), res2 = NULL;
		}
		p = p->next;
	}

	return ret;
}

int person_delete(PERSON **person, long long IMEI)
{
        if(NULL == *person)
        {
                return 0;
        }

        PERSON *p = *person, *s = NULL;

	if(p->IMEI == IMEI) {
		s = p;
		*person = p->next;
		free(s), s = NULL;
		return 0;
	}

        while(p->next != NULL)
        {
                if(p->next->IMEI == IMEI)
                {
                        s = p->next;
                        p->next = s->next;
                        free(s), s = NULL;
                        return 0;
                }
                p = p->next;
        }
	return 0;
}

int person_speed(PERSON *person, SPEED **speed)
{
	if(NULL == person)
        {
                return 0;
        }

	int max_speed = -1, count = 0, last_speed = -1;
	long sum_speed = 0, time = 0;

        int total_points = 0;
        int congestion_points = 0, slowly_points = 0;
        int congestion_trough = 0, slowly_trough = 0;

	PERSON *p = person;

	if(*speed == NULL) {
		*speed = (SPEED *)calloc(sizeof(SPEED), 1);
		if(*speed == NULL) {
			x_printf(E,  "speed malloc error !\n");
                        return -1;
                }
                (*speed)->last_speed = last_speed;
	} else {
                (*speed)->last_speed = (*speed)->average_speed;
        }

	while(p != NULL) {
		sum_speed += p->sum_speed;
		if(max_speed < p->max_speed) {
			max_speed = p->max_speed;
		}
		count += p->cnt;
		if(time < p->time) {
			time = p->time;
		}

                total_points += p->points.total_points;
                congestion_points += p->points.congestion_points;
                slowly_points += p->points.slowly_points;
                congestion_trough += p->points.congestion_trough;
                slowly_trough += p->points.slowly_trough;

		p = p->next;
	}

	(*speed)->max_speed = max_speed;
	(*speed)->time = time;
	(*speed)->average_speed = sum_speed / count;

        (*speed)->points.total_points = total_points;
        (*speed)->points.congestion_points = congestion_points;
        (*speed)->points.slowly_points = slowly_points;
        (*speed)->points.congestion_trough = congestion_trough;
        (*speed)->points.slowly_trough = slowly_trough;

	return 0;
}

char *get_road_speed(SPEED *speed, ROAD_INFO *road)
{
	if(NULL == speed) {
		return NULL;
	}
        if(road == NULL)
        {
                return NULL;
        }

	char res[2048] = { 0 };
	char *ret = NULL;
        char type[4] = { 0 };
        int diff = speed->average_speed - speed->last_speed;

        if(diff > 20) {
                strcpy(type, "++");
        } else if(diff > 10 && diff <= 20) {
                strcpy(type, "+");
        } else if(diff >= -20 && diff < -10) {
                strcpy(type, "-");
        } else if(diff < -20) {
                strcpy(type, "--");
        } else {
                strcpy(type, "~");
        }
	x_printf(I,  "roadID:%ld, roadName:%s, max_speed:%d, avg_speed:%d, time:%ld, length:%ld, direction:%d, type:%s, crossSection:%s, speedLimit:%d, roadLevel:%d, L1:%.7lf, B1:%.7lf, L2:%.7lf, B2:%.7lf, trafficID:%lld, slowly_trough:%d, congestion_trough:%d\n",
                        road->roadID, road->road_name, speed->max_speed, speed->average_speed,
                        speed->time, road->length, road->direction, type, road->cross_section,
                        road->speed_limit, road->road_level, road->start_lon, road->start_lat,
                        road->end_lon, road->end_lat, road->trafficID, speed->points.slowly_trough, speed->points.congestion_trough);

	sprintf(res, "%ld:%s:%d:%d:%ld:%ld:%d:%s:%s:%d:%d:%.7lf:%.7lf:%.7lf:%.7lf:%lld:%d:%d",
                        road->roadID, road->road_name, speed->max_speed, speed->average_speed,
                        speed->time, road->length, road->direction, type, road->cross_section,
                        road->speed_limit, road->road_level, road->start_lon, road->start_lat,
                        road->end_lon, road->end_lat, road->trafficID, speed->points.slowly_trough, speed->points.congestion_trough);

	ret = (char *)calloc(strlen(res) + 1, 1);
	if(ret == NULL) {
		x_printf(E,  "get redis speed info value malloc error !\n");
		return NULL;
	}
	strcpy(ret, res);

	return ret;
}

int person_destory(PERSON *person)
{
	if(NULL == person)
	{
		return 0;
	}

	PERSON *p = person, *s = NULL;
	while( p != NULL)
	{
		s = p;
		p = p->next;
		free(s), s = NULL;
	}
	return 0;
}

int traffic_insert(ROAD_INFO **road, ROAD_INFO *road_info, SPEED *speed)
{
    if(*road == NULL || road_info == NULL || speed == NULL)
    {
        x_printf(E,  "pointer parameter is NULL !\n");
        return -1;
    }

    if((*road)->trafficID == -1)
    {
        (*road)->trafficID = ((road_info->city_code * 100000000) + road_info->roadID ) * 10000 + 1;
    }
    else
    {
        if(speed->last_speed != speed->average_speed)
        {
            (*road)->trafficID = road_info->trafficID + 1;
        }
    }
    return 0;
}

int road_insert(ROAD_INFO **road, ROAD_INFO *road_info)
{
	if(NULL == *road) {
		*road = (ROAD_INFO *)malloc(sizeof(ROAD_INFO));
		if(*road == NULL) {
			x_printf(E,  "malloc road info error\n");
			return -1;
		}
		(*road)->roadID = road_info->roadID;
	        (*road)->length = road_info->length;
        	strcpy((*road)->road_name, road_info->road_name);
                /*
                if(road_info->road_name != NULL)
                {
	                (*road)->road_name = (char*)malloc(strlen(road_info->road_name) + 1);
			if((*road)->road_name == NULL) {
				x_printf(E,  "malloc road_info road name error!\n");
				return -1;
			}
        	        strcpy((*road)->road_name, road_info->road_name);
                }
                */

                strcpy( (*road)->cross_section, road_info->cross_section );
                /* //XXX revised on : 2015-07-04 ==>changed cross_section from pointer to array
                if(road_info->cross_section != NULL)
                {
                        (*road)->cross_section = (char *)calloc(sizeof(char), strlen(road_info->cross_section) + 1);
                        if( (*road)->cross_section == NULL)
                        {
                                x_printf(E,  "malloc road_info cross section failed !\n");
                                return -1;
                        }
                        strcpy( (*road)->cross_section, road_info->cross_section );
                }
                */

		(*road)->speed_limit = road_info->speed_limit;

        	(*road)->city_code = road_info->city_code;
                (*road)->county_code = road_info->county_code;
	        (*road)->direction = road_info->direction;
                (*road)->road_level = road_info->road_level;
                (*road)->start_lon = road_info->start_lon;
                (*road)->start_lat = road_info->start_lat;
                (*road)->end_lon = road_info->end_lon;
                (*road)->end_lat = road_info->end_lat;
                (*road)->trafficID = -1;
                return 0;
	}

	(*road)->roadID = road_info->roadID;
	(*road)->length = road_info->length;
       
       //XXX revised on : 2015-07-04 
        memset((*road)->road_name, 0, sizeof((*road)->road_name));
        memset((*road)->cross_section, 0, sizeof((*road)->cross_section));
        strcpy((*road)->road_name, road_info->road_name);
        strcpy( (*road)->cross_section, road_info->cross_section );
        /*
        if(road_info->road_name != NULL)
        {
	        free((*road)->road_name);
		(*road)->road_name = NULL;
	        (*road)->road_name = (char*)calloc(1, strlen(road_info->road_name) + 1);
		if((*road)->road_name == NULL) {
			x_printf(E,  "malloc road_info road name error!\n");
			return -1;
		}
	        strcpy((*road)->road_name, road_info->road_name);
        }

        if(road_info->cross_section != NULL)
        {
                free( (*road)->cross_section );
                (*road)->cross_section = NULL;
                (*road)->cross_section = (char *)calloc(sizeof(char), strlen(road_info->cross_section) + 1);
                if( (*road)->cross_section == NULL)
                {
                        x_printf(E,  "malloc road_info cross section failed !\n");
                        return -1;
                }
                strcpy( (*road)->cross_section, road_info->cross_section );
        }
        */

	(*road)->speed_limit = road_info->speed_limit;
        (*road)->city_code = road_info->city_code;
        (*road)->county_code = road_info->county_code;
        (*road)->direction = road_info->direction;
        (*road)->road_level = road_info->road_level;
        (*road)->start_lon = road_info->start_lon;
        (*road)->start_lat = road_info->start_lat;
        (*road)->end_lon = road_info->end_lon;
        (*road)->end_lat = road_info->end_lat;
        return 0;
}

char *get_road_id(ROAD_INFO *road)
{
	if(NULL == road) {
		return NULL;
	}
	
	char *ret = NULL;
	int len = sizeof(road->roadID) + sizeof(road->road_name) + sizeof(road->city_code);

	ret = (char *)calloc(len + 8, sizeof(char));
	if(ret == NULL) {
		x_printf(E,  "get redis road info value malloc error !\n");
		return NULL;
	}
        sprintf(ret, "%ld:%s:%ld", road->roadID, road->road_name, road->city_code);
        return ret;


#if 0
	char res[128] = { 0 };
        char *ret = NULL;

        sprintf(res, "%ld:%s:%ld", road->roadID, road->road_name, road->city_code);

        ret = (char *)malloc(strlen(res) + 1);
	if(ret == NULL) {
		x_printf(E,  "get redis road info value malloc error !\n");
		return NULL;
	}
        sprintf(ret, "%s", res);
        return ret;
#endif
}

int road_destory(ROAD_INFO *road)
{
	if(NULL == road) {
		return 0;
	}
        memset(road->road_name, 0, sizeof(road->road_name));
        memset(road->cross_section, 0, sizeof(road->cross_section));
        /*
	if(road->road_name != NULL) {
		free(road->road_name);
		road->road_name = NULL;
	}
        if(road->cross_section != NULL)
        {
                free(road->cross_section);
                road->cross_section = NULL;
        }
        */
/*
        if(road->speedLimit != NULL)
        {
                free(road->speedLimit);
                road->speedLimit = NULL;
        }
*/
	free(road), road = NULL;

        return 0;
}
/*
int main()
{
	PERSON *person = NULL;
	SPEED *speed = NULL;
	char *s;
	person_insert(&person, 1, 1, 1);
	person_insert(&person, 2, 1, 1);
	person_insert(&person, 3, 1, 2);
	s = person_imei(person);
	person_speed(person, &speed);
	person_insert(&person, 5, 1, 2);
	person_insert(&person, 7, 1, 1);
	person_speed(person, &speed);
	person_delete(&person, 2);
	person_speed(person, &speed);
	person_delete(&person, 1);

	person_destory(person);
	free(speed);
	free(s);
	return 0;
}
*/
