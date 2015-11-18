#include "P2_PrototypeOS.h"
/*
 * Generic Linked List
 *
 *  Created on: Nov 15, 2015
 *      Author: Derek
 */
/***************************************************************************
* Generic Linked List
****************************************************************************/
node_t *Node_init(void *data) {
	node_t  *elem = malloc(sizeof(node_t));
	elem->data = data;
	elem->parentNode = NULL;
	elem->childNode = NULL;
	return elem;
}

/*The queue is technically a linked list without a limit,
* but I'm limiting it by the size so it can replace ringbuffer and I can test it easier
* Passing in 0 will make it infinit*/
LinkedList *LList_init(uint32_t maxsize) {
	LinkedList *q = calloc(1, sizeof(LinkedList));
	q->maxsize = maxsize;
	q->count = 0;
	return q;
}

void LList_EnqueueGeneric(LinkedList *q, void* data) {
	LList_EnqueueNode(q, Node_init(data));
}

void* LList_DequeueGeneric(LinkedList *q) {
	node_t  *node = LList_DequeueNode(q);
	if(node == NULL){ return NULL;}
	void* result = node->data;
	free(node);
	return result;
}

void* LList_PeekGeneric(LinkedList *q) {
	if (q->count != 0) {	return q->firstNode->data;	}
	return NULL;
}


void LList_EnqueueNode(LinkedList *q, node_t* node) {
	if (q->maxsize == 0) { /*This is an unlimited Queue*/ }
	else if(q->count < 0 || q->maxsize < 0 || q->count > q->maxsize) { printf("ERROR: QUEUE is CORRUPT\n"); return; }
	else if (q->count == q->maxsize) { printf("ERROR: Queue is Full\n"); return; }
	if (q->count == 0) {
		q->firstNode = node;
		q->lastNode = q->firstNode;
	} else {
		node->parentNode = q->lastNode;
		q->lastNode->childNode = node;
	}
	q->lastNode = node;
	q->count++;
}

node_t* LList_DequeueNode(LinkedList *q) {
	if (q->count < 0 || q->maxsize < 0) {
		printf("ERROR: Invalid Pointer To Queue\n");
		return NULL;
	}
	
	node_t *node = NULL;
	if (q->count != 0) {
		node = q->firstNode;

		q->firstNode = node->childNode;
		q->count--;

		if (q->count > 0) {
			free(q->firstNode->parentNode);
			q->firstNode->parentNode = NULL;
		}
		else if (q->count == 0) {
			free(q->firstNode);
			free(q->lastNode);
			q->firstNode = NULL;
			q->lastNode = NULL;
		}
		else {
			q->firstNode = NULL;
			q->lastNode = NULL;
		}
	}
	else {
		printf("ERROR: Nothing to Dequeue\n");
	}
	return node;
}

node_t* LList_PeekNode(LinkedList *q) {
	if (q->count != 0) { return q->firstNode; }
	return NULL;
}

node_t* LList_LookupNode(LinkedList *q, node_t* (*callback) (node_t* node)) {
	node_t *node = q->firstNode;
	while(node!=NULL && node->data != NULL){
		node_t *selected = callback(node);
		if(selected != NULL){
			return selected;
		}
	}
	return NULL;
}


bool LList_IsFull(LinkedList *q) {return (q->maxsize - q->count == 0) ? true : false;}
bool LList_IsEmpty(LinkedList *q) {return (q->count) ? false : true;}


//
///*This was just an experiment.
// *  Wanted it to work for all and be used in the likes of SearchChildNodesByThreadID
// *  but I couldn't get things to cast back correctly.
// *   Also the GCC version used by altera doesn't allow trampolines which was essential for this.
// *   trampolines = functions inside of functions like javascript.  Just found out what those are a few minutes ago */
//void Node_Iterator(node_t *n, bool(*callback)(node_t* data, void* context), void *context) {
//	if (n == NULL) return;
//	if (n->data != NULL) {
//		if (callback(n, context)) {
//			Node_Iterator(n->childNode, callback, context);
//		}
//	}
//}




