
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sys/alt_alarm.h"
#include "sys/stdio.h"
#include "alt_types.h"


/***************************************************************************
* Boolean Definition
****************************************************************************/
#define FALSE 0
#define TRUE  !0
typedef enum bool { false = FALSE, true  = TRUE} bool;

#define NULL 0

/***************************************************************************
* Macros
****************************************************************************/
#define DISABLE_INTERRUPTS asm("wrctl status, zero");
#define ENABLE_INTERRUPTS asm("movi et, 1"); asm("wrctl status, et");

#define ALARMTICKS(x) ((alt_ticks_per_second()*(x))/10)

#define MAX 50000
#define QUANTUM_LENGTH 10
#define NUM_THREADS 8

/***********************************************************************
* Thread Structures
***********************************************************************/
typedef enum threadStatus {RUNNING = 0,READY = 1, WAITING = 2, START = 3, DONE = 4} threadStatus;
#define numberOfThreadStatuses  5 // 5 elements: Running, Ready, Waiting, Start, Done

typedef struct {
	int thread_id;
	threadStatus scheduling_status;
	int *context;
	int *sp;
	int *fp;
	int blocking_id;
} TCB;

typedef struct Node{
	TCB thread;
	struct Node *previous;
	struct Node *next;
} Node;

Node *(threadQueues[numberOfThreadStatuses]);
Node *(running_thread);

// Add a node to the specified status queue
void add_node(Node *new_node, threadStatus status) {
	if (threadQueues[status] == NULL) // 0 nodes
	{
		threadQueues[status] = new_node;
		threadQueues[status]->next = NULL;
		threadQueues[status]->previous = NULL;
	}
	else if (threadQueues[status]->next == NULL) // 1 node
	{
		threadQueues[status]->next = new_node;
		threadQueues[status]->previous = new_node;
		new_node->next = threadQueues[status];
		new_node->previous = threadQueues[status];
	}
	else // 2+ nodes
	{
		new_node->previous = threadQueues[status]->previous;
		new_node->previous->next = new_node;
		threadQueues[status]->previous = new_node;
		new_node->next = threadQueues[status];
	}
}

// Pop the first node from the specified status queue
Node * pop(threadStatus status) {
	Node *popped = NULL;
	if (threadQueues[status] == NULL) //0 nodes
	{
		// Do nothing. Popped is already NULL
	}
	else if (threadQueues[status]->next == NULL) //1
	{
		popped = threadQueues[status];
		threadQueues[status] = NULL;
		return popped;
	}
	else if (threadQueues[status]->next == threadQueues[status]->previous) //2
	{
		popped = threadQueues[status];
		threadQueues[status] = popped->next;
		threadQueues[status]->next = NULL;
		threadQueues[status]->previous = NULL;
	}
	else //3+
	{
		popped = threadQueues[status];
		threadQueues[status] = popped->next;
		Node *last = popped->previous;
		popped->next->previous = last;
		last->next = popped->next;
		popped->next = NULL;
		popped->previous = NULL;
		return popped;
	}
	return popped;
}

// Remove a node from the specified status queue
void remove_node(Node *node, threadStatus status) {
	if (node->next == NULL) //1 node
	{
		if (node == threadQueues[status])
		{
			threadQueues[status] = NULL;
		}
	}
	else if (threadQueues[status]->next == threadQueues[status]->previous)//2
	{
		threadQueues[status] = node->next;
		threadQueues[status]->next = NULL;
		threadQueues[status]->previous = NULL;
	}
	else//2+
	{
		Node *previous = node->previous;
		Node *next = node->next;
		previous->next = next;
		next->previous = previous;
		if (node == threadQueues[status])
		{
			threadQueues[status] = next;
		}
	}
}

