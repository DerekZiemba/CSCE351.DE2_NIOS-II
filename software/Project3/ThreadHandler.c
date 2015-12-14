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


//static ThreadControlBlock *current_running_thread      = NULL;
//
//static uint32_t *main_stack_pointer = NULL;
//
//static LinkedList lsReadyThreads = {0,0,NULL,NULL};

//static ThreadControlBlock MainThread = {
//		RUNNING, 			//state
//		'Z',				//threadID
//		NULL, 				//Stack Pointer
//		NULL, 				//Stack (Null for main thread),
//		NULL, 				//parentThread (Null for main thread)
//		{0,0,NULL,NULL} 	//childThreads (Threads that were joined to main thread)  These must complete before Main thread can execute
//};

//static node_t* RunningThreadNode = &{ &MainThread, 0, 0};
//static LinkedList* lsReadyThreads = &{0,0,NULL,NULL};
//static LinkedList* lsBlockedThreads = &{0, 0, NULL, NULL};
//static LinkedList* lsDoneThreads = &{0, 0, NULL, NULL};

static ThreadControlBlock* MainThread;
static node_t* RunningThreadNode;
static LinkedList* lsReadyThreads;
static LinkedList* lsBlockedThreads;
static LinkedList* lsDoneThreads;

static uint32_t nThreadsCreated = 0;
char GetNewUniqueThreadIdentifierChar() {
	uint32_t n = nThreadsCreated;
	if (n < 10) return '0' + n;
	else if (n < 10 + 26) return 'A' + n - 10;
	else if (n < 10 + 26 + 26) return 'a' + n - 10 - 26;
	nThreadsCreated++;
	return 0;
}

uint32_t GetActiveThreadCount(){
	CONDITIONALLY_DISABLE_INTERRUPTS
	uint32_t threadCount = 1 + lsReadyThreads->count + lsBlockedThreads->count;
	CONDITIONALLY_ENABLE_INTERRUPTS
    return threadCount;
}

void InitializeThreadHandler(){
	MainThread = malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(MainThread, "ThreadControlBlock");
	MainThread->threadID = GetNewUniqueThreadIdentifierChar();
	MainThread->tstate = RUNNING;
	MainThread->parentThread = NULL;
	MainThread->joinedThreads = LinkedList_CreateNew(0);
	MainThread->stack = NULL;
	MainThread->sp = NULL;

	RunningThreadNode = CreateNewNode(MainThread);

	lsReadyThreads = LinkedList_CreateNew(0);
	lsBlockedThreads = LinkedList_CreateNew(0);
	lsDoneThreads = LinkedList_CreateNew(0);
}


ThreadControlBlock *CreateThread(uint32_t stackBytes, void (*funcptr)(char threadID)){
	ThreadControlBlock *thread	= malloc(sizeof(ThreadControlBlock));
	CHECKMALLOC(thread, "ThreadControlBlock");
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
	ThreadControlBlock *thread = RunningThreadNode->data;
	CONDITIONALLY_ENABLE_INTERRUPTS
	return thread;
}

