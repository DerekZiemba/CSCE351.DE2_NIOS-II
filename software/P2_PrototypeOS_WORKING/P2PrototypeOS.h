/*
 * P2PrototypeOS.h
 *
 *  Created on: Nov 17, 2015
 *      Author: Derek
 */

#ifndef P2PROTOTYPEOS_H_
#define P2PROTOTYPEOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "sys/alt_alarm.h"
#include "sys/alt_stdio.h"
#include "alt_types.h"

#define RELATIVE_SPEED_MULTIPLIER 10
#define MAX 3000 * RELATIVE_SPEED_MULTIPLIER
#define MAIN_DELAY 15000 * RELATIVE_SPEED_MULTIPLIER
#define THREAD_DELAY 10 * RELATIVE_SPEED_MULTIPLIER
#define QUANTUM_LENGTH 500
#define NUM_THREADS 12
#define MAIN_THREAD_ID 1337
#define NULL_THREAD_ID -1
#define STACK_SIZE 16384//16384

#define SHOW_THREAD_STATS 1
#define SHOW_ITERRUPT_STATS 1

/***************************************************************************
* Boolean Definition
****************************************************************************/
#define FALSE 0
#define TRUE  !0
typedef enum bool { false = FALSE, true  = TRUE} bool;

/***************************************************************************
* Macros
****************************************************************************/
#define DISABLE_INTERRUPTS asm("wrctl status, zero");
#define ENABLE_INTERRUPTS asm("movi et, 1"); asm("wrctl status, et");

/* The resolution is 100ms but it accepts input in single millis */
#define ALARMTICKS(x) ((alt_ticks_per_second()*x)/1000)


/***********************************************************************
* Generic Linked List
***********************************************************************/
typedef struct node_t {
	void*			data;
	struct node_t*	parentNode;
	struct node_t*	childNode;
} node_t;

//void 	Node_Iterator(node_t *n, bool(*callback)(node_t* data, void* context), void *context);

typedef struct LinkedList {
	uint32_t	maxsize;
	uint32_t	count;
	node_t*		firstNode;
	node_t*		lastNode;
} LinkedList;

node_t *Node_init(void *data);

//Pass In 0 for unrestrained queue.
LinkedList*	LList_init(uint32_t max_size);

void 		LList_EnqueueGeneric(LinkedList *q, void* data) ;
void* 		LList_DequeueGeneric(LinkedList *q);
void* 		LList_PeekGeneric(LinkedList *q);

void 		LList_EnqueueNode(LinkedList *q, node_t* node);
node_t* 	LList_DequeueNode(LinkedList *q);
node_t* 	LList_PeekNode(LinkedList *q);
node_t* 	LList_LookupNode(LinkedList *q, node_t* (*callback) (node_t* node));

bool 		LList_IsFull(LinkedList *q);
bool 		LList_IsEmpty(LinkedList *q);
void 		LList_Print(LinkedList *q, char * format);


/***********************************************************************
* Thread Structures
***********************************************************************/
typedef enum threadStatus {  READY = 0, RUNNING = 1,  WAITING = 2,  DONE = 3} threadStatus;
#define numberOfThreadStatuses  4 // 6 queues:  Running, Ready, Waiting, Done

typedef struct ThreadControlBlock {
	alt_u32 thread_id;
	threadStatus scheduling_status;
	alt_u32 *context;
	alt_u32 *sp;
	alt_u32 *fp;
	alt_u32 blocking_id;
	alt_u32 totalTicks;
	alt_u32 lastStartTicks;
	alt_u32 startTicks;
} TCB;

//TCB *ThreadControlBlock_create(void (*start_routine)(int), int thread_id);


/***********************************************************************
* Thread Queue
***********************************************************************/
typedef LinkedList Queue;

typedef struct ThreadQueue {
	Queue	*(threadQueues[numberOfThreadStatuses]);
} ThreadQueue;

ThreadQueue* ThreadQueue_init();
alt_u32 ThreadCount(ThreadQueue *tq, threadStatus status);
void EnqueueThread(ThreadQueue *tq, threadStatus status, TCB *tcb);
TCB * DequeueThread(ThreadQueue *tq, threadStatus status);
TCB * PeekThread(ThreadQueue *tq, threadStatus status);
TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id);
TCB * PullThreadFromQueue(ThreadQueue *tq, threadStatus status, int thread_id);

/***********************************************************************
* Function Declarations
***********************************************************************/
void prototype_os(void);

alt_u32 myinterrupt_handler(void* context);
alt_u64 mythread_scheduler(alt_u64 param_list);
void mythread(alt_u32 thread_id);
TCB *mythread_create(void (*start_routine)(alt_u32), alt_u32 thread_id,  threadStatus status, int stacksizeBytes);
void mythread_join(alt_u32 thread_id);
void mythread_cleanup();

/***********************************************************************
* Called through Assembly
***********************************************************************/
static volatile int timer_interrupt_flag;
int check_timer_flag();
void set_timer_flag();
void reset_timer_flag();








#endif /* P2PROTOTYPEOS_H_ */
