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
// You can use malloc() to allocate memory space.
MySem* mysem_create(int32_t count, char* name) {
	MySem  *sem = malloc(sizeof(MySem));
	sem->lsBlockedThreads = LinkedList_CreateNew(0);
	sem->count = count;
	sem->name = name;
	//printf("Semaphore created with initial count: %d\n", sem->count);
	return sem;
}

// semaphore's signal operation.
void mysem_up( MySem* sem ) {
	DISABLE_INTERRUPTS();
	sem->count = sem->count + 1;

	ThreadControlBlock *currentThread = GetRunningThread();
	printf("---> MySem_Up on %s semaphore By %s_%c, . Updated count: %d\n", sem->name,currentThread->threadName, currentThread->threadID, sem->count);

	if (sem->count > 0) {

		while(sem->lsBlockedThreads->count > 0){
			ThreadControlBlock *blockedThread = Dequeue(sem->lsBlockedThreads);
			StartThread(blockedThread);
			printf("-------> Semaphore %s Unblocked: %s_%c. Threads waiting: %lu\n",sem->name,blockedThread->threadName, blockedThread->threadID, sem->lsBlockedThreads->count);
		}
	}
	ENABLE_INTERRUPTS();
}

//semaphore's wait operation.
void mysem_down( MySem* sem ) {
	DISABLE_INTERRUPTS();
	ThreadControlBlock *currentThread = GetRunningThread();

	 //printf("Wait on semaphore. Updated count: %d\n", sem->count);
	printf("---> MySem_Down on %s semaphore By %s_%c. Updated count: %d\n", sem->name,currentThread->threadName, currentThread->threadID, sem->count);
	if(sem->count > 0) {
		sem->count = sem->count - 1;
		//sem->LockingThread = currentThread;
		ENABLE_INTERRUPTS();
	}
	else if (sem->count <= 0){
		if (sem->count < 0){
			printf("\n BAD THINGS HERE\n");
		}
		Enqueue(sem->lsBlockedThreads, currentThread);
		//printf("-------> Semaphore %s: BlockedID: %s_%c. Threads waiting: %lu\n",sem->name,currentThread->threadName, currentThread->threadID, sem->lsBlockedThreads->count);

		BlockThread(currentThread);

		//Make the locking thread a child of the current thread so that the lockign thread will finish.
		//mythread_join(currentThread, sem->LockingThread);
		//mythread_block(currentThread);

		ENABLE_INTERRUPTS();
		while (currentThread->tstate == BLOCKED){}
		mysem_down(sem);
	}

}

void mysem_delete( MySem* sem ) {
	if(sem != NULL){
		printf("Deleting semaphore.\n");
		DISABLE_INTERRUPTS();
//		free(sem->lsBlockedThreads);
//		free(sem);
//		sem = NULL;
		ENABLE_INTERRUPTS();
	}
}

int32_t mysem_waitCount(MySem* sem){
	return sem->lsBlockedThreads->count;
}

int32_t mysem_value(MySem* sem){
	return sem->count;
}


