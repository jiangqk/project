#include "calculate.h"
#include "rr_cfg.h"
#include "async_api.h"
#include "utils.h"
#include "kv_road.h"
#include "kv_imei.h"


#define ASYNC_LIBEV_THRESHOLD 5
#define BUFF_USE_LEN		10240


static __thread char g_buff[ BUFF_USE_LEN ] = {0};
extern struct rr_cfg_file g_rr_cfg_file;


static int update_to_redis(KV_ROADID *kv_roadID, long old_roadID, struct ev_loop * loop)
{
	struct rr_link *link = &g_rr_cfg_file.links[0];
	/*1*/
	char *proto_1 = NULL;
	sprintf(g_buff, 
		"set %ld:roadSpeedInfo %d@%d", old_roadID, kv_roadID->max_speed, kv_roadID->avg_speed);
	x_printf(D, "redis command: %s", g_buff);

		cmd_to_proto(&proto_1, g_buff);
		memset(g_buff, 0, BUFF_USE_LEN);

	/*2*/
	char *proto_2 = NULL;
		sprintf(g_buff, "set %ld:speedInfo %d", kv_roadID->IMEI, kv_roadID->max_speed);
		x_printf(D, "redis command: %s", g_buff);

		cmd_to_proto(&proto_2, g_buff);
		memset(g_buff, 0, BUFF_USE_LEN);

	/*3*/
	char *proto_3 = NULL;
		sprintf(g_buff, "set %ld:timeInfo %ld@%ld", old_roadID, kv_roadID->end_time, 
					kv_roadID->used_time);
		x_printf(D, "redis command: %s", g_buff);

		cmd_to_proto(&proto_3, g_buff);
		memset(g_buff, 0, BUFF_USE_LEN);


	struct async_ctx *ac = NULL;
	struct cnt_pool *cpool = NULL;
	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_THRESHOLD);
	if (ac){
		void *sfd = (void*)(intptr_t)-1;
		int rc = pool_api_gain(&cpool, link->host, link->port, &sfd);
		if (rc){
			async_distory(ac);
		} else {
			if (proto_1){
				async_command(ac, PROTO_TYPE_REDIS, (void*)(intptr_t)sfd, cpool, NULL, NULL, proto_1, strlen(proto_1));
			}
			if (proto_2){
				async_command(ac, PROTO_TYPE_REDIS, (void*)(intptr_t)sfd, cpool, NULL, NULL, proto_2, strlen(proto_2));
			}
			if (proto_3){
				async_command(ac, PROTO_TYPE_REDIS, (void*)(intptr_t)sfd, cpool, NULL, NULL, proto_3, strlen(proto_3));
			}
			async_startup(ac);
		}
	}
	if (proto_1) {
		free(proto_1);
	}
	if (proto_2) {
		free(proto_2);
	}
	if (proto_3) {
		free(proto_3);
	}
	return 0;
}

int get_IMEI_road(long IMEI, long roadID, KV_IMEI *kv_IMEI, GPS_INFO *gps_info)
{
#if 0
        if (get_redis_IMEI(IMEI, redis_IMEI) == 0){  //IMEI is in the same road
                if (roadID == redis_IMEI->roadID)
                        return 1;
        }else if(get_redis_IMEI(IMEI, redis_IMEI) == 1){   //IMEI is a new
                set_redis_IMEI(gps_info, roadID, redis_IMEI, 1);
                        return 2;
        }else if (get_redis_IMEI(IMEI, redis_IMEI))
#endif

	int flag = -1;
	flag = get_IMEI_from_kv(IMEI, kv_IMEI);
	switch (flag){
		case -1: return -1;    //kv error
		case 0 :
			 if (roadID == kv_IMEI->roadID)
				return 1;
			 break;//same road       			 
		case 1:  return 2;     //first IMEI
		default: break;
	}

        return 0;   // IMEI has changed road
}

static int same_road(GPS_INFO *gps_info, long roadID, KV_IMEI *kv_IMEI)
{
        kv_IMEI->count += 1;
        kv_IMEI->max_speed_num += gps_info->max_speed;
        kv_IMEI->max_speed = (gps_info->max_speed > kv_IMEI->max_speed)?gps_info->max_speed:kv_IMEI->max_speed;
        kv_IMEI->end_time = gps_info->end_time;
        set_IMEI_to_kv(gps_info, roadID, kv_IMEI, 2);

        return 0;
}


static long change_road(GPS_INFO *gps_info, long roadID, KV_IMEI *kv_IMEI, KV_ROADID *kv_roadID)
{
	
	if (set_roadID_to_kv(kv_IMEI, kv_roadID, gps_info, roadID) == -1){
                x_printf(E, "Failed to set kv value what the key is roadID\n");
                return 0;
        }
	
        long old_roadID = kv_IMEI->roadID;
        if (set_IMEI_to_kv(gps_info, roadID,kv_IMEI, 1) == -1){
                x_printf(E, "Failed to set the kv value what the key is IMEI!\n");
                return 0;
        }

        return old_roadID;
}

int calculate(GPS_INFO *gps_info, ROAD_INFO *road_info, struct ev_loop *loop)
{

	long current_roadID = road_info->road_rootID * 1000 + road_info->segmentID;
	long old_roadID = 1;
	KV_IMEI kv_IMEI = {0};
	KV_ROADID kv_roadID = {0};
	int flag = get_IMEI_road(gps_info->IMEI, current_roadID, &kv_IMEI, gps_info);
	
	switch(flag) {
		case 0:
			old_roadID = change_road(gps_info, current_roadID, &kv_IMEI, &kv_roadID);
			if (0 == old_roadID){
				return -1;
			}
			update_to_redis(&kv_roadID, old_roadID, loop);
			break;
		case 1:
			if(same_road(gps_info, current_roadID, &kv_IMEI) == -1) {
				return -1;
			}
			break;
		case 2:
			if (set_IMEI_to_kv(gps_info, current_roadID, &kv_IMEI, 1) == -1){
				x_printf(E, "Failed to set the kv value what the key is IMEI FIRST!\n");
				return -1;
			}
			x_printf(D, "this IMEI is the first!\n");
			break;
		default:
			break;
	}
	return 0;
}

