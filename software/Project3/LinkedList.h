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
uint8_t* 	LinkedList_ToByteStream(const LinkedList* ls, const int elementByteSize);

void		InsertValueAtIndex(LinkedList* ls, uint32_t index, void* value);
void*		GetValueAtIndex(LinkedList* ls, uint32_t index); //The value will remains in the list
void*		PullValueAtIndex(LinkedList* ls, uint32_t index); 	//Deleted the node and returns the value contained in it.

/*A Doubly linked list isn't the most optimal for a queue.  But I'm going for re-usability here. */
void		Enqueue(LinkedList* ls, void* value); //Automatically Allocates node. Returns the newly allocated node.
void*		Dequeue(LinkedList *q); //Automatically Frees node. Returns ptr to stored object
void*		Peek(LinkedList *q); //Returns a reference to the next element to that would be normally dequeued
void* 		PeekTail(LinkedList* ls);

/*A Doubly linked list isn't the most optimal for a stack.  But I'm going for re-usability here. */
void		Push(LinkedList* ls, void* value); //Automatically Allocates node. Returns the newly allocated node.
void*		Pop(LinkedList* ls); //Automatically Frees node. Returns ptr to stored object




#endif /* LINKEDLIST_H_ */
