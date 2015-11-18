#include "P2_PrototypeOS.h"

/***********************************************************************
* Variables
***********************************************************************/
/* Note: Made it a variable so I can change it in the debugger.  */
static alt_alarm alarm;
static ThreadQueue *threads;


TCB *mythread_create(void (*start_routine)(alt_u32), alt_u32 thread_id,  threadStatus status, int stacksizeBytes) {
	TCB *tcb = malloc(sizeof(TCB));
	tcb->thread_id = thread_id;
	tcb->blocking_id = -1;
	tcb->scheduling_status = status;

	tcb->context = malloc(stacksizeBytes);
	tcb->fp = tcb->context + stacksizeBytes/4;
	tcb->sp = tcb->context +(stacksizeBytes/4) - (76/4);

	memcpy(tcb->sp + 20/4, &thread_id, 4);//r4 Argument one, is thread_id
	int one = 1;
	memcpy(tcb->sp + 68/4, &one, 4); 	//Location 68 gets loaded into r5 then into estatus.  Store the number one into the lowest byte
	memcpy(tcb->sp + 72/4, &start_routine, 4);//ea  Becomes program counter/Instruction that caused the exception

	void *(*ra)(void *) = &mythread_cleanup;
	memcpy(tcb->sp + 0, &ra, 4); //ra Will return here after executing start_routine
	return tcb;
}

// This is the scheduler. It works with Injection.S to switch between threads
alt_u64 mythread_scheduler(alt_u64 param_list){ // context pointer
#if SHOW_THREAD_STATS == 1
	char strBuff[160];
#endif

	TCB *thisThread = NULL;
	TCB *nextThread = NULL;

	alt_u32 * param_ptr = 	&param_list;
	alt_u32 stackpointer =  *param_ptr; //Returns in register r4
	alt_u32 framepointer =  *(param_ptr+1); //Returns in register r5

	alt_u64 returnValue;
	alt_u32 * retptr = &returnValue;

	alt_u16 nThreadsRunning = ThreadCount(threads, RUNNING);
	alt_u16 nThreadsReady = ThreadCount(threads, READY);
	alt_u16 nThreadsWaiting = ThreadCount(threads,WAITING);
	alt_u16 nThreadsDone = ThreadCount(threads, DONE);

	if(nThreadsRunning == 0){
		thisThread = malloc(sizeof(TCB));
		thisThread->thread_id = MAIN_THREAD_ID;
		thisThread->scheduling_status = RUNNING;
		thisThread->sp = stackpointer;
		thisThread->fp = framepointer;
		EnqueueThread(threads, thisThread->scheduling_status, thisThread);
		nThreadsRunning = ThreadCount(threads,RUNNING);
	}
#if SHOW_THREAD_STATS == 1
	sprintf(strBuff, "RunningThreads=%d | ReadyThreads=%d | WaitingThreads=%d | DoneThreads=%d", nThreadsRunning, nThreadsReady, nThreadsWaiting, nThreadsDone);
#endif
	if(nThreadsReady > 0 || nThreadsWaiting > 0){
		thisThread = DequeueThread(threads, RUNNING);

		if(thisThread->scheduling_status == DONE){
			EnqueueThread(threads, thisThread->scheduling_status , thisThread); //Should be set by thread cleanup
			nextThread = DequeueThread(threads, READY);
		}
		else if( thisThread->scheduling_status == WAITING){
			thisThread->sp = stackpointer;
			thisThread->fp = framepointer;
			EnqueueThread(threads, thisThread->scheduling_status , thisThread); //Should be set by thread cleanup
			nextThread = DequeueThread(threads, nThreadsReady > 0 ? READY : WAITING);
		}
		else{
			thisThread->sp = stackpointer;
			thisThread->fp = framepointer;
			thisThread->scheduling_status = READY;
			EnqueueThread(threads, thisThread->scheduling_status, thisThread);
			nextThread =  DequeueThread(threads, nThreadsReady > 0 ? READY : WAITING);
		}

		nextThread->scheduling_status = RUNNING;
		EnqueueThread(threads, nextThread->scheduling_status, nextThread);

		*(retptr) = nextThread->sp;
		*(retptr + 1) = nextThread->fp;
#if SHOW_THREAD_STATS == 1
		sprintf(strBuff + strlen(strBuff), " | Queueing ThreadID=%lu | Scheduling ThreadID=%lu", thisThread->thread_id, nextThread->thread_id);
#endif
	}
	else {
#if SHOW_THREAD_STATS == 1
		sprintf(strBuff + strlen(strBuff), " | No Queued Threads");
#endif
		returnValue = param_list;
	}
#if SHOW_THREAD_STATS == 1
	strcat(strBuff,"\n");
	printf(strBuff);
#endif
	return returnValue;
}



