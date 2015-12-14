#include "P2PrototypeOS.h"
#include <nios2.h>
/***********************************************************************
* Variables
***********************************************************************/
/* Note: Made it a variable so I can change it in the debugger.  */
static alt_alarm alarm;
static ThreadQueue *threads;
static volatile uint32_t g_tickCounter;

static volatile char strBuff[200]; //I'd rather not use stack space.

uint8_t bCheckInterruptsEnabled(){
	uint8_t status = 0;
	NIOS2_READ_STATUS(status);
	return status;
}

/*NOTE:  Interrupts are enabled/disabled in the assembly that calls this function*/
TCB *CreateThread(void (*funcptr)(uint32_t), uint32_t threadID,  threadStatus status, int stackBytes) {
	TCB *thread = malloc(sizeof(TCB));
	thread->thread_id = threadID;
	thread->blocking_id = -1;
	thread->scheduling_status = status;
	thread->totalTicks = 0;
	thread->lastStartTicks = g_tickCounter;
	thread->startTicks = g_tickCounter;
	thread->stack = malloc(stackBytes);

	thread->stackSize = stackBytes;
	thread->fp 		 = (uint32_t)thread->sp[-1];	// fp

    thread->sp       = (uint32_t *)(thread->stack + stackBytes/4 - 19);
    thread->sp[18]   = (uint32_t)funcptr;							// ea
    thread->sp[17]   = 1;											// estatus
    thread->sp[5]    = threadID;									// r4
    thread->sp[0]    = (uint32_t)CleanupThread;					// ra
    thread->sp[-1]   = (uint32_t)(thread->stack + stackBytes/4);	// fp


	return thread;
}

void* mythread_scheduler(void* context){ // stack pointer
	DISABLE_INTERRUPTS();
	TCB *thisThread = NULL;
	TCB *nextThread = NULL;

//	uint32_t * param_ptr = 	&context;
//	uint32_t stackpointer =  *param_ptr; //Returns in register r4
//	uint32_t framepointer =  *(param_ptr+1); //Returns in register r5
////
//	uint64_t returnValue;
//	uint32_t * retptr = &returnValue;

	alt_u16 nThreadsRunning = ThreadCount(threads, RUNNING);
	alt_u16 nThreadsReady = ThreadCount(threads, READY);
	alt_u16 nThreadsWaiting = ThreadCount(threads,WAITING);
	alt_u16 nThreadsDone = ThreadCount(threads, DONE);

	sprintf(strBuff, "---------> Running=%d | Ready=%d | Waiting=%d | Done=%d", nThreadsRunning, nThreadsReady, nThreadsWaiting, nThreadsDone);


	if(nThreadsRunning == 0 && (nThreadsReady != 0 || nThreadsWaiting != 0)){
		//Main Thread
		thisThread = malloc(sizeof(TCB));
		thisThread->thread_id = MAIN_THREAD_ID;
		thisThread->scheduling_status = RUNNING;
		thisThread->sp = (uint32_t*)context;
		thisThread->stack = (uint32_t*)context;
		//thisThread->fp = framepointer;
		thisThread->totalTicks = g_tickCounter;
		thisThread->lastStartTicks = g_tickCounter;
		thisThread->startTicks = 0;
		EnqueueThread(threads, thisThread->scheduling_status, thisThread);
		nThreadsRunning = ThreadCount(threads,RUNNING);
	}

	if(nThreadsReady > 0 || nThreadsWaiting > 0){
		thisThread = DequeueThread(threads, RUNNING);
		thisThread->totalTicks += g_tickCounter - thisThread->lastStartTicks;


		if(thisThread->scheduling_status == DONE){
			EnqueueThread(threads, thisThread->scheduling_status , thisThread); //Should be set by thread cleanup
			//Here was the phantom thread bug.  There's nothing in the ready queue.
			//nextThread = DequeueThread(threads, READY);
			nextThread =  DequeueThread(threads, nThreadsReady > 0 ? READY : WAITING);
		}
		else{
			thisThread->sp = (uint32_t*)context;
		//	thisThread->sp = stackpointer;
		//	thisThread->fp = framepointer;

			int32_t stackpointer = thisThread->sp;
			int32_t stackEnd = thisThread->stack;
			int32_t stackSize = thisThread->stackSize;
			int32_t bytesFree = stackpointer - stackEnd;
			int32_t stackBytesConsumption =  stackSize - bytesFree;
			sprintf(strBuff + strlen(strBuff), " | Stack Bytes Consumption= %d", stackBytesConsumption);

			if(thisThread->scheduling_status != WAITING){
				thisThread->scheduling_status = READY;
			}
			EnqueueThread(threads, thisThread->scheduling_status, thisThread);
			nextThread =  DequeueThread(threads, nThreadsReady > 0 ? READY : WAITING);
		}

		nextThread->scheduling_status = RUNNING;
		nextThread->lastStartTicks = g_tickCounter;
		EnqueueThread(threads, nextThread->scheduling_status, nextThread);

//		*(retptr) = nextThread->sp;
//		*(retptr + 1) = nextThread->fp;
		context = (void*) nextThread->sp;
		sprintf(strBuff + strlen(strBuff), " | Queueing=%lu | Scheduling=%lu", thisThread->thread_id, nextThread->thread_id);
	}
	else {

		TCB *temp = DequeueThread(threads, RUNNING);
		EnqueueThread(threads, temp->scheduling_status, temp);
		sprintf(strBuff + strlen(strBuff), " | No Queued Threads");
	}

#if SHOW_THREAD_STATS == 1
	strcat(strBuff,"\n");
	printf(strBuff);
	memset(strBuff, 0, sizeof(strBuff));
#endif

	ENABLE_INTERRUPTS();
	return context;
}



