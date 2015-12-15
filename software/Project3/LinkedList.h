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


/*******************************************************************************************************
* Node Functions.
*******************************************************************************************************/
node_t* 	Node_CreateNew(void *data);

			/* Inserts the node at the child that is index away from the first element. */
node_t* 	Node_InsertNode(LinkedList* ls, uint32_t index, node_t* newNode);

			/*Returns the node at the index given from the firstnode in the list. */
node_t* 	Node_GetNodeAtIndex(LinkedList* ls, const uint32_t index);
			/* Gets the first child node that matches the element pointer.  */
node_t* 	Node_GetNodeByElement(LinkedList* ls,  void* elementRef);

			/* Removes the node, splices the parent and child, and returns the removed node.  Node must be later freed.  */
node_t* 	Node_RemoveNode(LinkedList* ls, node_t* node);


/*******************************************************************************************************
* List FUnctions
*******************************************************************************************************/
void 		LinkedList_Free(LinkedList* ls);
LinkedList* LinkedList_CreateNew(uint32_t max_size);

			/*this needs more testing... It works for char's but I think that's it.  Basically allows you to get an array of chars*/
uint8_t* 	LinkedList_ToByteStream(const LinkedList* ls, const int elementByteSize);

void		LinkedList_InsertElementAtIndex(LinkedList* ls, uint32_t index, void* value);

			/*Returns a reference to the element at the specified index*/
void*		LinkedList_GetElementAtIndex(LinkedList* ls, uint32_t index); //The value will remains in the list
			/*Gets the index of an element*/
int32_t		LinkedList_GetElementIndex(LinkedList* ls, void* element);

			/*Removes the first instance of the element*/
void* 		LinkedList_RemoveElement(LinkedList* ls, void* element);
void*		LinkedList_RemoveElementAtIndex(LinkedList* ls, uint32_t index);

/*******************************************************************************************************
* Convenience Functions
*******************************************************************************************************/
			/*Inserts the element at the end of the list*/
void 		EnqueueElement(LinkedList* ls, void* element);
			/*Removes the element from the front of the list*/
void* 		DequeueElement(LinkedList* ls);
			/*Removes the element from the front of the list*/
void 		PushElement(LinkedList* ls, void* element);
			/*Gets the element from the front of the list*/
void* 		PeekElement(LinkedList* ls);
			/*Removes all instances of the element.  Returns the element if successful.*/
void* 		RemoveElement(LinkedList* ls, void* element);

/*The first element goes to the end and the second element becomes the first. Returns the new first element*/
void* 		RotateWrapAndPeek(LinkedList* ls);


#endif /* LINKEDLIST_H_ */