// Lookup a node in the specified status queue
Node * lookup_node(int id, threadStatus status) {
	Node * node = threadQueues[status];
	if (node == NULL) {
		return -1;
	}
	else if (node->next == NULL)
	{
		if (node->thread.thread_id == id)
			return node;
		else
			return -1;
	}
	else
	{
		if (node->thread.thread_id == id)
			return node;
		node = node->next;
		while (node != threadQueues[status])
		{
			if (node->thread.thread_id == id)
				return node;
			node = node->next;
		}
	}
	return -1;
}

void prototype_os();
alt_u64 mythread_scheduler(alt_u64 param_list);
alt_u32 mythread_handler(void *param_list);
void mythread(int thread_id);
void mythread_create(TCB *tcb, void *(*start_routine)(void*), int thread_id);
void mythread_join(int thread_id);
void mythread_cleanup();
int timer_interrupt_flag;

// Our operating system prototype
void prototype_os(){
	int i = 0;
	running_thread = NULL;
	TCB *threads[NUM_THREADS];
	for (i = 0; i < NUM_THREADS; i++)
	{
		// Here: call mythread_create so that the TCB for each thread is created
		TCB *tcb = (TCB *) malloc(sizeof(TCB));
		mythread_create(tcb, &mythread, i);
		threads[i] = tcb;
	}
	// Here: initialize the timer and its interrupt handler as is done in Project I
	alt_alarm * myAlarm;
	alt_alarm_start( &myAlarm, ALARMTICKS(QUANTUM_LENGTH), &mythread_handler, NULL);
	for (i = 0; i < NUM_THREADS; i++)
	{
		// Here: call mythread_join to suspend prototype_os
		mythread_join(i);
	}

	while (TRUE)
	{
		alt_printf ("This is the OS prototype for my exciting CSE351 course projects!\n");
		int j = 0;
		for (j = 0 ; j < MAX * 10; j++);
	}
}

// This is the scheduler. It works with Injection.S to switch between threads
alt_u64 mythread_scheduler(alt_u64 param_list) {
	DISABLE_INTERRUPTS
	alt_u32 * param_ptr = 	&param_list;
	alt_u32 stackpointer =  *param_ptr; //Returns in register r4
	alt_u32 framepointer =  *(param_ptr+1); //Returns in register r5

	// If running thread is null, then store the context and add to the run queue
	if (running_thread == NULL){
		// Store a new context (os_prototype, most likely)
		TCB *tcb = (TCB *) malloc(sizeof(TCB));
		tcb->thread_id = NUM_THREADS + 1; // TODO:set to something legitimate
		tcb->sp = stackpointer;
		tcb->fp = framepointer;
		Node *node = (Node *) malloc(sizeof(Node));
		node->thread = *tcb;
		running_thread = node;
		running_thread->thread.scheduling_status = RUNNING;
	}
	running_thread->thread.sp = stackpointer;
	running_thread->thread.fp = framepointer;

	// Here: perform thread scheduling
	Node *next = pop(READY);
	if (next != NULL  && next->thread.scheduling_status == READY)
	{
		// The context of the second thread (1) is crap. Something is probably wrong with creation or join. Else there's a problem in assembly with storing the fp
		if (running_thread->thread.scheduling_status == RUNNING) {
			running_thread->thread.scheduling_status = READY;
		}
		add_node(running_thread, running_thread->thread.scheduling_status);
		running_thread = (Node *) malloc(sizeof(Node));
		running_thread->thread = next->thread;
		//running_thread->thread->thread_id = next->thread->thread;
		running_thread->thread.scheduling_status = RUNNING;
	}
	else // No other threads available
	{
		alt_printf("Interrupted by the DE2 timer!\n");
		running_thread->thread.scheduling_status = RUNNING;
		return 0;
	}
	// Prepare values to return
	unsigned long long ret_list;
	int * rets = &ret_list;
	*(rets) = running_thread->thread.sp;
	*(rets + 1) = running_thread->thread.fp;

	ENABLE_INTERRUPTS
	return ret_list;
}

// Sets the timer_interrupt_flag that is checked by Injection.S
alt_u32 mythread_handler(void *param_list)
{
	// Here: the global flag is used to indicate a timer interrupt
	timer_interrupt_flag = 1;
	alt_printf("Interrupted by the timer!\n");
	return ALARMTICKS(QUANTUM_LENGTH);
}

