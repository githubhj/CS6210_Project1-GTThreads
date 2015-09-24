/*
 * gtthread_sched.h
 *
 *  Created on: Jan 27, 2015
 *      Author: harshit
 */

#ifndef __GTTHREAD_SCHED_H
#define __GTTHREAD_SCHED_H
#include "gtthread.h"

void BlockSignals();

void UnblockSignals();

void InitTimer();

dlink_t* FindNode(volatile queue_t*, gtid_t);

int DelNode(queue_t*, gtid_t);

dlink_t* NextRunningLink(queue_t*);

int PushBack_Queue(dlink_t*, queue_t*);

int InitLink(dlink_t*, gtthread_t*, gtid_t, gtid_t);

#endif


