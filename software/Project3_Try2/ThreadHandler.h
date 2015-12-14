/*
 * ThreadHandler.h
 *
 *  Created on: Dec 12, 2015
 *      Author: Derek
 */

#ifndef THREADHANDLER_H_
#define THREADHANDLER_H_



/* possible thread states */
enum ThreadState {NEW, READY, RUNNING, BLOCKED, DONE, NUM_TSTATES};
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <nios2.h>


/* thread control block */
typedef struct ThreadControlBlock {
    enum ThreadState  tstate;
    char* threadName;
    char threadID;
    uint32_t *sp;
    uint32_t *stack;
    struct ThreadControlBlock* parentThread;
    struct ThreadQueue*  joinedThreads;
} ThreadControlBlock;

typedef struct ThreadQueueNode_t {
	struct ThreadQueueNode_t*	parentNode;
	struct ThreadQueueNode_t*	childNode;
	ThreadControlBlock* data;
} ThreadQueueNode_t;


typedef struct ThreadQueue {
	int32_t	maxsize; //0 for unlimited
	int32_t	count;
	ThreadQueueNode_t*		firstNode;
	ThreadQueueNode_t*		lastNode;
} ThreadQueue;




void InitializeThreadHandler();

uint32_t GetActiveThreadCount();

ThreadControlBlock *CreateThread(uint32_t stack_size, char* name, void (*mythread)(char threadID));

ThreadControlBlock* GetRunningThread();

void StartThread(ThreadControlBlock *thread);

void JoinThread(ThreadControlBlock *thread);

void BlockThread(ThreadControlBlock *thread);

void CleanupThread();

void *ThreadScheduler(void *context);

/***************************************************************************
* ThreadQueue
****************************************************************************/


ThreadQueue* ThreadQueue_CreateNew(int32_t max_size);

void 					EnqueueThread(ThreadQueue* ls, ThreadControlBlock* thread);
ThreadControlBlock* 	DequeueThread(ThreadQueue* ls);
ThreadControlBlock* 	PeekNodeThread(ThreadQueue* ls);

/*Removes a thread no matter the position in queue*/
ThreadControlBlock* 	PullThreadByReference(ThreadQueue* ls,ThreadControlBlock* thread);

int32_t	GetThreadCound(ThreadQueue* ls);

#endif /* THREADHANDLER_H_ */
