#include "P2PrototypeOS.h"


node_t *Node_init(TCB *data) {
	node_t  *elem = malloc(sizeof(node_t));
	elem->data = data;
	elem->parentNode = NULL;
	elem->childNode = NULL;
	return elem;
}


/*The queue is technically a linked list without a limit,
* but I'm limiting it by the size so it can replace ringbuffer and I can test it easier
* Passing in 0 will make it infinit*/
Queue *Queue_init(uint32_t maxsize) {
	Queue *q = calloc(1, sizeof(Queue));
	q->maxsize = maxsize;
	q->count = 0;
	return q;
}

//void Queue_Enqueue(Queue *q, void *data) {
void Queue_Enqueue(Queue *q, TCB *data) {
	if (q->maxsize == 0) {
		//This is a limitless Queue
	}
	else if(q->count < 0 || q->maxsize < 0 || q->count > q->maxsize) {
		printf("ERROR: QUEUE is CORRUPT\n");
		return;
	}
	else if (q->count == q->maxsize) {
		printf("ERROR: Queue is Full\n");
		return;
	}
	node_t  *elem = Node_init(data);
	if (q->count == 0) {
		q->firstNode = elem;
		q->lastNode = q->firstNode;
	}
	else {
		elem->parentNode = q->lastNode;
		q->lastNode->childNode = elem;
	}
	q->lastNode = elem;
	q->count++;
}

TCB* Queue_Dequeue(Queue *q) {
	if (q->count < 0 || q->maxsize < 0) {
		printf("ERROR: Invalid Pointer To Queue\n");
		return NULL;
	}

	//void *data = NULL;
	TCB *data = NULL;
	if (q->count != 0) {
		node_t *oldLeadingNode = q->firstNode;
		data = oldLeadingNode->data;

		q->firstNode = oldLeadingNode->childNode;
		q->count--;

		//Freeing breaks things?  Works fine in Visual Studio Test program
		if (q->count > 0) {
			free(q->firstNode->parentNode);
			q->firstNode->parentNode = NULL;
		}
		else if (q->count == 0) {
			free(q->firstNode);
			free(q->lastNode);
			q->firstNode = NULL;
			q->lastNode = NULL;
			oldLeadingNode = NULL;
		}
		else {
			q->firstNode = NULL;
			q->lastNode = NULL;
			oldLeadingNode = NULL;
		}
	}
	else {
		printf("Nothing to Dequeue\n");
	}
	return data;
}

TCB* Queue_Peek(Queue *q) {
	TCB *data = NULL;
	if (q->count != 0) {
		data = q->firstNode->data;
	}
	return data;
}


ThreadQueue* ThreadQueue_init() {
	ThreadQueue *tq = malloc(sizeof(ThreadQueue));
	uint32_t i = 0;
	for(i=0; i<numberOfThreadStatuses; i++){
		tq->threadQueues[i] = Queue_init(0);//Init to unlimited size queues
	}
	return tq;
}

uint32_t ThreadCount(ThreadQueue *tq, threadStatus status){
	return (tq->threadQueues[status])->count;
}

void EnqueueThread(ThreadQueue *tq, threadStatus status, TCB *tcb) {
	if(status < READY || status > DONE){
		alt_printf("ERROR: Invalid Thread Status.  Possible Null Thread.\n");
	}
	Queue_Enqueue(tq->threadQueues[status], tcb);
}

TCB * DequeueThread(ThreadQueue *tq, threadStatus status){
	return Queue_Dequeue(tq->threadQueues[status]);
}

TCB * PeekThread(ThreadQueue *tq, threadStatus status){
	return Queue_Peek(tq->threadQueues[status]);
}


TCB * LookupThread(ThreadQueue *tq, threadStatus status, uint32_t thread_id){
	Queue *q = tq->threadQueues[status];
	node_t *node = q->firstNode;

	while(node!=NULL && node->data !=NULL){
		if(node->data->thread_id == thread_id) {
			return node->data;
		} else {
			node = node->childNode;
		}
	}
	return NULL;
}










