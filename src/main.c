#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "load_sniff_cfg.h"
#include "apply_def.h"
#include "rr_cfg.h"
#include "pool_api.h"
#include "libkv.h"

#include "sniff_evuv_cpp_api.h"

#include "add_session_cmd.h"

struct swift_cfg_list g_swift_cfg_list = {};
struct sniff_cfg_list g_sniff_cfg_list = {};
struct rr_cfg_file g_rr_cfg_file = {};
kv_handler_t *g_kv_handler = NULL;
kv_answer_t *g_kv_ans = NULL;

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;
	struct mount_info *link = NULL;
        int i = 0;
        
        NewArray(LIMIT_CHANNEL_KIND, link);
	
	for (i=0; i < LIMIT_CHANNEL_KIND; i++) {
		link[i].list = sniff_start( p_swift_worker, p_swift_worker->index, i );
		link[i].next = &link[i+1];
	}
        
	link[ LIMIT_CHANNEL_KIND - 1 ].next = NULL;
	p_swift_worker->mount = link;
}

#ifdef STORE_USE_QUEUE
static bool sniff_task_report(void *user, void *task)
{
        bool    ok = false;
        SNIFF_WORKER_PTHREAD *sniff_worker = (SNIFF_WORKER_PTHREAD *)user;

        
        ok = supex_task_push(&sniff_worker->tlist, task);
        if (ok) {
                x_printf(D, "push queue ok!");
                ATOMIC_INC(&sniff_worker->thave);
        } else {
                x_printf(E, "push queue fail!");
        }
        
	return ok;
}
static bool sniff_task_lookup(void *user, void *task)
{
        bool    ok = false;

        SNIFF_WORKER_PTHREAD *sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
        
        ok = supex_task_pull(&sniff_worker->tlist, task);
        if (ok) {
                x_printf(D, "pull queue ok!");
                ATOMIC_DEC(&sniff_worker->thave);
        }
        
        return ok;
}
#endif

#ifdef STORE_USE_UCMQ
static bool sniff_task_report(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char temp[32] = {};
	sprintf(temp, "%d_%d_%d", p_sniff_worker->batch, p_sniff_worker->genus, p_sniff_worker->index);
	bool ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));
	if (ok) {
                ATOMIC_INC(&p_sniff_worker->thave);
		x_printf(D, "push queue ok!");
        } else {
                x_printf(E, "push queue fail!");
        }
	return ok;
}
static bool sniff_task_lookup(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char temp[32] = {};
	sprintf(temp, "%d_%d_%d", p_sniff_worker->batch, p_sniff_worker->genus, p_sniff_worker->index);
	bool ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
	if (ok) {
                ATOMIC_DEC(&p_sniff_worker->thave);
		x_printf(D, "pull queue ok!");
	}
	return ok;
}
#endif
#ifdef STORE_USE_UCMQ_AND_QUEUE
#define MAX_SNIFF_TEMP_QUEUE_NUMBER		2//must >= 2
enum {
	MARK_USE_QUEUE = 0,
	MARK_USE_UCMQ,
};
struct queue_stat_info {
	int mark_report;
	unsigned int shift_report;
	int mark_lookup;
	unsigned int shift_lookup;
	int step_lookup;
	struct sniff_task_node temp;
	struct supex_task_list swap;
};

static struct queue_stat_info *g_queue_stat_list = NULL;

static bool sniff_task_report(void *user, void *task)
{
	bool ok = false;
	int new_mark = 0;
	unsigned int new_shift = 0;
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	struct sniff_task_node *p_task = (struct sniff_task_node *)task;
	struct queue_stat_info *p_stat = &g_queue_stat_list[ p_sniff_worker->batch * LIMIT_CHANNEL_KIND * g_sniff_cfg_list.file_info.worker_counts + p_sniff_worker->genus * g_sniff_cfg_list.file_info.worker_counts +  p_sniff_worker->index ];

	do {
		//---->use queue
		new_mark = MARK_USE_QUEUE;
		if (p_stat->mark_report == MARK_USE_QUEUE){
			p_task->shift = p_stat->shift_report;
		}else{
			p_task->shift = p_stat->shift_report + 1;
		}
		new_shift = p_task->shift;
		ok = supex_task_push(&p_sniff_worker->tlist, p_task);
		if (ok){
			x_printf(D, "push queue ok!");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
#if 1
			ATOMIC_INC(&p_sniff_worker->thave);
#endif
			break;
		}else{
			x_printf(W, "push queue failed!");
		}

		/*******************/
		char temp[32] = {};
		sprintf(temp, "%d_%d_%d", p_sniff_worker->batch, p_sniff_worker->genus, p_sniff_worker->index);
		/*******************/

		//---->use ucmq
		new_mark = MARK_USE_UCMQ;
		if (p_stat->mark_report == MARK_USE_UCMQ){
			p_task->shift = p_stat->shift_report;
		}else{
			p_task->shift = p_stat->shift_report + 1;
		}
		new_shift = p_task->shift;
		ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));
		if (ok) {
			x_printf(D, "push ucmq ok!");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
#if 1
			ATOMIC_INC(&p_sniff_worker->thave);
#endif
			break;
		}else{
			x_printf(E, "push ucmq failed!");
		}
	}while(0);

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	char temp[32] = {};
	bool ok = false;
        
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	struct sniff_task_node *p_task = (struct sniff_task_node *)task;
	struct queue_stat_info *p_stat = &g_queue_stat_list[ p_sniff_worker->batch * LIMIT_CHANNEL_KIND * g_sniff_cfg_list.file_info.worker_counts + p_sniff_worker->genus * g_sniff_cfg_list.file_info.worker_counts +  p_sniff_worker->index ];
