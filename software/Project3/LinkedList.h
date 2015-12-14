/*
 * LinkedList.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek Ziemba
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdint.h>

#define PRINT_ERRORS_TO_STDOUT 1
#define VERIFY_OPERATIONS_VALID 1

typedef struct node_t {
	void*			data;
	struct node_t*	parentNode;
	struct node_t*	childNode;
} node_t;


typedef struct LinkedList_t {
	uint32_t	maxsize; //0 for unlimited
	uint32_t	count;
	node_t*		firstNode;
	node_t*		lastNode;
} LinkedList;

LinkedList* LinkedList_CreateNew(uint32_t max_size);
void 		LinkedList_Free(LinkedList* ls);
uint8_t* 	LinkedList_ToArray(const LinkedList* ls, const int elementByteSize);

node_t* 	CreateNewNode(void *data);
node_t* 	GetNodeAtIndex(LinkedList* ls, const uint32_t index);
node_t* 	PullNode(LinkedList* ls, node_t* node);
node_t* 	InsertNode(LinkedList* ls, uint32_t index, node_t* newNode);
node_t* 	GetNodeByElement(LinkedList* ls, void* element);

int32_t 	GetNodeIndexByElement(node_t* rootNode,  void* elementRef);
int32_t		GetElementIndex(LinkedList* ls, void* element);

LinkedList* LinkedList_CreateNew(uint32_t max_size);
void 		LinkedList_Free(LinkedList* ls);
uint8_t* 	LinkedList_ToByteStream(const LinkedList* ls, const int elementByteSize);

void* 		PullAndFreeNode(LinkedList* ls, node_t* node);



void		InsertElementAtIndex(LinkedList* ls, uint32_t index, void* value);
void*		GetElementAtIndex(LinkedList* ls, uint32_t index); //The value will remains in the list
void*		PullElementAtIndex(LinkedList* ls, uint32_t index); 	//Deleted the node and returns the value contained in it.

void* 		PullElementByReference(LinkedList* ls, void* element);
void* 		PullElementByValue(LinkedList* ls, void* element, uint32_t elementByteSize);


void 		EnqueueNode(LinkedList* ls, node_t* node);
node_t* 	DequeueNode(LinkedList* ls);
node_t* 	PeekNode(LinkedList* ls);

void		EnqueueValue(LinkedList* ls, void* value); //Automatically Allocates node. Returns the newly allocated node.
void*		DequeueValue(LinkedList *q); //Automatically Frees node. Returns ptr to stored object
void*		PeekValue(LinkedList *q); //Returns a reference to the next element to that would be normally dequeued






#endif /* LINKEDLIST_H_ */
