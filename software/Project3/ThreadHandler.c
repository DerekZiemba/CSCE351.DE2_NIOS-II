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
//static LinkedList* lsBlockedThreads;


static uint32_t nThreadsCreated = 0;
char GetNewUniqueThreadIdentifierChar() {
	uint32_t n = nThreadsCreated;
	nThreadsCreated++;
	if (n < 10) return '0' + n;
	else if (n < 10 + 26) return 'A' + n - 10;
	else if (n < 10 + 26 + 26) return 'a' + n - 10 - 26;

	return 0;
}

uint32_t GetActiveThreadCount(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	uint32_t threadCount = 1 + lsActiveThreads->count + lsBlockedThreads->count;
	CONDITIONALLY_ENABLE_INTERRUPTS
    return threadCount;
}

void InitializeThreadHandler(){
	MainThread = malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(MainThread, "ThreadControlBlock");
	MainThread->threadName = "MainThread";
	MainThread->threadID = GetNewUniqueThreadIdentifierChar();
	MainThread->tstate = RUNNING;
	MainThread->parentThread = NULL;
	MainThread->joinedThreads = LinkedList_CreateNew(0);
	MainThread->stack = NULL;
	MainThread->sp = NULL;

	RunningThread = MainThread;

	lsActiveThreads = LinkedList_CreateNew(0);
	lsBlockedThreads = LinkedList_CreateNew(0);
	lsDoneThreads = LinkedList_CreateNew(0);
}


ThreadControlBlock *CreateThread(uint32_t stackBytes, char* name, void (*funcptr)(char threadID)){
	ThreadControlBlock *thread	= malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(thread, "ThreadControlBlock");
	thread->threadName = name;
	thread->threadID = GetNewUniqueThreadIdentifierChar();
	thread->tstate = NEW;
	thread->parentThread = NULL;
	thread->joinedThreads = LinkedList_CreateNew(0);

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


ThreadControlBlock* GetRunningThread(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock *thread = RunningThread;
	CONDITIONALLY_ENABLE_INTERRUPTS
	return thread;
}


/* NEW ----> READY */
void StartThread(ThreadControlBlock *thread){
	CONDITIONALLY_DISABLE_INTERRUPTS
	if(thread->tstate == NEW){
		thread->tstate = READY;
		EnqueueNode(lsActiveThreads, CreateNewNode(thread));
	}
	else if(thread->tstate == BLOCKED) {
		node_t* threadNode = GetNodeByElement(lsBlockedThreads, thread);
		node_t* threadNodeCheck = PullNode(lsBlockedThreads, threadNode);
		if(threadNode != threadNodeCheck){
			printf("!!!!!!!!Rogue Thread!!!!!!!");
		}
		thread->tstate = READY;
		//This thread was just unblocked so it will jump the queue
		InsertNode(lsActiveThreads, 0, threadNode);
	}
	else if(thread->tstate == READY) {
		//Do nothing, it is already in the ReadyQueue.
	}
	else {
		printf("This thread CANNOT BE STARTED!!!");
	}

	CONDITIONALLY_ENABLE_INTERRUPTS
}

/* READY --push into--> readyQ */
void JoinThread(ThreadControlBlock *thread){
	CONDITIONALLY_DISABLE_INTERRUPTS

	ThreadControlBlock *parentThread = GetRunningThread();
	EnqueueNode(parentThread->joinedThreads, CreateNewNode(thread));
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


/* Only Call from thread scheduler. Scheduler calls this when the CurrentRunning Thread is marked DONE*/
void TerminateThread(ThreadControlBlock *thread){
	char threadID = thread->threadID;
	if(thread->joinedThreads->count > 0) {
		printf("ERROR!!!!  THREAD WITH CHILD THREADS WAS TERMINATED!");
	}
	free(thread->stack);
	thread->stack = NULL;
//	free(thread->joinedThreads);
//	thread->joinedThreads = NULL;
//	thread->parentThread = NULL;
	printf("Terminating thread %c\n", threadID);
}


void CleanupThread(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock* thread = GetRunningThread();
	char threadID = thread->threadID;
	thread->tstate = DONE;

	CONDITIONALLY_ENABLE_INTERRUPTS

	printf("Completing thread %c\n", threadID);
//	forceInterrupt();

//	uint32_t stackptr;
//	NIOS2_READ_SP(stackptr);
	//SwapThreadQueue(thread, DONE);
    //free(thread);
    //thread = NULL;
    while(1);
}

ThreadControlBlock* SortThreadList(ThreadControlBlock* currentThread){

}

ThreadControlBlock* GetNextThread(ThreadControlBlock* currentThread){
	ThreadControlBlock *nextThread = NULL;
	ThreadControlBlock *temp = NULL;

	if(currentThread->tstate == BLOCKED){
		if(currentThread->joinedThreads->count > 0){

			temp = DequeueValue(currentThread->joinedThreads);
			nextThread = PullElementByReference(lsActiveThreads, temp);
			if(nextThread==NULL){
				nextThread = PullElementByReference(lsBlockedThreads, temp);
			}

			if(nextThread!=NULL){
				InsertNode(lsActiveThreads, 0, CreateNewNode(nextThread));
			}
		}
	}

	if(lsActiveThreads->count > 0){
		nextThread = DequeueValue(lsActiveThreads);

	} else if(lsBlockedThreads->count > 0){
		nextThread = DequeueValue(lsBlockedThreads);
	}else{
		printf("Problem");
	}
	return nextThread;
}


void *ThreadScheduler(void *context){
	CONDITIONALLY_DISABLE_INTERRUPTS
	ThreadControlBlock *nextThread;
	ThreadControlBlock *currentThread = GetRunningThread();
	currentThread->sp = (uint32_t *)context;

	uint32_t count =  GetActiveThreadCount();

	if( currentThread->tstate == BLOCKED ){
		if(count > 1){
			PrintThreadMessage("Moved %s_%c To BLOCKED List\n",currentThread->threadName, currentThread->threadID);
			EnqueueValue(lsBlockedThreads, currentThread);
		}else{
			return context;
		}

	} else {
		 if(currentThread->tstate == READY){
			//PrintThreadMessage("Moved %s_%c To READY List\n",currentThread->threadName, currentThread->threadID);
			EnqueueValue(lsActiveThreads, currentThread);
		}
		else if(currentThread->tstate == RUNNING){
			PrintThreadMessage("Moved %s_%c To READY List\n",currentThread->threadName, currentThread->threadID);
			currentThread->tstate = READY;
			EnqueueValue(lsActiveThreads, currentThread);
		}
		else if( currentThread->tstate == DONE){
			PrintThreadMessage("Moved %s_%c To DONE List\n",currentThread->threadName, currentThread->threadID);
			EnqueueValue(lsDoneThreads, currentThread);
			TerminateThread(currentThread);
		}
	}

	nextThread = GetNextThread(currentThread);
	context = (void *)nextThread->sp;
	RunningThread = nextThread;

	CONDITIONALLY_ENABLE_INTERRUPTS
	return context;
}



