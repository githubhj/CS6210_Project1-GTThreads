#ifndef __GTTHREAD_H
#define __GTTHREAD_H

#include <ucontext.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define KB (1024)
#define MB (1024*KB)
#define STACK_SIZE MB

extern volatile unsigned long THREAD_COUNT;
extern volatile unsigned long GLOBAL_PERIOD;
extern sigset_t signal_info;


typedef volatile long gtid_t;
typedef struct _gtthread_mutex_t{
	gtid_t threadid;
}gtthread_mutex_t;

typedef struct _gtthread_t{
	gtid_t parent_threadid;
	gtid_t threadid;
	ucontext_t thread_context;
	volatile int exited;
	volatile int finished;
	volatile int cancelled;
	volatile int runnning;
	void* returnval;
	volatile int child_count;
}gtthread_t;


typedef struct _dlink_t {
	struct _dlink_t* prev;
	struct _dlink_t* next;
	gtthread_t *thread_block;
}dlink_t;

typedef struct _queue_t{
	dlink_t* begin;/*Always points to main thread*/
	dlink_t* end;/*Always points to new thread*/
	dlink_t* running;/*Points to running thread*/
	volatile int length;
	volatile int main_exited;
}queue_t;

extern queue_t* schedule_queue;

dlink_t* PopTop_Queue(queue_t*);

typedef struct _return_data_t{
	gtid_t threadid;
	void* returnval;
}return_data_t;

typedef struct _return_node_t{
	return_data_t return_data;
	struct _return_node_t* next;
}return_node_t;

extern return_node_t* head_return_list;

void WrapperFunction(void *(*start_routine)(void*), void *arg);

int PushBack_Queue(dlink_t* , queue_t* );

int DelTop_Queue(queue_t*);

int InitLink(dlink_t* , gtthread_t *, gtid_t, gtid_t);

void timer_handler(int);

struct itimerval* StopTimer(void);

void StartTimer(struct itimerval*);

/* Must be called before any of the below functions. Failure to do so may
 * result in undefined behavior. 'period' is the scheduling quantum (interval)
 * in microseconds (i.e., 1/1000000 sec.). */
void gtthread_init(long period);

/* see man pthread_create(3); the attr parameter is omitted, and this should
 * behave as if attr was NULL (i.e., default attributes) */
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);

/* see man pthread_join(3) */
int  gtthread_join(gtthread_t thread, void **status);

/* gtthread_detach() does not need to be implemented; all threads should be
 * joinable */

/* see man pthread_exit(3) */
void gtthread_exit(void *retval);

/* see man sched_yield(2) */
int gtthread_yield(void);

/* see man pthread_equal(3) */
int  gtthread_equal(gtthread_t t1, gtthread_t t2);

/* see man pthread_cancel(3); but deferred cancelation does not need to be
 * implemented; all threads are canceled immediately */
int  gtthread_cancel(gtthread_t thread);

/* see man pthread_self(3) */
gtthread_t gtthread_self(void);


/* see man pthread_mutex(3); except init does not have the mutexattr parameter,
 * and should behave as if mutexattr is NULL (i.e., default attributes); also,
 * static initializers do not need to be implemented */
int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

/* gtthread_mutex_destroy() and gtthread_mutex_trylock() do not need to be
 * implemented */

#endif
