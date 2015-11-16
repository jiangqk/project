#include "kv_road.h"
extern kv_handler_t * g_kv_handler;

int set_roadID_to_kv(KV_IMEI * kv_IMEI, KV_ROADID *kv_roadID, GPS_INFO *gps_info, long roadID)
{
	char buff[10240] = {0};
	kv_answer_t *ans = NULL;
	memset(buff, '\0', sizeof(buff));
	
	kv_roadID->IMEI = gps_info->IMEI;
	kv_roadID->max_speed =kv_IMEI->max_speed;
	kv_roadID->avg_speed = kv_IMEI->max_speed_num / kv_IMEI->count;
	kv_roadID->end_time = kv_IMEI->end_time;
	kv_roadID->used_time = kv_IMEI->end_time -  kv_IMEI->str_time; 
	
	sprintf(buff, 
		"hmset %ld IMEI %ld max_speed %d avg_speed %d end_time %ld used_time %ld",
                        roadID, kv_roadID->IMEI, kv_roadID->max_speed, 
			kv_roadID->avg_speed, kv_roadID->end_time,  kv_roadID->used_time);
			
	kv_cmd(buff,ans);
	kv_answer_release(ans);
	return 0;
}