// Joins the thread with the calling thread
void mythread_join(alt_u32 blockingThreadID){
	alt_32 i=0;
	TCB *runningThread;
	TCB *blockingThread;

//	DISABLE_INTERRUPTS
	// Wait for timer the first time
	runningThread = PeekThread(threads,RUNNING);
	while (runningThread == NULL){
		//ENABLE_INTERRUPTS
		for (i = 0 ; i < THREAD_DELAY; i++){asm("nop");};
		//DISABLE_INTERRUPTS
		runningThread = PeekThread(threads,RUNNING);
	}

	printf("Joining ThreadID=%lu ", blockingThreadID);

	if(runningThread->thread_id==blockingThreadID){
		//ENABLE_INTERRUPTS
		while (runningThread->scheduling_status != DONE){
			for (i = 0 ; i < THREAD_DELAY; i++){};
		}
	}
	else{
		blockingThread = LookupThread(threads, READY, blockingThreadID);
		if(blockingThread->thread_id != blockingThreadID){
		//	ENABLE_INTERRUPTS
			printf("We have a problem...");
		}
		if(blockingThread==NULL){
		//	ENABLE_INTERRUPTS
			printf("ERROR: COULD NOT FIND THREAD TO JOIN WITH ID: %lu", blockingThreadID);
		}else{
			runningThread->scheduling_status = WAITING;
			runningThread->blocking_id =blockingThreadID;
		//	ENABLE_INTERRUPTS
			printf("Joined %lu\n", blockingThreadID);
			while (blockingThread->scheduling_status != DONE){
				for (i = 0 ; i < THREAD_DELAY; i++){};
			}
		}

	}
}

// Threads return here and space is freed
void mythread_cleanup(){
	// Unblock thread blocked by join
	DISABLE_INTERRUPTS

	TCB *running_thread = PeekThread(threads,RUNNING);
	free(running_thread->context);
	running_thread->scheduling_status = DONE;
	alt_printf("COMPLETED.\n");

//	TCB *blockedTCB = NULL;
//	int id = running_thread->blocking_id;
//
//	if (id > 0) {
//		blockedTCB = PullThreadFromQueue(threads, WAITING, id);
//		if (blockedTCB != NULL) {// not found
//			blockedTCB->scheduling_status = READY;
//			EnqueueThread(threads, READY, blockedTCB);
//		}
//	}
//
	ENABLE_INTERRUPTS
	while(TRUE);
}


/*********************************************************************
* Prototype OS
****************************************************************************/
void prototype_os(void) {
	printf("Started.%lu\n", ALARMTICKS(QUANTUM_LENGTH));
	alt_u32 i, j, iterations = 0;

	// Here: initialize the timer and its interrupt handler
	alt_alarm_start(&alarm, ALARMTICKS(QUANTUM_LENGTH), myinterrupt_handler, (void*)(&iterations));

	threads = ThreadQueue_init();

	for (i = 0; i < NUM_THREADS; i++){
		EnqueueThread(threads, READY,  mythread_create(&mythread, i, READY, STACK_SIZE));
		printf("Finished creation (%lu): sp: 0x%x\n", i, PeekThread(threads, READY)->context);
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


alt_u32 myinterrupt_handler(void* context) {
	alt_u32 counter = *(alt_u32 *)context;
	alt_u32 ticks = ALARMTICKS(QUANTUM_LENGTH);
#if SHOW_THREAD_STATS == 1
	printf("Interrupted! Elapsed=%lu ticks, Period=%lu, Main Loop Iterations: %lu.\n", alt_nticks(), ticks, counter);
#endif
	set_timer_flag();
	return ticks;
}

// Provided thread code.  Cannot be modified
void mythread(alt_u32 thread_id){
	int i, j, n;
	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++){
		printf("This is message %d of thread #%lu.\n", i, thread_id);
		for (j = 0; j < MAX; j++);
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
