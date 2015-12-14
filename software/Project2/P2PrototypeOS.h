#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "sys/alt_alarm.h"
#include "nios2.h"

#define QUANTUM_LENGTH 1000 //milliseconds

#define RELATIVE_SPEED_MULTIPLIER 30
#define MAX 3000 * RELATIVE_SPEED_MULTIPLIER
#define MAIN_DELAY 15000 * RELATIVE_SPEED_MULTIPLIER
#define THREAD_DELAY 10 * RELATIVE_SPEED_MULTIPLIER

#define NUM_THREADS 12
#define MAIN_THREAD_ID 1337
#define NULL_THREAD_ID -1
#define STACK_SIZE 16000

#define SHOW_THREAD_STATS 1
#define SHOW_ITERRUPT_STATS 1

/***************************************************************************
* Boolean Definition
****************************************************************************/
#define FALSE 0
#define TRUE  !0
typedef enum bool { false = FALSE, true  = TRUE} bool;


/***************************************************************************
* Macros
****************************************************************************/
#define DISABLE_INTERRUPTS() {  \
    asm("wrctl status, zero");  \
}

#define ENABLE_INTERRUPTS() {   \
    asm("movi et, 1");          \
    asm("wrctl status, et");    \
}


/* The resolution is 100ms but it accepts input in single millis */
#define ALARMTICKS(x) ((alt_ticks_per_second()*x)/1000)

/***********************************************************************
* Thread Structures
***********************************************************************/
typedef enum threadStatus {  READY = 0, RUNNING = 1,  WAITING = 2,  DONE = 3} threadStatus;
#define numberOfThreadStatuses  4 // 6 queues:  Running, Ready, Waiting, Done

typedef struct ThreadControlBlock {
	uint32_t thread_id;
	threadStatus scheduling_status;
	uint32_t *stack;
	uint32_t *sp;
	uint32_t *fp;
	uint32_t blocking_id;
	uint32_t totalTicks;
	uint32_t lastStartTicks;
	uint32_t startTicks;
} TCB;

//TCB *ThreadControlBlock_create(void (*start_routine)(int), int thread_id);

/***************************************************************************
* Linked List Queue
****************************************************************************/
typedef struct node_t {
	//void*			data;
	TCB* data;
	struct node_t*	parentNode;
	struct node_t*	childNode;
} node_t;


node_t*	Node_init(TCB *data);

typedef struct Queue {
	alt_32	maxsize;
	alt_32	count;
	node_t*	firstNode;
	node_t*	lastNode;
} Queue;

//Pass In 0 for unrestrained queue.
Queue*	Queue_init(alt_32 max_size);
void	Queue_Enqueue(Queue *q, TCB *data);
TCB*	Queue_Dequeue(Queue *q);
TCB*	Queue_Peek(Queue *q);

/***********************************************************************
* Thread Queue
***********************************************************************/
typedef struct ThreadQueue {
	Queue	*(threadQueues[numberOfThreadStatuses]);
} ThreadQueue;

ThreadQueue* ThreadQueue_init();
uint32_t ThreadCount(ThreadQueue *tq, threadStatus status);
void EnqueueThread(ThreadQueue *tq, threadStatus status, TCB *tcb);
TCB * DequeueThread(ThreadQueue *tq, threadStatus status);
TCB * PeekThread(ThreadQueue *tq, threadStatus status);
TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id);

/***********************************************************************
* Function Declarations
***********************************************************************/
void prototype_os(void);

uint32_t myinterrupt_handler(void* context);
uint64_t mythread_scheduler(uint64_t context);
void mythread(uint32_t thread_id);
TCB *CreateThread(void (*start_routine)(uint32_t), uint32_t thread_id,  threadStatus status, int stacksizeBytes);
void mythread_join(uint32_t thread_id);
void CleanupThread();

/***********************************************************************
* Called through Assembly
***********************************************************************/
static volatile int timer_interrupt_flag;
int check_timer_flag();
void set_timer_flag();
void reset_timer_flag();







