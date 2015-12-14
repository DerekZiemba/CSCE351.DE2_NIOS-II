/*
 * Semaphore.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek Ziemba
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "LinkedList.h"
#include <stdint.h>
#include "ThreadHandler.h"

#define DEBUG_MESSAGES 1
#define SEM_MAX 10000

typedef struct mysem_t {
	ThreadControlBlock* LockingThread;
	ThreadQueue *lsBlockedThreads;
	int32_t count;
	char* name;
} MySem;

MySem* mysem_create(int32_t count, char* name);

void mysem_up(MySem* sem);
void mysem_down(MySem* sem);
void mysem_delete(MySem* sem);

int32_t mysem_waitCount(MySem* sem);
int32_t mysem_value(MySem* sem);

#endif /* SEMAPHORE_H_ */