TCB* GetRunningThread(){
	DISABLE_INTERRUPTS();
	TCB* thread = PeekThread(threads, RUNNING);
	ENABLE_INTERRUPTS();
	return thread;
}

// Joins the thread with the calling thread
void mythread_join(uint32_t blockingThreadID){
	alt_32 i=0;
	TCB *runningThread;
	TCB *blockingThread;

	DISABLE_INTERRUPTS();
	// Wait for timer the first time
	runningThread = PeekThread(threads,RUNNING);
	while (runningThread == NULL){
		ENABLE_INTERRUPTS();
		for (i = 0 ; i < THREAD_DELAY; i++){asm("nop");};
		DISABLE_INTERRUPTS();
		runningThread = PeekThread(threads,RUNNING);
	}

	printf("Joining ThreadID=%lu ", blockingThreadID);

	if(runningThread->thread_id==blockingThreadID && runningThread->scheduling_status != DONE){
		ENABLE_INTERRUPTS();
		while (runningThread->scheduling_status != DONE){
			for (i = 0 ; i < THREAD_DELAY; i++){};
		}
	} else{
		blockingThread = LookupThread(threads, READY, blockingThreadID);
		if(blockingThread==NULL){
			blockingThread = LookupThread(threads, DONE, blockingThreadID);
			ENABLE_INTERRUPTS();
			if(blockingThread!=NULL){
				printf("Thread has already finished: %lu \n", blockingThreadID);
			} else{
				printf("ERROR: COULD NOT FIND THREAD TO JOIN WITH ID: %lu \n", blockingThreadID);
			}

		} else{
			runningThread->scheduling_status = WAITING;
			runningThread->blocking_id =blockingThreadID;
			ENABLE_INTERRUPTS();
			alt_printf("... Has Been Joined \n");
			while (blockingThread->scheduling_status != DONE){
				for (i = 0 ; i < THREAD_DELAY; i++){};
			}
		}

	}
}

