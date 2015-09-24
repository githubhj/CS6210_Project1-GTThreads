#include "gtthread.h"
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "gtthread_sched.h"

queue_t* schedule_queue;
return_node_t* head_return_list;
volatile unsigned long THREAD_COUNT;
volatile unsigned long GLOBAL_PERIOD;
sigset_t signal_info;

void timer_handler(int signum){
	/*static int count =0;*/
	dlink_t* next_run_dlink;
	dlink_t* old_running_link;
	BlockSignals();

	/*Get next running link*/
	next_run_dlink = NextRunningLink(schedule_queue);
	if(next_run_dlink==NULL){
		if(schedule_queue->begin->thread_block->returnval != NULL){
			exit(*(int *)(schedule_queue->begin->thread_block->returnval));
		}
		else{
			exit(0);
		}
	}

	/*Save the context of present running link
	//local_context = *(ucontext_t*)return_context;
	//schedule_queue->running->thread_block->thread_context.uc_mcontext = local_context.uc_mcontext;*/

	/*Make present running link as running 0*/
	schedule_queue->running->thread_block->runnning = 0;

	/*Make temp_dlink as the present running link*/
	next_run_dlink->thread_block->runnning = 1;
	/*if((schedule_queue->running->thread_block->cancelled||schedule_queue->running->thread_block->exited||schedule_queue->running->thread_block->finished)){
		schedule_queue->running = next_run_dlink;

		if()
		old_running_link
	}*/
	old_running_link = schedule_queue->running;
	schedule_queue->running = next_run_dlink;

	InitTimer();
	UnblockSignals();
	swapcontext(&(old_running_link->thread_block->thread_context),&(schedule_queue->running->thread_block->thread_context));

	/*while(1){
		if(swapcontext(schedule_queue->running->thread_block->thread_context.uc_link,&(schedule_queue->running->thread_block->thread_context))==-1){
			fprintf(stderr,"ERROR:swapcontext() Failed!!!");
			exit(1);
		}

		//Dude I am done with my work
		BlockSignals();

		//Get next running link
		next_run_dlink = NextRunningLink(schedule_queue);

		//If returned link was the running link
		if(next_run_dlink == schedule_queue->running){
			//Dude I ran everything for you
			exit(0);
		}

		//Else execute the next running link
		else{
			//Make schedule queue running go finished
			schedule_queue->running->thread_block->finished = 1;
			schedule_queue->running->thread_block->runnning = 0;

			//Make temp_dlink as the present running link
			next_run_dlink->thread_block->runnning = 1;
			schedule_queue->running = next_run_dlink;

			//Initialize timer, gonna give this guy full time stamp
			InitTimer();

			//Unblock Signals
			UnblockSignals();

		}


	}*/
}

/* Just an example. Nothing to see here. */

/* Must be called before any of the below functions. Failure to do so may
 * result in undefined behavior. 'period' is the scheduling quantum (interval)
 * in microseconds (i.e., 1/1000000 sec.). */
void gtthread_init(long period){

	struct sigaction sa;
	struct itimerval timer;
	GLOBAL_PERIOD = period;

	/*Allocate memory for a link*/
	dlink_t *main_dlink;
	main_dlink = (dlink_t*)malloc(sizeof(dlink_t));
	gtthread_t *main_thread;
	main_thread = (gtthread_t*)malloc(sizeof(gtthread_t));

	//Initialize main link
	if(InitLink(main_dlink,main_thread,0,-1)==-1){
		fprintf(stderr,"ERRROR: InitLink() on main did not work\n");
	}
	main_dlink->thread_block->runnning = 1;

	//Initialize the dqueue
	schedule_queue = (queue_t*)malloc(sizeof(queue_t));
	schedule_queue->begin = main_dlink;
	schedule_queue->end = main_dlink;
	schedule_queue->running = main_dlink;
	schedule_queue->length = 1;
	schedule_queue->main_exited = 0;

	//Get main context
	if(getcontext(&(main_dlink->thread_block->thread_context))==-1){
			fprintf(stderr,"ERRROR: getcontext() on main did not work\n");
	}

	//Initialize return list
	head_return_list = NULL;

	/*Install timer handler as the signal handler for SIFVTALARM*/
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = timer_handler;
	sa.sa_flags = 0;
	sigaction (SIGVTALRM, &sa, NULL);

	/* Configure the timer to expire after period usec... */
	sigemptyset (&signal_info);
	sigaddset(&signal_info, SIGVTALRM);
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = (suseconds_t)period;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = (suseconds_t)period;

	/* Start a virtual timer. It counts down whenever this process is
	   executing. */
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}


int DelTop_Queue(queue_t* local_queue){
	dlink_t* temp_link;

	struct itimerval *old_timer;
	old_timer = StopTimer();

	temp_link = local_queue->begin;
	local_queue->begin = local_queue->begin->prev;
	local_queue->begin->next = NULL;
	local_queue->length--;

	free(temp_link->thread_block->thread_context.uc_link);
	//free(temp_link->thread_block->thread_context);
	free(temp_link);

	StartTimer(old_timer);
	return 0;
}

