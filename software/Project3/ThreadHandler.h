/*
 * ThreadHandler.h
 *
 *  Created on: Dec 12, 2015
 *      Author: Derek
 */

#ifndef THREADHANDLER_H_
#define THREADHANDLER_H_

#include "prototypeOS.h"
#include "LinkedList.h"

#define FORFEIT_TIME_SLICE() {		\
	uint32_t sp; 					\
	NIOS2_READ_SP(sp); 				\
	ThreadScheduler((void*)sp);		\
}

/* possible thread states */
enum ThreadState {NEW, READY, RUNNING, BLOCKED, DONE, TERMINATED, NUM_TSTATES};


/* thread control block */
typedef struct ThreadControlBlock {
    enum ThreadState  tstate;
    /*The number of threads blocked on this thread.
     * Thread scheduler will take this number into account
     * relative to the total system threads then assign a priority.
     * If this thread is blocking 4 out of 10 threads it will get 4/10th second scheduled time.*/
    int8_t blockingThreads;
    char* threadName;
    char threadID;
    uint32_t *sp;
    uint32_t *stack;
} ThreadControlBlock;


void InitializeThreadHandler();

uint32_t GetActiveThreadCount();

ThreadControlBlock *CreateThread(uint32_t stack_size, char* name, void (*mythread)(char threadID));

ThreadControlBlock* GetRunningThread();

void StartThread(ThreadControlBlock *thread);

void JoinThread(ThreadControlBlock *thread);

void BlockThread(ThreadControlBlock *thread);

void CleanupThread();

void *ThreadScheduler(void *context);

void ForfeitTimeSlot(ThreadControlBlock *thread);

#endif /* THREADHANDLER_H_ */