#if 1
	AO_T have = ATOMIC_GET(&p_sniff_worker->thave);
        
	if (have <= 0 && p_stat->step_lookup == 2) {
		ok = supex_task_pull(p_sniff_worker->glist, p_task);
                if (ok) {
                        x_printf(D, "pull queue ok!");
                }
                return ok;
	}
#endif
	switch(p_stat->step_lookup){
		case 0:
			/*init vms step*/
			ok = supex_task_pull(&p_sniff_worker->tlist, p_task);
			if (ok){
                                x_printf(D, "pull queue ok!");
#if 1
				ATOMIC_DEC(&p_sniff_worker->thave);
#endif
				p_stat->step_lookup ++;
			}
			break;
		case 1:
			/*done old step*/
			/*******************/
			sprintf(temp, "%d_%d_%d", p_sniff_worker->batch, p_sniff_worker->genus, p_sniff_worker->index);
			/*******************/
			ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
			if (ok){
				if (p_task->shift == 1){
					supex_task_push(&p_stat->swap, p_task);//push l
					p_stat->step_lookup ++;
				}else{
					//do nothing, is old task
				}
			}else{
				p_stat->step_lookup ++;
			}
			break;
		case 2:
			/*done new step*/
			if (p_stat->mark_lookup == MARK_USE_QUEUE){
				ok = supex_task_pull(&p_sniff_worker->tlist, p_task);
				if (ok){
					if (p_task->shift == p_stat->shift_lookup){
						//do nothing, is next task
#if 1
						ATOMIC_DEC(&p_sniff_worker->thave);
#endif
					}else{
						/*not first push then pop,it will get the push task in one loop.*/
						memcpy(&p_stat->temp, p_task, sizeof(struct sniff_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
#if 1
						if (ok){
							ATOMIC_DEC(&p_sniff_worker->thave);
						}
#endif
						supex_task_push(&p_stat->swap, &p_stat->temp);//push l
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					}
				}else{
#if 0
					/*maybe no task come,so add idle task come*/
					//do nothing, no task
#else
					if (have > 0){
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
						if (ok){
							ATOMIC_DEC(&p_sniff_worker->thave);
						}
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					}else{
						//do nothing, no task
					}
#endif
				}
			}else{
				/*******************/
				sprintf(temp, "%d_%d_%d", p_sniff_worker->batch, p_sniff_worker->genus, p_sniff_worker->index);
				/*******************/
				ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));
				if (ok){
					if (p_task->shift == p_stat->shift_lookup){
						//do nothing, is next task
#if 1
						ATOMIC_DEC(&p_sniff_worker->thave);
#endif
					}else{
						memcpy(&p_stat->temp, p_task, sizeof(struct sniff_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
#if 1
						if (ok){
							ATOMIC_DEC(&p_sniff_worker->thave);
						}
#endif
						supex_task_push(&p_stat->swap, &p_stat->temp);//push l
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					}
				}else{
#if 0
					//do nothing, no task
#else
					if (have > 0){
						ok = supex_task_pull(&p_stat->swap, p_task);//pull l
						if (ok){
							ATOMIC_DEC(&p_sniff_worker->thave);
						}
						p_stat->shift_lookup ++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					}else{
						//do nothing, no task
					}
#endif
				}
			}
			break;
		default:
			break;
	}
	return ok;
}
#endif


/**
 * 根据条件编译初始化文件队列和内存队列
 */
static void swift_entry_init(void)
{
#if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE)
	bool ok = mq_store_init("./mq_data/logs", "./mq_data/data");
	if (!ok) {
		x_perror("mq_store_init");
		exit(EXIT_FAILURE);
	}
#ifdef STORE_USE_UCMQ_AND_QUEUE
	int all = g_swift_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.worker_counts * LIMIT_CHANNEL_KIND;
	
        NewArray(all, g_queue_stat_list);
        
	assert(g_queue_stat_list);
        
	while(all--) {
		struct queue_stat_info *p_stat = &g_queue_stat_list[ all ];
		supex_task_init(&p_stat->swap, sizeof(struct sniff_task_node), MAX_SNIFF_TEMP_QUEUE_NUMBER);
	}
#endif
#endif
        
        /*
         * 初始化支持的命令
         */
        init_session_cmd();
	
	pool_api_init(g_rr_cfg_file.map_server_host, g_rr_cfg_file.map_server_port, 10000, false);
	pool_api_init(g_rr_cfg_file.links[0].host, g_rr_cfg_file.links[0].port, 10000, false);
}

static void swift_shut_down(void)
{
        const SWIFT_WORKER_PTHREAD    *swift_worker = g_swift_worker_pthread;
        const int swift_worker_total = SWIFT_WORKER_COUNTS;
        int     i = 0;
        int     thds = 0;
        
        struct ThreadSuspend *cond = NULL;
        
        New(cond);
        
        if (! ThreadSuspendInit(cond)) {
                exit(EXIT_FAILURE);
        }
        
        /*通过每个swift_worker挂起sniff_worker的所有线程*/
        for (i = 0; i < swift_worker_total; i++) {
                struct mount_info       *link = NULL;
                
                int j = 0;
                
                link = (struct mount_info*)swift_worker[i].mount;
                
                for (j = 0; j < LIMIT_CHANNEL_KIND; j++) {
                        
                        thds++;
                        
                        sniff_suspend_thread(link[j].list, cond);
                }
        }
        /*
         * 等待所有sniff_worker挂起
         */
        ThreadSuspendWait(cond, thds * SNIFF_WORKER_COUNTS);
        
        /*
         * 由于 sniff_worker 线程还在挂起状态，所以不能释放挂起条件
         */
}

/*
 * 每个swift_worker 对应一批（一个）sniff_worker
 * @see swift_pthrd_init()
 */
static void swift_reload_cfg(void)
{
        struct supex_argv       *argv = &g_swift_cfg_list.argv_info;
        struct swift_cfg_file   *fileinfo = &g_swift_cfg_list.file_info;

        const SWIFT_WORKER_PTHREAD    *swift_worker = g_swift_worker_pthread;
        const int               swift_worker_total = SWIFT_WORKER_COUNTS;
        int                     i = 0;
        int                     thds = 0;
        bool                    ok = false;

        struct ThreadSuspend cond = NULLOBJ_THREADSUSPEND;

        /*通过每个swift_worker挂起sniff_worker的所有线程*/
        for (i = 0; i < swift_worker_total; i++) {
                struct mount_info *link = NULL;

                int j = 0;

                link = (struct mount_info *)swift_worker[i].mount;

                for (j = 0; j < LIMIT_CHANNEL_KIND; j++) {
                        thds++;

                        sniff_suspend_thread(link[j].list, &cond);
                }
        }

        /*
         * 等待所有sniff_worker挂起
         */
        ThreadSuspendWait(&cond, thds * SNIFF_WORKER_COUNTS);

        /*
         * 重新加载配置文件
         */
        ok = reload_swift_cfg_file(fileinfo, argv->conf_name);

        if (ok) {
                char path[MAX_PATH_SIZE] = {};

                snprintf(path, sizeof(path), "%s/%s", fileinfo->log_path, fileinfo->log_file);

                SLogOpen(path, SLogIntegerToLevel(fileinfo->log_level));

                x_printf(D, "reload configure file [%s] success.", argv->conf_name);
        }

        ok = read_rr_cfg(&g_rr_cfg_file, argv->conf_name);

        if (ok) {
                x_printf(D, "reload configure file [%s] success.", argv->conf_name);
        }

        /*
         * 复位 sniff_worker
         */
        ThreadSuspendEnd(&cond);
}


int main(int argc, char** argv)
{
        bool    ok = false;
	//---> init swift
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	ok = load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
        if (!ok) {
                exit(EXIT_FAILURE);
        }
        
	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_call;
	

	g_swift_cfg_list.entry_init = swift_entry_init;

	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;

        g_swift_cfg_list.reload_cfg = swift_reload_cfg;

        g_swift_cfg_list.shut_down = swift_shut_down;
        

	swift_mount(&g_swift_cfg_list);

	//---> init route
	ok = read_rr_cfg(&g_rr_cfg_file, g_swift_cfg_list.argv_info.conf_name);
        if (!ok) {
                exit(EXIT_FAILURE);
        }

	//---> init libkv
	g_kv_handler = kv_create(NULL);	

	//---> init sniff
        
        snprintf(g_sniff_cfg_list.argv_info.conf_name,
                 sizeof(g_sniff_cfg_list.argv_info.conf_name),
                 "%s", g_swift_cfg_list.argv_info.conf_name);
        
        snprintf(g_sniff_cfg_list.argv_info.serv_name,
                 sizeof(g_sniff_cfg_list.argv_info.serv_name),
                 "%s", g_swift_cfg_list.argv_info.serv_name);
        
        snprintf(g_sniff_cfg_list.argv_info.msmq_name,
                 sizeof(g_sniff_cfg_list.argv_info.msmq_name),
                 "%s", g_swift_cfg_list.argv_info.msmq_name);

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
	
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;
	
	g_sniff_cfg_list.vmsys_init = sniff_vms_init;

	sniff_mount(&g_sniff_cfg_list);

	swift_start();
        
	return 0;
}