dlink_t* PopTop_Queue(queue_t* local_queue){

	dlink_t* temp_link;

	struct itimerval *old_timer;
	old_timer = StopTimer();

	//No Link to del
	if(local_queue->begin==NULL && local_queue->end == NULL){
		StartTimer(old_timer);
		return NULL;
	}

	//Only 1 Link
	else if(local_queue->length == 1){
		/*
		temp_link = local_queue->begin;
		local_queue->begin = local_queue->end = NULL;
		local_queue->length--;*/
		StartTimer(old_timer);
		return NULL;
	}

	//More than one Link
	else{
		temp_link = local_queue->begin;
		local_queue->begin = local_queue->begin->prev;
		local_queue->begin->next = NULL;
		local_queue->length--;
		StartTimer(old_timer);
		return temp_link;
	}
}

struct itimerval* StopTimer(){
	struct itimerval zero_timer;

	struct itimerval* old_timer = (struct itimerval*)malloc(sizeof(struct itimerval));

	//Timer off
	memset(&zero_timer, 0, sizeof(zero_timer));
	setitimer (ITIMER_VIRTUAL, &zero_timer, old_timer);

	return old_timer;
}

void StartTimer(struct itimerval* old_timer){
	setitimer (ITIMER_VIRTUAL, old_timer, NULL);
	free(old_timer);
}

/* see man pthread_create(3); the attr parameter is omitted, and this should
 * behave as if attr was NULL (i.e., default attributes) */
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg){
	struct itimerval* timer;
	gtid_t parent_id;
	dlink_t* thread_dlink;
	THREAD_COUNT++;

	//Allocate memory for a link
	thread_dlink = (dlink_t*)malloc(sizeof(dlink_t));
	if(thread_dlink == NULL){
		return -1;
	}


	//Stop Timer
	BlockSignals();
	parent_id = schedule_queue->running->thread_block->threadid;
	schedule_queue->running->thread_block->child_count++;
	UnblockSignals();

	//Initialize the link
	if(InitLink(thread_dlink,thread,THREAD_COUNT,parent_id)==-1){
		return -1;
	}

	//Make Thread Context
	if(getcontext(&(thread->thread_context))==-1){
		return -1;
	}
	thread->thread_context.uc_stack.ss_sp = (char*)malloc(sizeof(char)*20*MB);
	thread->thread_context.uc_stack.ss_size = sizeof(char)*20*MB;
	if(getcontext(thread->thread_context.uc_link)==-1){
		return -1;
	}
	makecontext(&(thread->thread_context),WrapperFunction, 2, start_routine,arg);

	//Add Link to Queue
	return(PushBack_Queue(thread_dlink,schedule_queue));
}

/* see man pthread_join(3) */
int  gtthread_join(gtthread_t thread, void **status){
	gtid_t thread_id;
	dlink_t* temp_dlink;

	thread_id = thread.threadid;
	BlockSignals();
	temp_dlink = FindNode(schedule_queue,thread_id);
	if(temp_dlink == NULL){
		return -1;
	}
	else if(temp_dlink == schedule_queue->begin)
	{
		while(schedule_queue->main_exited==0){
			UnblockSignals();
			gtthread_yield();
			BlockSignals();
		}
		if(temp_dlink->thread_block->cancelled==1){
					*status = (void*)(int)-1;
		}
		else if(status!=NULL){
			*status = temp_dlink->thread_block->returnval;
		}
		UnblockSignals();
		return 0;
	}
	else{

		while(!(temp_dlink->thread_block->exited || temp_dlink->thread_block->finished|| temp_dlink->thread_block->cancelled)){
			UnblockSignals();
			gtthread_yield();
			BlockSignals();
		}
		if(temp_dlink->thread_block->cancelled==1){
			if(status!=NULL){
				*status = (void*)(int)-1;
			}
		}
		else if(status!=NULL){
			*status = temp_dlink->thread_block->returnval;
		}
		UnblockSignals();
		return 0;
	}
}

/* gtthread_detach() does not need to be implemented; all threads should be
 * joinable */

/* see man pthread_exit(3) */
void gtthread_exit(void *retval){
	dlink_t* temp_link;
	dlink_t* temp_link2;

	BlockSignals();
	temp_link = schedule_queue->running;
	schedule_queue->running->thread_block->returnval = retval;
	temp_link->thread_block->exited = 1;

	if(schedule_queue->running != schedule_queue->begin){
		temp_link2 = FindNode(schedule_queue,schedule_queue->running->thread_block->parent_threadid);
		if(temp_link2 == NULL){
			fprintf(stderr,"gtthread_exit() error: no parent found");
		}
		else{
			temp_link2->thread_block->child_count--;
			UnblockSignals();
			raise(SIGVTALRM);
		}
	}

	else{
		temp_link->thread_block->exited = 0;
		schedule_queue->main_exited = 1;
		while(schedule_queue->running->thread_block->child_count!=0){
				UnblockSignals();
				raise(SIGVTALRM);
				BlockSignals();
		}

		//exit(*(int*)retval);
	}


	/*while(temp_link->thread_block->child_count!=0){
		UnblockSignals();
		raise(SIGVTALRM);
		BlockSignals();
	}

	if(schedule_queue->running == schedule_queue->begin){
		exit(0);
	}
	else{
		temp_link2 = FindNode(schedule_queue,schedule_queue->running->thread_block->parent_threadid);
		if(temp_link2 == NULL){
			fprintf(stderr,"Error: no parent found");
		}
		else{
			temp_link2->thread_block->child_count--;
			temp_link->thread_block->exited = 1;
			UnblockSignals();
			raise(SIGVTALRM);
		}
	}*/
}

