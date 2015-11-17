#include <assert.h>
#include <unistd.h>

#include "sniff_api.h"
#include "sniff_evuv_cpp_api.h"
#include "rr_cfg.h"
#include "tcp_api.h"
#include "async_api.h"
#include "apply_def.h"
#include "pool_api.h"
#include "utils.h"
#include "decode_gps.h"
#include "match_road.h"


#define REDIS_ERR -1
#define REDIS_OK 0
extern struct rr_cfg_file g_rr_cfg_file;

static void _vms_erro( void **base )
{
}


static void *_vms_new( void )
{
	return NULL;
}

static int _vms_init( void **base, int last, struct sniff_task_node *task )
{
	if (*base != NULL){
		x_printf(S, "No need to init LUA VM!");
		return 0;
	}
	*base = _vms_new( );
	//assert( *base );
	return 0;
}

int sniff_vms_init(void *user, void *task)
{
	int error = 0;
	error = sniff_for_alone_vm( user, task, _vms_init, _vms_erro );
	if (error) {
		exit(EXIT_FAILURE);
	}
	return error;
}


int import_to_redis(char command[],void * loop,char host[],unsigned short port)
{
	printf("import_to_redis:%s\n",command);
	struct cnt_pool *cpool = NULL;
	struct async_ctx *ac = NULL;

        ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 2);
        if (ac) {
                void *sfd = (void*)-1;
                int rc = pool_api_gain(&cpool, host, port, &sfd);
                if (rc){
//                        cnt_free ( cpool, &sfd );
                        async_distory(ac);
                        return -1; 
                }

                /*data*/
                char *proto;
                int ok = cmd_to_proto(&proto, command);
                if (ok == REDIS_ERR) {
                        cnt_push ( cpool, &sfd );
                        async_distory(ac);
                        return -1; 
                }
                /*send*/
		async_command(ac, PROTO_TYPE_REDIS, (int)sfd, cpool, NULL, NULL, proto, strlen(proto));
                free(proto);

		async_startup(ac);
                return 0;
        }
        
        return -1;
}




int sniff_vms_call(void *user, void *task)
{
	struct sniff_task_node *p_task = task;
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t delay = time(NULL) - p_task->stamp;
	struct ev_loop *loop = p_sniff_worker->evuv.loop;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %ld\t<delay> %ld",
			p_sniff_worker->genus, p_task->shift, p_task->stamp, delay);

	x_printf(D, "%s", p_task->data);

	if (delay >= OVERLOOK_DELAY_LIMIT){
		x_printf(W, "overlook one task");
		return -1;
	}

	GPS_INFO *gps_info = (GPS_INFO *)calloc(1, sizeof(GPS_INFO));
	assert(gps_info);
	int ok = decode_gps(p_task->data, gps_info);
        if (ok == -1){
                x_printf(E, "Failed decode http data!");
		free(gps_info); gps_info = NULL;
                return -1;
        }


        ok = match_road(loop, gps_info);
        if (ok == -1){
                x_printf(E, "Failed match road!");
		free(gps_info); gps_info = NULL;
                return -1;
        }
	return 0;
}

