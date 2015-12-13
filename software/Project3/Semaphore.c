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
#include "thread_handler.h"

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
	//printf("Signal semaphore. Updated count: %d\n", sem->count);

	if (sem->count > 0) {
		if (sem->lsBlockedThreads->count > 0){
			ThreadControlBlock *blockedThread = (ThreadControlBlock *) Dequeue(sem->lsBlockedThreads);
			mythread_start(blockedThread);
			ThreadControlBlock *thread = GetCurrentRunningThread();
			mythread_join(thread, blockedThread);
			printf("%s: Unblocked: %c, Threads waiting: %lu\n",sem->name, blockedThread->threadID, sem->lsBlockedThreads->count);
		}
	}
	ENABLE_INTERRUPTS();
}

//semaphore's wait operation.
void mysem_down( MySem* sem ) {
	DISABLE_INTERRUPTS();
	ThreadControlBlock *currentThread = GetCurrentRunningThread();

	// printf("Wait on semaphore. Updated count: %d\n", sem->count);
	if(sem->count > 0) {
		sem->count = sem->count - 1;
		ENABLE_INTERRUPTS();
	}
	else if (sem->count <= 0){
		if (sem->count < 0){
			printf("\n BAD THINGS HERE\n");
		}
		mythread_block(currentThread);
		Enqueue(sem->lsBlockedThreads, (void *)currentThread);
		printf("%s: BlockedID: %c, Threads waiting: %lu\n",sem->name, currentThread->threadID, sem->lsBlockedThreads->count);
		ENABLE_INTERRUPTS();
		while (currentThread->state == BLOCKED){}
		mysem_down(sem);
	}

}

void mysem_delete( MySem* sem ) {
	if(sem != NULL){
		printf("Deleting semaphore.\n");
		DISABLE_INTERRUPTS();
		free(sem->lsBlockedThreads);
		free(sem);
		sem = NULL;
		ENABLE_INTERRUPTS();
	}
}

int32_t mysem_waitCount(MySem* sem){
	return sem->lsBlockedThreads->count;
}

int32_t mysem_value(MySem* sem){
	return sem->count;
}