/* see man sched_yield(2) */
int gtthread_yield(void){
	dlink_t *next_run_dlink,*temp_dlink;

	if(schedule_queue == NULL){
		printf("ERROR: Dude you did not initialize stuff!!");
		exit(1);
	}
	else{
		raise(SIGVTALRM);
		return 0;
	}

}

/* see man pthread_equal(3) */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
	if(t1.threadid==t2.threadid){
		return 1;
	}
	else{
		return 0;
	}
}

/* see man pthread_cancel(3); but deferred cancelation does not need to be
 * implemented; all threads are canceled immediately */
int  gtthread_cancel(gtthread_t thread){
	dlink_t* temp_link;

	temp_link = FindNode(schedule_queue,thread.threadid);

	BlockSignals();

	//No thread found, return
	if(temp_link == NULL){
		UnblockSignals();
		return -1;
	}
	else if(temp_link->thread_block->cancelled == 1 || temp_link->thread_block->exited == 1 || temp_link->thread_block->finished == 1){
			UnblockSignals();
			return -1;
	}

	//Else if thread was main, exit
	else if(temp_link == schedule_queue->begin){

		if(schedule_queue->main_exited == 1){
			UnblockSignals();
			return -1;
		}
		else{
			schedule_queue->main_exited = 1;
			temp_link->thread_block->cancelled =1;
			UnblockSignals();
		}
	}
	//Else if thread was running
	else if(temp_link  == schedule_queue->running){
		temp_link->thread_block->cancelled =1;
		UnblockSignals();
		raise(SIGVTALRM);
	}
	//Else some other thread
	else{
		temp_link->thread_block->cancelled =1;
		UnblockSignals();
		return 0;
	}
	return -1;
}

/* see man pthread_self(3) */
gtthread_t gtthread_self(void){
	struct itimerval *timer;
	gtthread_t return_thread;

	BlockSignals();
	return_thread = *(schedule_queue->running->thread_block);
	UnblockSignals();
	return return_thread;
}


/* see man pthread_mutex(3); except init does not have the mutexattr parameter,
 * and should behave as if mutexattr is NULL (i.e., default attributes); also,
 * static initializers do not need to be implemented */
int  gtthread_mutex_init(gtthread_mutex_t *mutex){
	mutex->threadid = -1;
	return 0;
}
int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	BlockSignals();
	if(schedule_queue == NULL){
		fprintf(stderr,"\ngtthread_mutex_lock() Error: Did not initialize gtthread library with gtthread_init()\n");
		UnblockSignals();
		return -1;
	}
	else{
		while(mutex->threadid != -1){
			UnblockSignals();
			raise(SIGVTALRM);
			BlockSignals();
		}
		mutex->threadid = schedule_queue->running->thread_block->threadid;
		UnblockSignals();
		return 0;
	}
}
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
	BlockSignals();
	if(schedule_queue == NULL){
			fprintf(stderr,"\ngtthread_mutex_lock() Error: Did not initialize gtthread library with gtthread_init()\n");
			UnblockSignals();
			return -1;
	}
	else if(mutex->threadid == -1){
		UnblockSignals();
		return 0;
	}
	else if(schedule_queue->running->thread_block->threadid == mutex->threadid){
		mutex->threadid = -1;
		UnblockSignals();
		return 0;
	}
	else{
		fprintf(stderr,"\ngtthread_mutex_unlock() Error: Unlock by a different thread not permitted\n");
		return -1;
	}
}

/* gtthread_mutex_destroy() and gtthread_mutex_trylock() do not need to be
 * implemented */

void WrapperFunction(void*(*start_routine)(void *), void* arg){
	void *returnval;
	dlink_t* temp_dlink;

	returnval = start_routine(arg);

	BlockSignals();
	//Need to fire a new thread
	schedule_queue->running->thread_block->finished =1;
	schedule_queue->running->thread_block->returnval = returnval;
	temp_dlink = FindNode(schedule_queue,schedule_queue->running->thread_block->parent_threadid);
	if(temp_dlink == NULL){
		fprintf(stderr,"gtthread_exit() error: no parent found");
		exit(1);
	}
	else{
		temp_dlink->thread_block->child_count--;
		UnblockSignals();
		raise(SIGVTALRM);
	}
}