////void* Queue_Dequeue(Queue *q) {
//TCB* Queue_Dequeue(Queue *q) {
//	if (q->count < 0 || q->maxsize < 0) {
//		printf("ERROR: Invalid Pointer To Queue\n");
//		return NULL;
//	}
//
//	//void *data = NULL;
//	TCB *data = NULL;
//	if (q->count != 0) {
//		node_t *oldLeadingNode = q->firstNode;
//		data = oldLeadingNode->data;
//
//		q->firstNode = oldLeadingNode->childNode;
//		q->count--;
//
//		//Freeing breaks things?  Works fine in Visual Studio Test program
//		if (q->count > 0) {
//			free(q->firstNode->parentNode);
//			q->firstNode->parentNode = NULL;
//		}
//		else if (q->count == 0) {
//			free(q->firstNode);
//			free(q->lastNode);
//			q->firstNode = NULL;
//			q->lastNode = NULL;
//			oldLeadingNode = NULL;
//		}
//		else {
//			q->firstNode = NULL;
//			q->lastNode = NULL;
//			oldLeadingNode = NULL;
//		}
//	}
//	else {
//		printf("ERROR: Nothing to Dequeue\n");
//	}
//	return data;
//}




//void Queue_Print(Queue *q, char * format) {
//	bool lambda(node_t* data, void *context) {
//		printf((char*) context, (char*)data->data);
//		return true;
//	}
//	Node_Iterator(q->firstNode, &lambda, (void*) format);
//	printf("\n");
//}


/***************************************************************************
* ThreadQueue Linked List
****************************************************************************/
ThreadQueue* ThreadQueue_init() {
	ThreadQueue *tq = malloc(sizeof(ThreadQueue));
	alt_u32 i = 0;
	for(i=0; i<numberOfThreadStatuses; i++){
		tq->threadQueues[i] = LList_init(0);//Init to unlimited size queues
	}
	return tq;
}

alt_u32 ThreadCount(ThreadQueue *tq, threadStatus status){
	return (tq->threadQueues[status])->count;
}

void EnqueueThread(ThreadQueue *tq, threadStatus status, TCB *tcb) {
	if(status < READY || status > DONE){
		alt_printf("ERROR: Invalid Thread Status.  Possible Null Thread.\n");
	}
	LList_EnqueueGeneric(tq->threadQueues[status], (void*) tcb);
}

TCB * DequeueThread(ThreadQueue *tq, threadStatus status){
	void* ptr = LList_DequeueGeneric(tq->threadQueues[status]);
	return ptr==NULL ? NULL : (TCB *) ptr;
}

TCB * PeekThread(ThreadQueue *tq, threadStatus status){
	void* ptr = LList_Peek(tq->threadQueues[status]);
	return ptr==NULL ? NULL : (TCB *) ptr;
}


TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id){
	node_t * lambda(node_t *node){ return (((TCB*) node->data)->thread_id == thread_id) ? node : NULL;}
	node_t *ptr = LLIst_LookupNode(tq, &lambda);
	return ptr==NULL ? NULL : ((TCB *) ptr->data);
}

//TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id){
////	TCB *tcb = SearchChildNodesByThreadID(tq->threadQueues[status]->firstNode, thread_id);
//	//Node_Iterator(threadman.threadQueues[status]->firstNode, &lambda, (void*) tcb);
////	return tcb;
//	Queue *q = tq->threadQueues[status];
//	node_t *node = q->firstNode;
//
//	while(node!=NULL && node->data !=NULL){
//		//TCB *tcb = (TCB *) node->data;
////		if(tcb->thread_id == thread_id) {
////			return tcb;
//		if(node->data->thread_id == thread_id) {
//			return node->data;
//		} else {
//			node = node->childNode;
//		}
//	}
//	return NULL;
//}

TCB * PullThreadFromQueue(ThreadQueue *tq, threadStatus status, int thread_id){
	Queue *q = tq->threadQueues[status];
	node_t *node = q->firstNode;

	while(node!=NULL && node->data !=NULL){
		//TCB *tcb = (TCB *) node->data;
		TCB *tcb = node->data;
		if(tcb->thread_id == thread_id) {
			//This is going to suck... I hope it works.
			bool bIsFirstNode = q->firstNode == node;
			bool bIsLastNode = q->lastNode == node;
			bool bHasAParent = node->parentNode != NULL;
			bool bHasAChild = node->childNode != NULL;

			if(bIsFirstNode && bHasAChild) {
				q->firstNode = q->firstNode->childNode;
				free(q->firstNode->parentNode);
				q->firstNode->parentNode = NULL;
				q->count = q->count -1;
			}
			else if(bIsLastNode && bHasAParent) {
				q->lastNode = q->lastNode->parentNode;
				free(q->lastNode->childNode);
				q->lastNode->childNode = NULL;
				q->count = q->count -1;
			}
			else if(bHasAParent && bHasAChild){
				node->parentNode->childNode = node->childNode;
				node->childNode->parentNode = node->parentNode;
				free(node);
				node=NULL;
			}
			else{
				printf("WTF PullFromQueue");
			}

			return tcb;
		} else {
			node = node->childNode;
		}
	}
	return NULL;
}
