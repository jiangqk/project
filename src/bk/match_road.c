#include "match_road.h"
#include "calculate.h"

#define ASYNC_LIBEV_THRESHOLD 10

extern struct rr_cfg_file g_rr_cfg_file;

char MAP_HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
"User-Agent: curl/7.33.0\r\n"
"Host: %s:%d\r\n"
"Content-Type: application/json; charset=utf-8\r\n"
"Connection:%s\r\n"
"Content-Length:%d\r\n"
"Accept: */*\r\n\r\n%s";	


static int judge_angle(int angle, int direction)
{
	if(angle == -1)
	{
		return 0;
	}

	int diff = abs(angle - direction);
	if(diff < 30)
	{
		return 1;
	}

	return 0;
}

static int decode_body(char *body, ROAD_INFO *road_info, int direction)
{
	const char* str_val = NULL;
	cJSON *cfg = NULL;
	long long lineID = -1;

	cJSON *err = cJSON_Parse(body);
	if (err == NULL){
		x_printf(E, "Failed to create cJSON object");
		return -1;
	}

	cJSON *obj = cJSON_GetObjectItem(err, "ERRORCODE");
	if (obj == NULL){
		cJSON_Delete(err);
		return -1;
	}

	if (0 != atoi(obj->valuestring)){
		memset(road_info->road_name, 0, sizeof(road_info->road_name));
		cJSON_Delete(err);
		return -1;
	}

	obj = cJSON_GetObjectItem(err, "RESULT");
	if (NULL != obj){
		int flag = 0;
		int angle = 0;
		if (NULL != (cfg =  cJSON_GetObjectItem(obj, "angle2"))){
			angle = cfg->valueint;
			flag = judge_angle(angle, direction);		
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "angle1"))){
			road_info->direction = cfg->valueint;
		}else{
			road_info->direction = angle;
		}
		x_printf(D, "road_info direction is %d\n", road_info->direction);

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "lineID"))){
			lineID = atoll(cfg->valuestring);
			road_info->roadID = lineID / 1000 + flag;
			x_printf(D, "road_info roadID is %lld\n", road_info->roadID);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "RN"))){
			str_val = cfg->valuestring;
			if (strcmp(str_val, "NULL") == 0){
				str_val ==    "ÎÃ³ƵÀ·"; 
			}
			strcpy(road_info->road_name, str_val);
			x_printf(D,"roadName: %s\n", road_info->road_name);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "CD"))){
			road_info->city_code = atol(cfg->valuestring);
			x_printf(D, "road_info city_code is %ld\n", road_info->city_code);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "LN"))){
			road_info->length = atol(cfg->valuestring);
			x_printf(D, "road_info length is %ld\n", road_info->length);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "CS"))){
			str_val = cfg->valuestring;
			strncpy(road_info->cross_section, str_val, MAX_CROSS_SECTION);
			x_printf(D, "road_info cross_section is %s\n", road_info->cross_section);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "SR"))){
			str_val = cfg->valuestring;
			if (strcmp(str_val, "NULL") == 0){
				road_info->speed_limit = -1;
			}else{
				road_info->speed_limit = atoi(str_val);
				x_printf(D, "road_info speed_limit is %d\n", road_info->speed_limit);
			}
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "RT"))){
			road_info->road_level = atoi(cfg->valuestring);
			x_printf(D, "road_info road_level is %d\n", road_info->road_level);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "L1"))){
			road_info->start_lon = atof(cfg->valuestring);
			x_printf(D, "road_info start_lon is %lf\n", road_info->start_lon);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "B1"))){
			road_info->start_lat = atof(cfg->valuestring);
			x_printf(D, "road_info start_lat is %lf\n", road_info->start_lat);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "L2"))){
			road_info->end_lon = atof(cfg->valuestring);
			x_printf(D, "road_info end_lon is %lf\n", road_info->end_lon);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "B2"))){
			road_info->end_lat = atof(cfg->valuestring);
			x_printf(D, "road_info end_lat is %lf\n",  road_info->end_lat);
		}

		if (NULL != (cfg = cJSON_GetObjectItem(obj, "CC"))){
			road_info->county_code = atoi(cfg->valuestring);
			x_printf(D, "road_info country_code is %d\n", road_info->county_code);
		}

	}

	cJSON_Delete(err);
	return 0;
}






void http_callback(struct async_ctx *ac, void *reply, void *data)
{
	x_printf(D, "------------\n");
	if (reply){
		struct http_status *p_hst = &ac->obj.replies.work->parse.http_info.hs;
		struct net_cache *cache = reply;
		GPS_INFO *gps_info = data;
		int direction = gps_info->direction;	
		long long IMEI = gps_info->IMEI;

		x_printf(D, "IMEI IS:[%ld]\thttp_callback data: %s", IMEI, cache->buf_addr);

		if (p_hst->body_size == 0) {
			x_printf(E, "Failed to send http request to map server!");
		}else{
			char *body = cache->buf_addr + p_hst->body_offset;
			int size = p_hst->body_size;
			x_printf(D,  "RECEIVE BODY:%s\n", body);

			ROAD_INFO *road_info = (ROAD_INFO *)calloc(1, sizeof(ROAD_INFO));
			assert(road_info);

			int ok = decode_body(body, road_info, direction);
			if (0 == ok){
				x_printf(D, "direction:%d\n", road_info->direction);
				x_printf(D, "road_name:%s\n", road_info->road_name);

				struct ev_loop *loop = ac->obj.settings.loop;
				calculate(gps_info, road_info, loop);
			}
			free(road_info);
		}
	}
	free(data);
	return;
}

static int forward_to_server(char *host, int port, const char *data, size_t size, struct ev_loop *loop, ASYNC_CALL_BACK fncb, void *with)
{
	struct cnt_pool *cpool = NULL;
	struct async_ctx *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 1);
	if (ac) {
		void *sfd = (void*)(intptr_t)-1;
		int rc = pool_api_gain(&cpool, host, port, &sfd);
		if (rc) {
			async_distory(ac);
			return -1; 
		}

		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, fncb, with, data, size);

		async_startup(ac);
		return 0;
	}
	return -1;
}

int match_road(struct ev_loop *loop, GPS_INFO *gps_info)
{
	char *host = g_rr_cfg_file.map_server_host;
	int port = g_rr_cfg_file.map_server_port;
	const char *appKey = g_rr_cfg_file.appKey;
	const char *secret = g_rr_cfg_file.secret;
	char sign_src[256] = {0};
	char sign_dst[64] = {0};
	char body[256] = {0};
	char buff[512] = {0};

	snprintf(sign_src, 256, "appKey%sdirection%dlatitude%lflongitude%lfsecret%s",
			appKey, gps_info->direction, gps_info->latitude, gps_info->longitude, secret);
	sha1_hash(sign_src, strlen(sign_src), sign_dst);

	snprintf(body, 256, "longitude=%lf&latitude=%lf&direction=%d&sign=%s&appKey=%s",
			gps_info->longitude, gps_info->latitude, gps_info->direction, sign_dst, appKey);

	snprintf(buff, 500, MAP_HTTP_FORMAT, "mapapi/v3/pointMatchRoad", host, port, "Keep-Alive", strlen(body), body);

	x_printf(D, "Whole http string: %s", buff);

	return forward_to_server(host, port, buff, strlen(buff), loop, http_callback, gps_info);
}
