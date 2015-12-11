#include "P2PrototypeOS.h"
/*
 * Queue
 *
 *  Created on: Nov 15, 2015
 *      Author: Derek
 */
/***************************************************************************
* Generic Linked List Queue
****************************************************************************/
//node_t *Node_init(void *data) {
node_t *Node_init(TCB *data) {
	node_t  *elem = malloc(sizeof(node_t));
	elem->data = data;
	elem->parentNode = NULL;
	elem->childNode = NULL;
	return elem;
}

//

/*The queue is technically a linked list without a limit,
* but I'm limiting it by the size so it can replace ringbuffer and I can test it easier
* Passing in 0 will make it infinit*/
Queue *Queue_init(alt_32 maxsize) {
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
		printf("ERROR: Nothing to Dequeue\n");
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


bool Queue_IsFull(Queue *q) {return (q->maxsize - q->count == 0) ? true : false;}
bool Queue_IsEmpty(Queue *q) {return (q->count) ? false : true;}

