#include "P2_PrototypeOS.h"

/***********************************************************************
* Variables
***********************************************************************/
/* Note: Made it a variable so I can change it in the debugger.  */
static alt_alarm alarm;
static volatile ThreadQueue *threads;
static TCB *(running_thread);

alt_u32 myinterrupt_handler(void* context) {
	alt_u32 counter = *(alt_u32 *)context;
	/* This function is called once per second by default.  It is a callback function */
	printf("Interrupted! Elapsed=%lu ticks, Period=%lu, Loop Iterations: %lu.\n", alt_nticks(), ALARMTICKS(QUANTUM_LENGTH), counter);
	//j=0; // It's able to sometimes beat the interrupt.  It averages 205,240 cycles.
	// Here: the global flag is used to indicate a timer interrupt
	timer_interrupt_flag = 1;
	return ALARMTICKS(QUANTUM_LENGTH);
}


// This is the scheduler. It works with Injection.S to switch between threads
alt_u64 mythread_scheduler(alt_u64 param_list){ // context pointer
	//DISABLE_INTERRUPTS
	alt_u32 * param_ptr = 	&param_list;
	alt_u32 stackpointer =  *param_ptr; //Returns in register r4
	alt_u32 framepointer =  *(param_ptr+1); //Returns in register r5


	//TCB *running_thread = PeekThread(threads,RUNNING);
	bool bAlreadySetMainThread = false;
	if(running_thread == NULL){
		running_thread = DequeueThread(threads, READY);
		if(bAlreadySetMainThread){
			alt_printf("No Running Thread and already created a main thread.");
		}
//		running_thread = malloc(sizeof(TCB));
//		running_thread->thread_id = MAIN_THREAD_ID;
//		running_thread->sp = stackpointer;
//		running_thread->fp = framepointer;

		running_thread->scheduling_status = RUNNING;
		//EnqueueThread(threads, RUNNING, running_thread);
//		bAlreadySetMainThread = true;
	}
	running_thread->sp = stackpointer;
	running_thread->fp = framepointer;

	TCB *nextThread = DequeueThread(threads, READY);
	if (nextThread != NULL){ //Check if one was available for popping from the ready queue
		if (running_thread->scheduling_status == RUNNING) {
			running_thread->scheduling_status = READY;
		}

		EnqueueThread(threads, running_thread->scheduling_status, running_thread);
		nextThread->scheduling_status = RUNNING;
		running_thread = nextThread;

	} else { // No other threads available
		alt_printf("Thread Queue Empty!\n");
		running_thread->scheduling_status = RUNNING;
		return 0;
	}

	// Prepare values to return
	alt_u64 ret_list;
	alt_u32 * rets = &ret_list;
	*(rets) = running_thread->sp;
	*(rets + 1) = running_thread->fp;

	//ENABLE_INTERRUPTS //Enabling interupts here causes it to jump back up to running_thread->sp = stackpointer;

	return ret_list;
}


// Provided thread code
void mythread(alt_u32 thread_id){
	// The declaration of j as an integer was added on 10/24/2011
	int i, j, n;
	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++){
		printf("This is message %d of thread #%d.\n", i, thread_id);
		for (j = 0; j < MAX; j++);
	}
}

// Creates a thread and adds it to the ready queue
TCB *mythread_create(void (*start_routine)(alt_u32), alt_u32 thread_id,  threadStatus status ) {
	TCB *tcb = calloc(1, sizeof(TCB));
	tcb->thread_id = thread_id;
	tcb->blocking_id = -1;
	tcb->scheduling_status = status;
	tcb->context = malloc(4096);
	tcb->fp = tcb->context + 4096/4;
	tcb->sp = tcb->context + 128/4;

	register int sp asm ("sp");
	//printf("%x", sp);

	int one = 1;

	//Gap for muldiv handler
	//memcpy(tcb->sp + 8/4,  xxxx, 4);//r1
	//memcpy(tcb->sp + 12/4, xxxx, 4);//r2
	//memcpy(tcb->sp + 16/4, xxxx, 4);//r3
	memcpy(tcb->sp + 20/4, &thread_id, 4);//r4 Argument one, is thread_id
	//memcpy(tcb->sp + 24/4, &one, 4);//r5 /estatus?
	//memcpy(tcb->sp + 28/4, xxxx, 4);//r6
	//memcpy(tcb->sp + 32/4, xxxx, 4);//r7
	//memcpy(tcb->sp + 32/4, xxxx, 4);//r8


	memcpy(tcb->sp + 68/4, &one, 4);//r5 /estatus?
	memcpy(tcb->sp + 72/4, &start_routine, 4);//ea  Becomes program counter/Instruction that caused the exception
	memcpy(tcb->sp + 0, &mythread_cleanup, 4); //ra Will return here after executing start_routine

	//memcpy(tcb->sp + 76/4, &start_routine, 4);//StackPointer

	//I think th eproblem is right here.
	//memcpy(tcb->sp + 84/4, &tcb->fp, 4);//fp

	alt_printf("Finished creation (%x): sp: (%x)\n", thread_id, tcb->context);
	return tcb;
}

