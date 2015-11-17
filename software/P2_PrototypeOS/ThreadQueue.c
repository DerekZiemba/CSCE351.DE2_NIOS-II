#include "P2_PrototypeOS.h"


//void ThreadMan_init() {
////	ThreadManager *tmp = calloc(1,sizeof(ThreadManager));
////	threadman = *tmp;
////	int i = 0;
////	for(i=0; i<numberOfThreadStatuses; i++){
////		threadman.threadQueues[i] = Queue_init(0);//Init to unlimited size queues
////	}
////	threadman.running_thread = calloc(1,sizeof(TCB));
////	threadman.running_thread->thread_id = NULL_THREAD_ID;
////	threadman.running_thread->scheduling_status = NULL_THREAD_ID;
//}

ThreadQueue* ThreadQueue_init() {
	ThreadQueue *tq = calloc(1, sizeof(ThreadQueue));
	alt_u32 i = 0;
	for(i=0; i<numberOfThreadStatuses; i++){
		tq->threadQueues[i] = Queue_init(0);//Init to unlimited size queues
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
	//tcb->scheduling_status = status;
	//Queue_Enqueue(tq->threadQueues[status], (void*) tcb);
	Queue_Enqueue(tq->threadQueues[status], tcb);
}

TCB * DequeueThread(ThreadQueue *tq, threadStatus status){
	return Queue_Dequeue(tq->threadQueues[status]);
//	void* ptr = Queue_Dequeue(tq->threadQueues[status]);
//	if(ptr == NULL){
//		return NULL;
//	}
//	return (TCB *) ptr;
}

TCB * PeekThread(ThreadQueue *tq, threadStatus status){
	return Queue_Peek(tq->threadQueues[status]);
//	void* ptr = Queue_Peek(tq->threadQueues[status]);
//	if(ptr == NULL){
//		return NULL;
//	}
//	return (TCB *) ptr;
}

//TCB *SearchChildNodesByThreadID(node_t* n, int thread_id) {
//	if (n == NULL) return NULL;
//	if (n->data != NULL) {
//		TCB *tcb = (TCB *) n->data;
//		if(tcb->thread_id == thread_id) {
//			return tcb;
//		} else {
//			return SearchChildNodesByThreadID(n->childNode, thread_id);
//		}
//	}
//	return NULL;
//}

TCB * LookupThread(ThreadQueue *tq, threadStatus status, int thread_id){
//	TCB *tcb = SearchChildNodesByThreadID(tq->threadQueues[status]->firstNode, thread_id);
	//Node_Iterator(threadman.threadQueues[status]->firstNode, &lambda, (void*) tcb);
//	return tcb;
	Queue *q = tq->threadQueues[status];
	node_t *node = q->firstNode;

	while(node!=NULL && node->data !=NULL){
		//TCB *tcb = (TCB *) node->data;
//		if(tcb->thread_id == thread_id) {
//			return tcb;
		if(node->data->thread_id == thread_id) {
			return node->data;
		} else {
			node = node->childNode;
		}
	}
	return NULL;
}

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










