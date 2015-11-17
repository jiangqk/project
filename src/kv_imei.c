#include "kv_imei.h"

extern kv_handler_t * g_kv_handler;
 

int kv_cmd(char * buff,kv_answer_t *ans)
{	
	ans = kv_ask(g_kv_handler, buff, strlen(buff));
	if (ans->errnum != ERR_NONE){
		if (ans->errnum == ERR_NIL){
			x_printf(D, "ERRROR ：%s this IMEI has not data!\n",buff);
			kv_answer_release(ans);
			return 1;
		}
		x_printf(E, "ERROR :%s, errnum is %d, err is %s\n", buff,ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}
	
	return 0 ;
	
}
 
void get_IMEI_to_mm(KV_IMEI *kv_IMEI,kv_answer_t *ans)
{
	unsigned long len = kv_answer_length(ans);	
		
	 if(len != 1){
		int i = 0;
		kv_answer_value_t *value = NULL;
		kv_answer_iter_t *iter = NULL;
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);
		while((value = kv_answer_next(iter)) != NULL){
			i = i + 1;
			switch(i){
				case 2:  kv_IMEI->count = atoi(value->ptr); break;
				case 4:  kv_IMEI->max_speed_num = atoi(value->ptr); break;
				case 6:  kv_IMEI->max_speed = atoi(value->ptr); break;
				case 8:  kv_IMEI->str_time = atoll(value->ptr); break;
				case 10: kv_IMEI->end_time = atoll(value->ptr); break;
				case 12: kv_IMEI->roadID = atoll(value->ptr); break;
				default: break;
			}
		}
	}
		kv_answer_release(ans);	
}
 
int get_IMEI_from_kv(long IMEI, KV_IMEI *kv_IMEI)
{
	char buff[10240] = {0};
	unsigned long len  = 0;
	kv_answer_t *ans;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "hgetall %ld", IMEI);
	
	len = kv_cmd(buff,ans);
	if (1 == len )
		return 1;
	else if (-1 == len )
		return -1;
	get_IMEI_to_mm(kv_IMEI,ans);
	
	return 0;

}

int set_IMEI_to_kv(GPS_INFO *gps_info, long roadID, KV_IMEI *kv_IMEI, int flag)
{
	char buff[1240] = {0};
	memset(buff, '\0', sizeof(buff));
	kv_answer_t *ans = NULL;
	
	if (flag == 1){
		sprintf(buff,"hmset %ld count 1 max_speed_num %d max_speed %d str_time %ld end_time %ld roadID %ld",
				gps_info->IMEI, gps_info->max_speed, gps_info->max_speed, 
				gps_info->start_time, gps_info->end_time, roadID);
	}else if (flag == 2){
		sprintf(buff, 
				"hmset %ld count %d max_speed_num %d max_speed %d str_time %ld end_time %ld roadID %ld", 
				gps_info->IMEI, kv_IMEI->count, kv_IMEI->max_speed_num, kv_IMEI->max_speed, 
				kv_IMEI->str_time, kv_IMEI->end_time, roadID);
	}
	
	kv_cmd(buff,ans);
	kv_answer_release(ans);
	
	return 0;	
}