// Joins the thread with the calling thread
void mythread_join(alt_u32 thread_id){
	int i=0;

//	DISABLE_INTERRUPTS
//	// Wait for timer the first time
//	TCB *running_thread = PeekThread(threads,RUNNING);
//	ENABLE_INTERRUPTS
	while (running_thread == NULL){
		for (i = 0 ; i < MAX; i++);
//		DISABLE_INTERRUPTS
//		running_thread = PeekThread(threads,RUNNING);
//		ENABLE_INTERRUPTS
	}

	int calling_id = running_thread->thread_id;
	alt_printf("Joining Thread(%x).\n", calling_id);

	DISABLE_INTERRUPTS
	TCB *tcb = LookupThread(threads, READY, thread_id);
	if (tcb != NULL && tcb->scheduling_status != DONE){
		//NOTE: SHOULD Push this onto WAITING STACK
		tcb->blocking_id = calling_id;
		running_thread->scheduling_status = WAITING;
		alt_printf("Joined (%x)\n", thread_id);
	}
	ENABLE_INTERRUPTS
	// Wait for timer
	while (running_thread->scheduling_status == WAITING){
		for (i = 0 ; i < MAX; i++);
	}
}

// Threads return here and space is freed
void mythread_cleanup(){
	// Unblock thread blocked by join
	DISABLE_INTERRUPTS

	TCB *blockedTCB = NULL;
	int id = running_thread->blocking_id;

	if (id > 0) {
		blockedTCB = PullThreadFromQueue(threads, WAITING, id);
		if (blockedTCB != NULL) {// not found
			blockedTCB->scheduling_status = READY;
			EnqueueThread(threads, READY, blockedTCB);
		}
	}
	ENABLE_INTERRUPTS
	alt_printf("COMPLETED.\n");

	DISABLE_INTERRUPTS
	free(running_thread->context);
	running_thread->scheduling_status = DONE;
//	TCB *tcb =Thread_Unqueue(RUNNING);
//	free(tcb->context);
//	tcb->scheduling_status = DONE;
//	Thread_Queue(DONE,tcb);
	ENABLE_INTERRUPTS

	while(TRUE);
}

int check_timer_flag(){
	return timer_interrupt_flag; //returns in registers (2 and 3)
}

void reset_timer_flag(){
	timer_interrupt_flag = 0; //returns in registers (2 and 3)
}

/***************************************************************************
* Prototype OS
****************************************************************************/
void prototype_os(void) {
	printf("Started.%lu\n", ALARMTICKS(QUANTUM_LENGTH));

	threads = ThreadQueue_init();

	alt_u32 i = 0;
	for (i = 0; i < NUM_THREADS; i++){
		// Here: call mythread_join to suspend prototype_os
		TCB *tcb = mythread_create(&mythread, i, READY);
		EnqueueThread(threads, READY, tcb);
	}

	alt_u32 j = 0;
	// Here: initialize the timer and its interrupt handler
	alt_alarm_start(&alarm, ALARMTICKS(QUANTUM_LENGTH), myinterrupt_handler, (void*)(&j));

	for (i = 0; i < NUM_THREADS; i++){
		// Here: call mythread_join to suspend prototype_os
		mythread_join(i);
	}

	while (true) {
		alt_printf("This is the prototype os for my exciting CSE351 course projects!\n");

		//Here: think about what MAX is used for. Pick up an appropriate value for it experimentally.
		for (j = 0; j < 150000; j++) {
			//With 110633 as the Max and no nop's, the interrupt interrupts right in the middle of printing the above sentence.
			asm("nop"); //One nop reduces collisions
			asm("nop");  //further reduces collisions.
		}
	}
}

/***********************************************************************
* Entry Point
***********************************************************************/
int main() { prototype_os(); return 0; }
