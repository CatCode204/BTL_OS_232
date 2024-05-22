#include "os-cfg.h"
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue; //FORGET ABOUT THIS BECAUSE IT IS OUTDATED
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
	int i ;
	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
        mlq_ready_queue[i].slot = MAX_PRIO - i;
        mlq_ready_queue[i].top = -1;
        mlq_ready_queue[i].bot = 0;
    }
#endif
	ready_queue.size = 0;
    ready_queue.bot = -1;
    ready_queue.top = 0;
	run_queue.size = 0;
    run_queue.bot = -1;
    run_queue.top = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
    struct pcb_t* rt = NULL;
    pthread_mutex_lock(&queue_lock);
    for (int i=0;i < MAX_PRIO;++i)
        if (!empty(&mlq_ready_queue[i]) && mlq_ready_queue[i].slot > 0) {
            --mlq_ready_queue[i].slot;
            rt = dequeue(&mlq_ready_queue[i]);
            break;
        }
    if (rt == NULL) {
        for (int i = 0; i < MAX_PRIO; ++i)
            mlq_ready_queue[i].slot = MAX_PRIO - i;
        for (int i = 0; i < MAX_PRIO; ++i)
            if (!empty(&mlq_ready_queue[i])) {
                rt = dequeue(&mlq_ready_queue[i]);
                --mlq_ready_queue[i].slot;
                break;
            }
    }
    pthread_mutex_unlock(&queue_lock);
    return rt;
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
    struct pcb_t * rt = NULL;
    pthread_mutex_lock(&queue_lock);
    rt = dequeue(&ready_queue);
    if (rt != NULL) enqueue(&ready_queue, rt);
    pthread_mutex_unlock(&queue_lock);
}

void put_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