// Provided thread code
void mythread(int thread_id)
{
	// The declaration of j as an integer was added on 10/24/2011
	int i, j, n;
	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++)
	{
		printf("This is message %d of thread #%d.\n", i, thread_id);
		for (j = 0; j < MAX; j++);
	}
}

// Creates a thread and adds it to the ready queue
void mythread_create(TCB *tcb, void *(*start_routine)(void*), int thread_id)
{
	alt_printf("Creating...\n");
	// Creates a Thread Control Block for a thread
	tcb->thread_id = thread_id;
	tcb->blocking_id = -1;
	tcb->scheduling_status = READY;
	tcb->context = malloc(4000);
	tcb->fp = tcb->context + 4000/4;
	tcb->sp = tcb->context + 128/4;

	int one = 1;
	void *(*ra)(void *) = &mythread_cleanup;
	memcpy(tcb->sp + 0, &ra, 4);//ra
	memcpy(tcb->sp + 20/4, &thread_id, 4);//r4?
	memcpy(tcb->sp + 72/4, &start_routine, 4);//ea
	memcpy(tcb->sp + 68/4, &one, 4);//estatus
	memcpy(tcb->sp + 84/4, &tcb->fp, 4);//fp

	// Add to ready queue
	Node *node = (Node *) malloc(sizeof(Node));
	node->thread = *tcb;
	add_node(node, READY);
	alt_printf("Finished creation (%x): sp: (%x)\n", thread_id, tcb->context);
}

// Joins the thread with the calling thread
void mythread_join(int thread_id)
{
	// Wait for timer the first time
	int i;
	while (running_thread == NULL)
		for (i = 0 ; i < MAX; i++);
	int joined = FALSE;
	Node *temp = -1;
	temp = lookup_node(thread_id, READY);
	TCB *tcb;
	int calling_id = running_thread->thread.thread_id;
	alt_printf("Joining if not finished.\n");

	temp = lookup_node(thread_id, READY);
	if (temp != 0xffffffff)
		tcb = &temp->thread;
	if (temp != 0xffffffff && tcb->scheduling_status != DONE){
		// Join the thread
		tcb->blocking_id = calling_id;
		running_thread->thread.scheduling_status = WAITING;
		joined = TRUE;
	}

	if (joined == TRUE)
		alt_printf("Joined (%x)\n", thread_id);
	// Wait for timer
	while (running_thread->thread.scheduling_status == WAITING)
		for (i = 0 ; i < MAX; i++);
}

// Threads return here and space is freed
void mythread_cleanup()
{
	// Unblock thread blocked by join
	DISABLE_INTERRUPTS
	int id = running_thread->thread.blocking_id;
	if (id > 0) {
		Node * temp = 0xffffffff;
		temp = lookup_node(running_thread->thread.blocking_id, WAITING); //Blocking ID was not the expected value Camtendo 11/4
		if (temp != 0xffffffff) // not found
		{
			Node * blocked_node = (Node *) malloc(sizeof(Node));
			blocked_node->thread = temp->thread;
			blocked_node->thread.scheduling_status = READY;
			remove_node(temp, WAITING);
			add_node(blocked_node, READY);
		}
	}
	ENABLE_INTERRUPTS
	alt_printf("COMPLETED.\n");
	DISABLE_INTERRUPTS
	free(running_thread->thread.context);
	running_thread->thread.scheduling_status = DONE;
	ENABLE_INTERRUPTS
	while(TRUE);
}

int check_timer_flag()
{
	return timer_interrupt_flag; //returns in registers (2 and 3)
}

void reset_timer_flag()
{
	timer_interrupt_flag = 0; //returns in registers (2 and 3)
}

// The main method that starts up the prototype operating system
int main()
{
	alt_printf("Hello from Nios II!\n");
	prototype_os();
	return 0;
}