/* NEW ----> READY */
void StartThread(ThreadControlBlock *thread){
	CONDITIONALLY_DISABLE_INTERRUPTS
	if(thread->tstate == NEW){
		thread->tstate = READY;
		EnqueueNode(lsReadyThreads, CreateNewNode(thread));
	}
	else if(thread->tstate == BLOCKED) {
		node_t* threadNode = GetNodeByElement(lsBlockedThreads, thread);
		node_t* threadNodeCheck = PullNode(lsBlockedThreads, threadNode);
		if(threadNode != threadNodeCheck){
			printf("!!!!!!!!Rogue Thread!!!!!!!");
		}
		EnqueueNode(lsReadyThreads, threadNode);
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
	BlockThread(parentThread);


	CONDITIONALLY_ENABLE_INTERRUPTS
}

/* RUNNING ----> BLOCKED */
void BlockThread(ThreadControlBlock* thread){
	CONDITIONALLY_DISABLE_INTERRUPTS
	thread->tstate = BLOCKED;
	CONDITIONALLY_ENABLE_INTERRUPTS
	forceInterrupt();
	//while(thread->tstate == BLOCKED){}
}


/* Only Call from thread scheduler. Scheduler calls this when the CurrentRunning Thread is marked DONE*/
void TerminateThread(ThreadControlBlock *thread){
	char threadID = thread->threadID;
	if(thread->joinedThreads->count > 0) {
		printf("ERROR!!!!  THREAD WITH CHILD THREADS WAS TERMINATED!");
	}
	free(thread->joinedThreads);
	thread->joinedThreads = NULL;
	thread->parentThread = NULL;
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
	forceInterrupt();

//	uint32_t stackptr;
//	NIOS2_READ_SP(stackptr);
	//SwapThreadQueue(thread, DONE);
    //free(thread);
    //thread = NULL;
    while(1);
}



void *ThreadScheduler(void *context){
	CONDITIONALLY_DISABLE_INTERRUPTS

	ThreadControlBlock *currentThread = GetRunningThread();
	currentThread->sp = (uint32_t *)context;

	if(currentThread->tstate == READY){
		PrintThreadMessage("Moved Running Thread %c To READY List", currentThread->threadID);
		currentThread->tstate = READY;
		EnqueueNode(lsReadyThreads, RunningThreadNode);
	}
	else if(currentThread->tstate == RUNNING){
		PrintThreadMessage("Moved Running Thread %c To READY List", currentThread->threadID);
		currentThread->tstate = READY;
		EnqueueNode(lsReadyThreads, RunningThreadNode);
	}
	else if( currentThread->tstate == BLOCKED ){
		PrintThreadMessage("Moved Thread %c To BLOCKED List", currentThread->threadID);
		EnqueueNode(lsBlockedThreads, RunningThreadNode);
	}
	else if( currentThread->tstate == DONE){
		PrintThreadMessage("Moved Thread %c To DONE List", currentThread->threadID);
		EnqueueNode(lsDoneThreads, RunningThreadNode);
		TerminateThread(currentThread);
	}

	int readyThreads = lsReadyThreads->count;
	if(readyThreads == 0){
		printf("No Ready Threads.  DEAAADDDDLOOOOOCKKCKKKKK");
	}

	RunningThreadNode = DequeueNode(lsReadyThreads);
	ThreadControlBlock *nextThread = GetRunningThread();
	context = (void *)nextThread->sp;

	CONDITIONALLY_ENABLE_INTERRUPTS
	return context;
}


//void *mythread_schedule(void *context){
//	ThreadControlBlock *currentThread;
//	ThreadControlBlock *nextThread;
//
//	CONDITIONALLY_DISABLE_INTERRUPTS
//	if(MainThread.sp == NULL){
//		MainThread->sp = (uint32_t *)context;
//		return (void *)(MainThread->sp);
//	}
//
//	currentThread = DequeueThread(MainThread->lsChildThreads[RUNNING]);
//
//
//	currentThread->sp = (uint32_t *)context;
//	SwapThreadQueue(currentThread, currentThread->tstate);
//
//	uint32_t readyCount = currentThread->lsChildThreads[READY];
//	uint32_t blockedCount = currentThread->lsChildThreads[BLOCKED];
//
//	while(readyCount > 0){
//
//	if(currentThread->state == BLOCKED){
//
//	}
//
//
//		ThreadControlBlock *nextThread;
//		if(currentThread->lsJoinedThreads->count > 0) {
//			nextThread = Dequeue(currentThread->lsJoinedThreads);
//			while()
//		}
//
//		 Peek(&ThreadHandlerQueues[READY]);
//
//
//	    if (ThreadHandler.threadStateQueues[READY].count > 0){
//	        if (currentThread != NULL){
//	            if(currentThread->tstate == RUNNING){
//	            	currentThread->tstate = READY;
//	            }
//	            currentThread->sp = (uint32_t *)context;
//	            Enqueue(&lsReadyThreads, (void*) currentThread);
//	        }
//	        else if (main_stack_pointer == NULL){
//	            main_stack_pointer = (uint32_t *)context;
//	        }
//
//	        currentThread = (ThreadControlBlock *)Dequeue(&lsReadyThreads);
//	        int allAreBlockedCounter = lsReadyThreads.count;
//	        while(currentThread->tstate == BLOCKED){
//	        	if (allAreBlockedCounter < 0) {
//	        		printf("we have some problems");
//	        	}
//	        	Enqueue(&lsReadyThreads, (void*) currentThread);
//	        	currentThread = (ThreadControlBlock *)Dequeue(&lsReadyThreads);
//	        	allAreBlockedCounter--;
//	        }
//	        currentThread->state = RUNNING;
//
//	        context = (void *)(currentThread->sp);
//	    }
//	    else if (currentThread==NULL && main_stack_pointer!=NULL){
//	        context = (void *)main_stack_pointer;
//	    }
//
//
//
//	CONDITIONALLY_ENABLE_INTERRUPTS
//    return context;
//}




