/*
 * LinkedList.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek Ziemba
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdint.h>
#include <stdlib.h>

#define PRINT_ERRORS_TO_STDOUT 1
#define VERIFY_OPERATIONS_VALID 1

typedef struct node_t {
	void*			data;
	struct node_t*	parentNode;
	struct node_t*	childNode;
} node_t;


typedef struct LinkedList {
/*Set to none-zero value to enforce a maxsize limit*/
	uint32_t	maxsize; //0 for unlimited
/*The number of nodes currently allocated*/
	uint32_t	count;
	node_t*		firstNode;
	node_t*		lastNode;
} LinkedList;


/*******************************************************************************************************
* Node Functions.
*******************************************************************************************************/
node_t* 	Node_CreateNew(void *data);

			/*Returns the node at the index given from the firstnode in the list. */
node_t* 	Node_GetNodeAtIndex(LinkedList* ls, const size_t index);
			/* Gets the first child node that matches the element pointer.  */
node_t* 	Node_GetNodeByElement(LinkedList* ls, const  void* elementRef);

			/* Removes the node, splices the parent and child, and returns the removed node.  Node must be later freed.  */
node_t* 	Node_RemoveNode(LinkedList* ls, node_t* node);

/* Inserts the node at the child that is index away from the first element. */
node_t* 	Node_InsertNode(LinkedList* ls, const size_t index, node_t* newNode);

/* Returns an array of pointers where the first element is the rootnode and the last element is the (n)th child (count). */
uintptr_t*	Node_ChildrenToArray(node_t* rootnode, size_t element_byte_size, size_t count);

/*******************************************************************************************************
* List FUnctions
*******************************************************************************************************/
void 		LinkedList_Free(LinkedList* ls);

			/*Pass in 0 for unlimited size.  Else if the list goes over max_size an error will be thrown*/
LinkedList* LinkedList_CreateNew(size_t max_size);

/*Pass in 0 for unlimited size.  Else if the list goes over max_size an error will be thrown*/
LinkedList* LinkedList_CreateNewFromArray(uint8_t* the_array, size_t element_byte_size, size_t start_index, size_t count);

uint8_t* 	LinkedList_ToArray(LinkedList* ls, int elementByteSize);

void		LinkedList_InsertElementAtIndex(LinkedList* ls, size_t index, void* value);

			/*Returns a reference to the element at the specified index*/
void*		LinkedList_GetElementAtIndex(LinkedList* ls, size_t index); //The value will remains in the list
			/*Gets the index of an element*/
size_t		LinkedList_GetElementIndex(LinkedList* ls, void* element);

			/*Removes the first instance of the element*/
void* 		LinkedList_RemoveElement(LinkedList* ls, void* element);
void*		LinkedList_RemoveElementAtIndex(LinkedList* ls, size_t index);

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
			/*Removes instance of the element.  Returns the element if successful.*/
void* 		RemoveElement(LinkedList* ls, void* element);

#endif /* LINKEDLIST_H_ */
