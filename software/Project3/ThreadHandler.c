/*
 * ThreadHandler.c
 *
 *  Created on: Dec 12, 2015
 *      Author: Derek
 */

#include "ThreadHandler.h"
#include <sys/alt_alarm.h>
#include <nios2.h>
#include "alarm_handler.h"


static ThreadControlBlock* MainThread;
static ThreadControlBlock* RunningThread;
static LinkedList* lsActiveThreads;
static LinkedList* lsDoneThreads;


static uint32_t nThreadsCreated = 0;
char GetNewUniqueThreadIdentifierChar() {
	uint32_t n = nThreadsCreated;
	nThreadsCreated++;
	if (n < 10) return '0' + n;
	else if (n < 10 + 26) return 'A' + n - 10;
	else if (n < 10 + 26 + 26) return 'a' + n - 10 - 26;
	return 0;
}


void InitializeThreadHandler(){
	MainThread = malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(MainThread, "ThreadControlBlock");
	MainThread->threadName = "MainThread";
	MainThread->threadID = GetNewUniqueThreadIdentifierChar();
	MainThread->tstate = RUNNING;
	MainThread->stack = NULL;
	MainThread->sp = NULL;
	RunningThread = MainThread;
	lsActiveThreads = LinkedList_CreateNew(0);
	lsDoneThreads = LinkedList_CreateNew(0);
}


uint32_t GetActiveThreadCount(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	uint32_t threadCount = 1 + lsActiveThreads->count;
	//uint32_t threadCount = (RunningThread != NULL ? 1 : 0) + lsActiveThreads->count;
	CONDITIONALLY_ENABLE_INTERRUPTS
    return threadCount;
}

ThreadControlBlock* GetRunningThread(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock *thread = RunningThread;
	CONDITIONALLY_ENABLE_INTERRUPTS
	return thread;
}

ThreadControlBlock *CreateThread(uint32_t stackBytes, char* name, void (*funcptr)(char threadID)){
	ThreadControlBlock *thread	= malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(thread, "ThreadControlBlock");
	thread->threadName = name;
	thread->threadID = GetNewUniqueThreadIdentifierChar();
	thread->tstate = NEW;

    thread->stack = malloc(stackBytes);
    CHECKMALLOC(thread->stack, "thread->stack");

    thread->sp       = (uint32_t *)(thread->stack + stackBytes/4 - 19);
    thread->sp[18]   = (uint32_t)funcptr;							// ea
    thread->sp[17]   = 1;											// estatus
    thread->sp[5]    = thread->threadID;							// r4
    thread->sp[0]    = (uint32_t)CleanupThread;					// ra
    thread->sp[-1]   = (uint32_t)(thread->stack + stackBytes/4);	// fp

    return thread;
}



/* NEW ----> READY.  */
void StartThread(ThreadControlBlock *thread){
	CONDITIONALLY_DISABLE_INTERRUPTS
	if(thread != NULL && (thread->tstate == NEW || thread->tstate == READY || thread->tstate == BLOCKED)){
		if(thread->tstate == NEW){
			EnqueueElement(lsActiveThreads, thread);
		}
		else if(thread->tstate == BLOCKED ) {
			if(thread != RunningThread){
				//Move it to the front of the queue.
				ThreadControlBlock* removed = RemoveElement(lsActiveThreads,thread);
				if(removed != NULL){
					PushElement(lsActiveThreads, removed );
				} else{
					printf("!!!!!ROGUE THREAD!!!!!!!!");
					while(1){};
				}
			}
		}
		thread->tstate = READY;
	}
	else {
		printf("!!!!!ROGUE THREAD!!!!!!!!");
		//while(1){};
	}
	CONDITIONALLY_ENABLE_INTERRUPTS
}

/* READY --push into--> readyQ */
void JoinThread(ThreadControlBlock *thread){
	printf("Joining Thread %s_%c\n", thread->threadName, thread->threadID);

	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock *parentThread = GetRunningThread();
//	EnqueueElement(parentThread->joinedThreads, thread);
	parentThread->tstate = BLOCKED;
	CONDITIONALLY_ENABLE_INTERRUPTS
}

/* RUNNING ----> BLOCKED */
void BlockThread(ThreadControlBlock* thread){
	CONDITIONALLY_DISABLE_INTERRUPTS
	thread->tstate = BLOCKED;
	CONDITIONALLY_ENABLE_INTERRUPTS
	//forceInterrupt();
	//while(thread->tstate == BLOCKED){}
}



void TerminateThread(ThreadControlBlock *thread){
	char threadID = thread->threadID;
	thread->tstate = TERMINATED;
	RemoveElement(lsActiveThreads, thread);
	printf("Terminating thread %c\n", threadID);
}


void CleanupThread(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock* thread = GetRunningThread();
	char threadID = thread->threadID;
	thread->tstate = DONE;
	free(thread->stack);
	thread->stack = NULL;
	CONDITIONALLY_ENABLE_INTERRUPTS
	printf("Completing thread %c\n", threadID);

    while(1);
}

void ForfeitTimeSlot(ThreadControlBlock *thread){
	forceInterrupt();
}


ThreadControlBlock* GetNextThread(){
	ThreadControlBlock *readyThread = NULL;

	int i = lsActiveThreads->count;
	if(i > 0){
		node_t* node = Node_GetNodeAtIndex(lsActiveThreads, 0);
		ThreadControlBlock *nextThread = node->data;
		ThreadControlBlock *inactiveThread = NULL;

		while(node != NULL && nextThread != NULL && readyThread == NULL){
			if(nextThread->tstate == READY || nextThread->tstate == RUNNING) {
				readyThread = nextThread;
			} else if(nextThread->tstate == BLOCKED){
				if(i==1){
					nextThread->tstate == READY;
					readyThread = nextThread;
				}
			} else{
				inactiveThread = nextThread;
			}
			node = node->childNode;
			nextThread = node->data;

			if(inactiveThread != NULL){
				Node_InsertNode(lsDoneThreads, lsDoneThreads->count, Node_RemoveNode(lsActiveThreads, node->parentNode));
				inactiveThread = NULL;
			}
		}
	}
	return  readyThread;
}


void *ThreadScheduler(void *context){
	CONDITIONALLY_DISABLE_INTERRUPTS

	ThreadControlBlock *currentThread = RunningThread;
	ThreadControlBlock *nextThread = GetNextThread();;

	if(currentThread->tstate == READY || currentThread->tstate == RUNNING || currentThread->tstate == BLOCKED){
		currentThread->sp = (uint32_t *)context;

		if(nextThread != NULL){
			EnqueueElement(lsActiveThreads, currentThread);
			nextThread = RemoveElement(lsActiveThreads, nextThread);
		}
		else{
			nextThread = currentThread;
		}
	}
	else if(currentThread->tstate == DONE){
		TerminateThread(currentThread);
	}


	if(nextThread !=NULL){
		context = (void *)nextThread->sp;
		RunningThread = nextThread;
	}else if(currentThread == NULL){
		RunningThread = MainThread;
	}


	printf(" | %s_%c --> %s_%c | ", currentThread->threadName, currentThread->threadID, RunningThread->threadName, RunningThread->threadID);
	CONDITIONALLY_ENABLE_INTERRUPTS
	return context;
}


