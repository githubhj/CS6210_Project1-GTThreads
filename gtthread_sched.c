#include "gtthread.h"
#include "gtthread_sched.h"
#include <sys/time.h>
/* Just an example. Nothing to see here. */


//Routine to block Alarm Signal
void BlockSignals(){
	sigprocmask(SIG_BLOCK, &signal_info, NULL);

}


//Routine to unblock alarm signal
void UnblockSignals(){
	sigprocmask(SIG_UNBLOCK, &signal_info, NULL);

}

//Routine to Initialize the timer with Global Period
void InitTimer(){
	struct itimerval timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = (suseconds_t)GLOBAL_PERIOD;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = (suseconds_t)GLOBAL_PERIOD;
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
}

//A routine to find out desired threadid node
dlink_t* FindNode(volatile queue_t* queue, gtid_t threadid){
	dlink_t* link;

	//Start Critical Section
	BlockSignals();

	if(queue->begin == NULL || queue->end == NULL){
		//End Critical Section
		UnblockSignals();
		return NULL;
	}
	else{
		link = queue->begin;
		while(link != NULL){
			if(link->thread_block->threadid == threadid){
				break;
			}
			else{
				link = link->prev;
			}
		}
		//End Critical Section
		UnblockSignals();
		return link;
	}

}

//A routine to delete a node
int DelNode(queue_t* queue, gtid_t threadid){
	dlink_t* link;

	link = FindNode(queue,threadid);
	if(link == NULL){
		return -1;
	}

	else{
		//Start Critical Section
		BlockSignals();

		if(link->next==NULL && queue->begin == link){
			queue->begin = link->prev;
			link->prev->next = NULL;
			link->next = link->prev = NULL;
			free(link->thread_block);
			free(link);
			queue->length--;
			//End Critical Section
			UnblockSignals();
			return 0;
		}
		else if(link->prev == NULL && queue->end == link){
			queue->end = link->next;
			link->next->prev = NULL;
			free(link->thread_block);
			free(link);
			queue->length--;
			//End Critical Section
			UnblockSignals();
			return 0;
		}
		else{
			link->prev->next = link->next;
			link->next->prev = link->prev;
			queue->length--;
			free(link->thread_block);
			free(link);
			//End Critical Section
			UnblockSignals();
			return 0;
		}
	}
}

//A routine to find the next running node
dlink_t* NextRunningLink(queue_t* queue){
	dlink_t* temp_link;
	//dlink_t *temp2_link;

	//Start Critical Section
	BlockSignals();

	temp_link = queue->running->prev;


	while(temp_link){
		if(!((temp_link->thread_block->cancelled) || (temp_link->thread_block->exited) || (temp_link->thread_block->finished))){
			//End Critical Section
			//InitTimer();
			UnblockSignals();
			return temp_link;
		}
		else{
			temp_link = temp_link->prev;
		}
	}

	temp_link = queue->begin;

	while(temp_link != queue->running){
		if(!((temp_link->thread_block->cancelled) || (temp_link->thread_block->exited) || (temp_link->thread_block->finished))){
			//InitTimer();
			UnblockSignals();
			return temp_link;
		}
		else{
			temp_link = temp_link->prev;
		}
	}

	if(queue->running->thread_block->cancelled||queue->running->thread_block->exited||queue->running->thread_block->finished){
		return NULL;
	}

	//End Critical Section
	//Initialize timer
	//InitTimer();
	UnblockSignals();
	return queue->running;
}

/*Initialize a Link:
 *Allocate memory to thread block
 *Allocate memory to thread context
 * */
int InitLink(dlink_t* link, gtthread_t *thread, gtid_t thread_id, gtid_t parent_id){

	link->next = NULL;
	link->prev = NULL;
	link->thread_block = thread;
	link->thread_block->threadid = thread_id;
	link->thread_block->parent_threadid = parent_id;
	link->thread_block->exited = 0;
	link->thread_block->finished=0;
	link->thread_block->cancelled=0;
	link->thread_block->runnning=0;
	link->thread_block->child_count = 0;
	link->thread_block->returnval = NULL;
	//link->thread_block->thread_context = (ucontext_t*)malloc(sizeof(ucontext_t));
	/*if(link->thread_block->thread_context == NULL){
			return -1;
	}*/
	link->thread_block->thread_context.uc_link = (ucontext_t*)malloc(sizeof(ucontext_t));
	if(link->thread_block->thread_context.uc_link == NULL){
		return -1;
	}
	else{
		return 0;
	}
}

/* Add a link to Queue
 * Make it a critical code
 * Turn off the timer
 * */
int PushBack_Queue(dlink_t* link, queue_t* local_queue){

	if(local_queue == NULL || link == NULL){
		return -1;
	}

	else{


		//Start Critical Section
		BlockSignals();

		/* CRITICAL SECTION
		 * Add a link to queue
		 *
		 * */

		//Only one link
		if(local_queue->begin==NULL && local_queue->end == NULL){
			local_queue->begin = link;
			local_queue->end = link;
			link->prev = NULL;
			link->next = NULL;
		}
		else{
			link->next = local_queue->end;
			local_queue->end->prev = link;
			local_queue->end = link;
			link->prev = NULL;
		}
		local_queue->length++;

		/* END OF CRITICAL SECTION
		 * */

		//End Critical Section
		UnblockSignals();

		return 0;
	}
}

void push_return_node(return_node_t** head_node, return_data_t new_data){

	return_node_t* new_node = (return_node_t*) malloc(sizeof(return_node_t));

	new_node->return_data  = new_data;

	new_node->next = (*head_node);

	(*head_node) = new_node;
}

int find_if_thread_returned(gtid_t threadid){
	if(head_return_list==NULL){
			return -1;
	}

	else{
		return_node_t* temp_node = head_return_list;
		while(temp_node){
			if(temp_node->return_data.threadid == threadid){
				return 0;
			}
			else{
				temp_node = temp_node->next;
			}
		}
		return -1;
	}
}

void* find_return_data(gtid_t threadid){
	if(head_return_list==NULL){
		return NULL;
	}

	else{
		return_node_t* temp_node = head_return_list;
		while(temp_node){
			if(temp_node->return_data.threadid == threadid){
				return temp_node->return_data.returnval;
			}
			else{
				temp_node = temp_node->next;
			}
		}
		return NULL;
	}
}




