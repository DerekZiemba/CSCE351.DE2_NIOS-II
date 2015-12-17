/*
 * Semaphore.c
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek Ziemba
 */
#include "prototypeOS.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "Semaphore.h"


// It returns the starting address a semaphore variable.
MySem* mysem_create(int32_t count, char* name) {
	MySem  *sem = malloc(sizeof(MySem));
	sem->lsBlockedThreads = LinkedList_CreateNew(0);
	sem->count = count;
	sem->name = name;
	return sem;
}

// semaphore's signal operation.
void mysem_up( MySem* sem ) {
	sem->count = sem->count + 1;
	//ThreadControlBlock *currentThread = GetRunningThread();
	//printf("---> MySem_Up on %s semaphore By %s_%c, . Updated count: %d\n", sem->name,currentThread->threadName, currentThread->threadID, sem->count);

	if (sem->count > 0) {

		while(sem->lsBlockedThreads->count > 0){
			ThreadControlBlock *blockedThread = DequeueElement(sem->lsBlockedThreads);
			StartThread(blockedThread);
			printf("\n-------> Semaphore %s Unblocked: %s_%c. Threads waiting: %lu  ",sem->name,blockedThread->threadName, blockedThread->threadID, sem->lsBlockedThreads->count);
		}
	}
}

//semaphore's wait operation.
void mysem_down( MySem* sem ) {

	ThreadControlBlock *currentThread = GetRunningThread();
	 //printf("Wait on semaphore. Updated count: %d\n", sem->count);
	//printf("---> MySem_Down on %s semaphore By %s_%c. Updated count: %d\n", sem->name,currentThread->threadName, currentThread->threadID, sem->count);
	if(sem->count > 0) {
		sem->count = sem->count - 1;
	}
	else if (sem->count <= 0){
		if (sem->count < 0){
			printf("\n BAD THINGS !!!!\n");
		}

		EnqueueElement(sem->lsBlockedThreads, currentThread);
		printf("\n-------> Semaphore %s: BlockedID: %s_%c. Threads waiting: %lu  ",sem->name,currentThread->threadName, currentThread->threadID, sem->lsBlockedThreads->count);

		BlockThread(currentThread);

		while (currentThread->tstate == BLOCKED){
			//I tried to make it trigger an interrupt that allowed the thread handler to run, but it just results in bad things...
			//FORFEIT_TIME_SLICE();
		}
		mysem_down(sem);
	}


}

void mysem_delete( MySem* sem ) {
	if(sem != NULL){
		printf("\nDeleting semaphore.  ");
		free(sem->lsBlockedThreads);
		free(sem);
		sem = NULL;
	}
}

int32_t mysem_waitCount(MySem* sem){
	return sem->lsBlockedThreads->count;
}

int32_t mysem_value(MySem* sem){
	return sem->count;
}


