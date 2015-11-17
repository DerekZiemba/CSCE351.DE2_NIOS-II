#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sys/alt_alarm.h"
#include "sys/alt_stdio.h"
#include "alt_types.h"

#define MAX 125000
#define QUANTUM_LENGTH 2000
#define NUM_THREADS 12
#define MAIN_THREAD_ID 31337
#define NULL_THREAD_ID -1
#define STACK_SIZE 8192//16384



/***************************************************************************
* Boolean Definition
****************************************************************************/
#define FALSE 0
#define TRUE  !0
typedef enum bool { false = FALSE, true  = TRUE} bool;


/***************************************************************************
* Macros
****************************************************************************/
#define DISABLE_INTERRUPTS asm("wrctl status, zero");
#define ENABLE_INTERRUPTS asm("movi et, 1"); asm("wrctl status, et");

/* The resolution is 100ms but it accepts input in single millis */
#define ALARMTICKS(x) ((alt_ticks_per_second()*x)/1000)

/***********************************************************************
* Thread Structures
***********************************************************************/
typedef enum threadStatus {  READY = 0, RUNNING = 1,  WAITING = 2,  DONE = 3} threadStatus;
#define numberOfThreadStatuses  4 // 6 queues:  Running, Ready, Waiting, Done

typedef struct ThreadControlBlock {
	alt_u32 thread_id;
	threadStatus scheduling_status;
	void *context;
	alt_u32 *sp;
	alt_u32 *fp;
	alt_u32 blocking_id;

} TCB;

//TCB *ThreadControlBlock_create(void (*start_routine)(int), int thread_id);

/***************************************************************************
* Generic Linked List Queue
****************************************************************************/
typedef struct node_t {
	//void*			data;
	TCB* data;
	struct node_t*	parentNode;
	struct node_t*	childNode;
} node_t;

//node_t*	Node_init(void *data);
node_t*	Node_init(TCB *data);
//void 	Node_Iterator(node_t *n, bool(*callback)(node_t* data, void* context), void *context);

typedef struct Queue {
	alt_32	maxsize;
	alt_32	count;
	node_t*	firstNode;
	node_t*	lastNode;
} Queue;

//Pass In 0 for unrestrained queue.
Queue*	Queue_init(alt_32 max_size);
//void	Queue_Enqueue(Queue *q, void *data);
void	Queue_Enqueue(Queue *q, TCB *data);
//void*	Queue_Dequeue(Queue *q);
//void*	Queue_Peek(Queue *q);
TCB*	Queue_Dequeue(Queue *q);
TCB*	Queue_Peek(Queue *q);
bool 	Queue_IsFull(Queue *q);
bool 	Queue_IsEmpty(Queue *q);
//void 	Queue_Print(Queue *q, char * format);

/* QUEUE_H_ */


/***********************************************************************
* Thread Queue
***********************************************************************/
typedef struct ThreadQueue {
	Queue	*(threadQueues[numberOfThreadStatuses]);
} ThreadQueue;

ThreadQueue* ThreadQueue_init();
alt_u32 ThreadCount(ThreadQueue *tq, threadStatus status);
void EnqueueThread(ThreadQueue *tq, threadStatus status, TCB *tcb);
TCB * DequeueThread(ThreadQueue *tq, threadStatus status);
TCB * PeekThread(ThreadQueue *tq, threadStatus status);
TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id);
TCB * PullThreadFromQueue(ThreadQueue *tq, threadStatus status, int thread_id);

/***********************************************************************
* Function Declarations
***********************************************************************/
void prototype_os(void);

alt_u32 myinterrupt_handler(void* context);
alt_u64 mythread_scheduler(alt_u64 param_list);
void mythread(alt_u32 thread_id);
TCB *mythread_create(void (*start_routine)(alt_u32), alt_u32 thread_id,  threadStatus status, int stacksizeBytes);
void mythread_join(alt_u32 thread_id);
void mythread_cleanup();

/***********************************************************************
* Called through Assembly
***********************************************************************/
static volatile int timer_interrupt_flag;
int check_timer_flag();
void set_timer_flag();
void reset_timer_flag();