// Threads return here and space is freed
void CleanupThread(){
	// Unblock thread blocked by join
	DISABLE_INTERRUPTS();

	TCB *thisThread = PeekThread(threads,RUNNING);
	free(thisThread->stack);
	thisThread->scheduling_status = DONE;
	thisThread->totalTicks += g_tickCounter - thisThread->lastStartTicks;
	uint32_t totalMillis = thisThread->totalTicks * 10;
	printf("........COMPLETED........ ThreadID=%lu.  Total Runtime=%lums.  Of a total time pool of %lu milliseconds \n", thisThread->thread_id, totalMillis, (g_tickCounter - thisThread->startTicks)*10);

	ENABLE_INTERRUPTS();
	while(TRUE);
}


/*********************************************************************
* Prototype OS
****************************************************************************/
void prototype_os(void) {
	printf("Started.%lu\n", ALARMTICKS(QUANTUM_LENGTH));
	uint32_t i, j, iterations = 0;

	// Here: initialize the timer and its interrupt handler
	alt_alarm_start(&alarm, ALARMTICKS(QUANTUM_LENGTH), myinterrupt_handler, (void*)(&iterations));

	threads = ThreadQueue_init();

	for (i = 0; i < NUM_THREADS; i++){
		TCB *tcb = CreateThread(&mythread, i, READY, STACK_SIZE);
		EnqueueThread(threads, READY, tcb );
		printf("Finished creation (%lu): sp: 0x%x\n", i, tcb->stack);
	}

	for (i = 0; i < NUM_THREADS; i++){
		// Here: call mythread_join to suspend prototype_os
		mythread_join(i);
	}

	while (true) {
		alt_printf("This is the prototype os for my exciting CSE351 course projects!\n");
		for (j = 0; j < MAIN_DELAY; j++) {iterations++;}
	}
}


uint32_t myinterrupt_handler(void* context) {
	uint32_t counter = *(uint32_t *)context;
	uint32_t ticks = ALARMTICKS(QUANTUM_LENGTH);
	g_tickCounter = alt_nticks();
#if SHOW_ITERRUPT_STATS == 1
	//printf("---------> Interrupted! Elapsed=%lu ticks, Period=%lu, Main Loop Iterations= %lu.\n", g_tickCounter, ticks, counter);
#endif
	set_timer_flag();
	return ticks;
}

uint16_t stackTest(
		int16_t iterationsToDo,
		int16_t iteration,
		int32_t initialStackPointer,
		int32_t framePointer,
		int32_t stackEnd,
		int32_t stackConsumption,
		int32_t  stackRemainingBytes){

	if(iteration == 0){
		NIOS2_READ_SP(initialStackPointer);
	}

	if(iteration == 0){
		TCB* me = GetRunningThread();
		stackEnd = (uint32_t)*(&(me->stack));
		framePointer = &me->fp;
	}

	if(iterationsToDo > 0){
		iterationsToDo = iterationsToDo -1;
		iteration = iteration + 1;

		int32_t stackptr = 0;
		NIOS2_READ_SP(stackptr);

		stackConsumption =  initialStackPointer - stackptr;
		stackRemainingBytes = stackptr - stackEnd;
		int16_t bytesPerCall = stackConsumption / iteration;

		return stackTest(iterationsToDo,iteration,initialStackPointer, framePointer, stackEnd,stackConsumption,stackRemainingBytes);
	}
	return iteration;
}

// Provided thread code.  Cannot be modified
void mythread(uint32_t thread_id){
	int i, j, n;
	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++){
		printf("This is message %d of thread #%lu.\n", i, thread_id);
		for(j=0 ; j < MAX ; j++){}
//		for(j=0 ; j < 10 ; j++){
//			uint16_t iterations = stackTest(250, 0, 0, 0,0, 0, 0);
//		}

	}
}



int check_timer_flag(){ return timer_interrupt_flag; }
void set_timer_flag(){ timer_interrupt_flag = 1; }
void reset_timer_flag(){ timer_interrupt_flag = 0; }

/***********************************************************************
* Entry Point
***********************************************************************/
int main() {
	printf("PrototypeOS ");
	prototype_os(); return 0;
}
