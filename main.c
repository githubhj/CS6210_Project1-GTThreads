/*
 * main.c
 *
 *  Created on: Jan 19, 2015
 *      Author: harshit
 */
#include "gtthread.h"
#include <stdio.h>
#include <time.h>

#define CHOPSTICK_NUM 5
#define PHILOSOPHER_NUM 5

gtthread_mutex_t chopstick[CHOPSTICK_NUM];
gtthread_t philosphers[PHILOSOPHER_NUM];

void* philosopher_thread(void *arg){

	int myid = *(int*)arg;
	int j;

	switch(myid){
	case 1:
		while(1){
			printf("Philosopher %d: Trying to take Chopstick No. 1\n",myid);
			gtthread_mutex_lock(&chopstick[0]);
			printf("Philosopher %d: Got Chopstick No. 1\n",myid);
			printf("Philosopher %d: Trying to take Chopstick No. 2\n",myid);
			gtthread_mutex_lock(&chopstick[1]);
			printf("Philosopher %d: Got Chopstick No. 2\n",myid);
			printf("Philosopher %d: Eating with Chopstick No. 1 & 2\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			printf("Philosopher %d: Releasing Chopstick No. 1\n",myid);
			gtthread_mutex_unlock(&chopstick[0]);
			printf("Philosopher %d: Releasing Chopstick No. 2\n",myid);
			gtthread_mutex_unlock(&chopstick[1]);
			printf("Philosopher %d: Thinking....\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
		}
		break;
	case 2:
		while(1){
			printf("Philosopher %d: Trying to take Chopstick No. 3\n",myid);
			gtthread_mutex_lock(&chopstick[2]);
			printf("Philosopher %d: Got Chopstick No. 3\n",myid);
			printf("Philosopher %d: Trying to take Chopstick No. 2\n",myid);
			gtthread_mutex_lock(&chopstick[1]);
			printf("Philosopher %d: Got Chopstick No. 2\n",myid);
			printf("Philosopher %d: Eating with Chopstick No. 2 & 3\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			printf("Philosopher %d: Releasing Chopstick No. 3\n",myid);
			gtthread_mutex_unlock(&chopstick[2]);
			printf("Philosopher %d: Releasing Chopstick No. 2\n",myid);
			gtthread_mutex_unlock(&chopstick[1]);
			printf("Philosopher %d: Thinking....\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
				}
		break;
	case 3:
		while(1){
			printf("Philosopher %d: Trying to take Chopstick No. 4\n",myid);
			gtthread_mutex_lock(&chopstick[3]);
			printf("Philosopher %d: Got Chopstick No. 4\n",myid);
			printf("Philosopher %d: Trying to take Chopstick No. 3\n",myid);
			gtthread_mutex_lock(&chopstick[2]);
			printf("Philosopher %d: Got Chopstick No. 3\n",myid);
			printf("Philosopher %d: Eating with Chopstick No. 3 & 4\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			printf("Philosopher %d: Releasing Chopstick No. 4\n",myid);
			gtthread_mutex_unlock(&chopstick[3]);
			printf("Philosopher %d: Releasing Chopstick No. 3\n",myid);
			gtthread_mutex_unlock(&chopstick[2]);
			printf("Philosopher %d: Thinking....\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			}
		break;
	case 4:
		while(1){
			printf("Philosopher %d: Trying to take Chopstick No. 5\n",myid);
			gtthread_mutex_lock(&chopstick[4]);
			printf("Philosopher %d: Got Chopstick No. 5\n",myid);
			printf("Philosopher %d: Trying to take Chopstick No. 4\n",myid);
			gtthread_mutex_lock(&chopstick[3]);
			printf("Philosopher %d: Got Chopstick No. 4\n",myid);
			printf("Philosopher %d: Eating with Chopstick No. 4 & 5\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			printf("Philosopher %d: Releasing Chopstick No. 5\n",myid);
			gtthread_mutex_unlock(&chopstick[4]);
			printf("Philosopher %d: Releasing Chopstick No. 4\n",myid);
			gtthread_mutex_unlock(&chopstick[3]);
			printf("Philosopher %d: Thinking....\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			}
		break;
	case 5:
		while(1){
			printf("Philosopher %d: Trying to take Chopstick No. 1\n",myid);
			gtthread_mutex_lock(&chopstick[0]);
			printf("Philosopher %d: Got Chopstick No. 1\n",myid);
			printf("Philosopher %d: Trying to take Chopstick No. 5\n",myid);
			gtthread_mutex_lock(&chopstick[4]);
			printf("Philosopher %d: Got Chopstick No. 5\n",myid);
			printf("Philosopher %d: Eating with Chopstick No. 1 & 5\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			printf("Philosopher %d: Releasing Chopstick No. 1\n",myid);
			gtthread_mutex_unlock(&chopstick[0]);
			printf("Philosopher %d: Releasing Chopstick No. 5\n",myid);
			gtthread_mutex_unlock(&chopstick[4]);
			printf("Philosopher %d: Thinking....\n",myid);
			gtthread_yield();
			for(j=0; j < rand() % 999999; ++j);
			}
		break;
	default:
		fprintf(stderr,"Philosopher Thread Error: Wrong thread ID\n");
		break;
	}

}

int main(){

	//Local Variables
	int i,t[]={1,2,3,4,5};

	//Initialize the scheduler
	gtthread_init(1000);

	//Initialize the mutex
	for(i=0;i<CHOPSTICK_NUM;i++){
		gtthread_mutex_init(&chopstick[i]);
	}

	//Create Threads
	for(i=1;i<PHILOSOPHER_NUM+1;i++){
		gtthread_create(&philosphers[i-1],philosopher_thread,(void*)&t[i-1]);
	}

	//Loop Forever
	for(;;);

}
