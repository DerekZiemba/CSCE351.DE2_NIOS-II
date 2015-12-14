/*
 * LinkedList.c
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek
 */

#include "LinkedList.h"
#include <stdlib.h>
#include <stdio.h>

/*******************************************************************************************************
* These are internal macros for the purpose of making code easier to read
*******************************************************************************************************/
#define GetByteAt(dataPtr,byteNumber) (uint8_t)*((uintptr_t*)(&(dataPtr)) + byteNumber)

#define bIsListUnlimitedSize(ls) (ls->maxsize==0)
#define bListHasNodes(ls) (ls->count > 0)
/*If the pointer is invalid usually these will be wrong*/
#define bIsValidList(ls) ((ls->count >= 0 && ls->maxsize >= 0) && (bIsListUnlimitedSize(ls) ? 1 : ls->count <= ls->maxsize) )
#define bListFull(ls) (!bIsListUnlimitedSize(ls) && ls->count >= ls->maxsize)
#define bIsCloserToBeginning(ls,index) (index <= (ls->count - index))
#define bIsValidInsertAtIndex(ls,index) ((index >= 0 && index <= ls->count) && (bIsListUnlimitedSize(ls) ? 1 : index <= ls->maxsize))
#define bIsValidGetAtIndex(ls,index) (index >= 0 && index < ls->count)


/*******************************************************************************************************
* The code section is influenced by the tunables specified in the header.
* They are defined so that there is no overhead to call a function when not necessary
*******************************************************************************************************/
#if PRINT_ERRORS_TO_STDOUT == 1
uint8_t _PrintErrorTostdout(uint8_t bTestPassed, char* msg) {
	if (!bTestPassed && PRINT_ERRORS_TO_STDOUT) { printf(msg); };
	return bTestPassed;
}
#define CheckErrorAndPrint(bTestPassed, msg) _PrintErrorTostdout(bTestPassed,msg)
#else
#define CheckErrorAndPrint(bTestPassed, msg) 1 //Will always just true otherwise
#endif


#if VERIFY_OPERATIONS_VALID == 1
uint8_t _VerifyViableInsertOperation(LinkedList* ls, uint32_t index) {
	return CheckErrorAndPrint(bIsValidList(ls), "ERROR: List is CORRUPT\n") ? (
		CheckErrorAndPrint(!bListFull(ls), "ERROR: List is Full\n") ?
			CheckErrorAndPrint(bIsValidInsertAtIndex(ls, index), "ERROR: List is Full\n") : 0) : 0;
}

uint8_t _VerifyViableGetOperation(LinkedList* ls, uint32_t index) {
	return CheckErrorAndPrint(bIsValidList(ls), "ERROR: List is CORRUPT\n") ? (
		CheckErrorAndPrint(bListHasNodes(ls), "ERROR: List is Empty.  Null Ptr Returned\n") ?
			CheckErrorAndPrint(bIsValidGetAtIndex(ls, index), "ERROR: No node exists at index\n") : 0) : 0;
}

#define VerifyViableInsertOperation(ls,index) _VerifyViableInsertOperation(ls, index)
#define VerifyViableGetOperation(ls,index) _VerifyViableGetOperation(ls, index)
#else
#define VerifyViableInsertOperation(ls,index)  1 //Will always just true otherwise
#define VerifyViableGetOperation(ls,index)  1 //Will always just true otherwise
#endif

#define CHECKMALLOC(x, msg) {					\
	if(x == NULL) {								\
        printf("\nUnable to allocate space!\n");	\
        printf(msg);							\
        exit(1);								\
	}											\
}


/*******************************************************************************************************
* Node Functions
*******************************************************************************************************/
node_t* CreateNewNode(void *data) {
	node_t  *n = (node_t*) malloc(sizeof(node_t));
	CHECKMALLOC(n,"LinkedList Node");
	n->data = data;
	n->parentNode = NULL;
	n->childNode = NULL;
	return n;
}

//Gets the node at the index.  Do not free this node without splicing the list.
node_t* GetNodeAtIndex(LinkedList* ls, const uint32_t index) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		if (VerifyViableGetOperation(ls, index)) {
			node_t* node = NULL;
			uint32_t i = 1;
			if (bIsCloserToBeginning(ls, index)) {
				node = ls->firstNode;
				for (i = 1; i <= index; i++) {
					node = node->childNode;
				}
				return node;
			}
			else {
				node = ls->lastNode;
				uint32_t stop = ls->count - index;
				for (i = 1; i <= stop; i++) { node = node->parentNode; }
				return node;
			}
		}
	}
	return NULL;
}

node_t* GetNodeByElement(LinkedList* ls,  void* elementRef) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		node_t* node = ls->firstNode;
		while (node != NULL) {
			if (node->data == elementRef) {
				return node;
			}
			node = node->childNode;
		}
	}
	return NULL;
}

//Removes the node and splices the list. Remember to free the node if no longer used.
node_t* PullNode(LinkedList* ls, node_t* node) {
	if (node != NULL) {
		if (ls->count > 1) {
			if (node->parentNode == NULL) { //If this is the first node
				if (node->childNode != NULL) { //And it has a child
					ls->firstNode = node->childNode;
					ls->firstNode->parentNode = NULL; //Set the reference to this node to null
				}
			}
			else if (node->childNode == NULL) { //If this is the last node
				ls->lastNode = node->parentNode;
				ls->lastNode->childNode = NULL; //Set the reference to this node to null
			}
			else { //We gotta splice the nodes
				node->childNode->parentNode = node->parentNode;
				node->parentNode->childNode = node->childNode;
			}
			ls->count--;
		}
		else if(ls->firstNode == node) {
			ls->firstNode = NULL;
			ls->lastNode = NULL;
			ls->count--;
		}

		return node;
	}
	return NULL;
}


node_t* InsertNode(LinkedList* ls, uint32_t index, node_t* newNode) {
	if (VerifyViableInsertOperation(ls, index)) {
		//First things first, wipe out any references to any other parent or child
		newNode->parentNode = NULL;
		newNode->childNode = NULL;

		if (ls->count == 0) { //First node to go into the list.  It is both the first and last node.
			ls->firstNode = newNode;
			ls->lastNode = ls->firstNode;
		}
		else if (index == 0) { //We are inserting it at the beginning.
			newNode->childNode = ls->firstNode;
			ls->firstNode->parentNode = newNode;
			ls->firstNode = newNode;
		}
		else if (index >= ls->count ) { //We are appending the node to the end
			newNode->parentNode = ls->lastNode;
			ls->lastNode->childNode = newNode;
			ls->lastNode = newNode;
		}
		else {
			//He's gonna get pushed down 1 slot.
			node_t* victim = GetNodeAtIndex(ls,index);
			newNode->parentNode = victim->parentNode;
			newNode->parentNode->childNode = newNode;
			newNode->childNode = victim;
			victim->parentNode = newNode;
		}
		ls->count++;
	}
	return newNode;
}

/*returns index of foundNode from the rootnode*/
int32_t GetNodeIndexByElement(node_t* rootNode,  void* elementRef) {
	node_t* node = rootNode;
	uint32_t index = 0;
	while (node != NULL) {
		if (node->data == elementRef) {
			return index;
		}
		node = node->childNode;
		index++;
	}
	return -1;
}

int32_t GetElementIndex(LinkedList* ls, void* element) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		return GetNodeIndexByElement(ls->firstNode, element);
	}
	return -1;
}


void* PullAndFreeNode(LinkedList* ls, node_t* node){
	void* elementRef = NULL;
	if(node != NULL){
		elementRef = node->data;
		node = PullNode(ls, node);
		free(node);
		node = NULL;
	}
	return elementRef;
}


/*******************************************************************************************************
* Internal List Functions
*******************************************************************************************************/
void* m_GetElementPullNodeAndFree(LinkedList* ls, node_t* node){
	void* elementRef = NULL;
	if(node != NULL){
		elementRef = node->data;
		node = PullNode(ls, node);
		free(node);
		node = NULL;
	}
	return elementRef;
}

/*******************************************************************************************************
* List Functions
*******************************************************************************************************/
LinkedList* LinkedList_CreateNew(uint32_t max_size) {
	LinkedList *ls = malloc(sizeof(LinkedList));
	CHECKMALLOC(ls,"LinkedList");
	ls->maxsize = max_size;
	ls->count = 0;
	ls->firstNode = NULL;
	ls->lastNode = NULL;
	return ls;
}

void LinkedList_Free(LinkedList* ls) {
	node_t* node = ls->firstNode;
	while(node != NULL){
		free(node);
		node = NULL;
	}
	free(ls);
	ls = NULL;
}

//This creates a new node and adds the value to it
void InsertElementAtIndex(LinkedList* ls, uint32_t index, void* value) {
	InsertNode(ls, index, CreateNewNode(value));
}

//This does not remove the value from the list.
void* GetElementAtIndex(LinkedList* ls, uint32_t index) {
	node_t* node = GetNodeAtIndex(ls, index);
	return node == NULL ? NULL : node->data;
}





void* PullElementByReference(LinkedList* ls, void* element) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		uint32_t index = GetNodeIndexByElement(ls->firstNode,  element);
		if(index >= 0) {
			node_t* node = GetNodeAtIndex(ls, index);
			return m_GetElementPullNodeAndFree(ls, node);
		}
	}
	return NULL;
}



//This removes the value from the list and frees the node.
void* PullElementAtIndex(LinkedList* ls, uint32_t index) {
	node_t* node =  GetNodeAtIndex(ls, index);
	if (node != NULL) {
		return m_GetElementPullNodeAndFree(ls,node);
	}
	return NULL;
}


//Mallocs bytes, be sure to free later.
uint8_t* LinkedList_ToByteStream(const LinkedList* ls, const int elementByteSize) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		uint8_t* buffer = malloc(ls->count * elementByteSize);
		node_t* node = ls->firstNode;
		int offset = 0;
		while (node != NULL) {
			int byteNum = 0;
			for (byteNum = 0; byteNum < elementByteSize; byteNum++) {
				buffer[offset] = GetByteAt(node->data, byteNum);
				offset++;
			}
			node = node->childNode;
		}
		return buffer;
	}
	return NULL;
}

/*******************************************************************************************************
* Queue Functions
*******************************************************************************************************/
//Appends to end of list
void EnqueueNode(LinkedList* ls, node_t* node) {
	InsertNode(ls, ls->count, node);
}

//Pull from start of list.
node_t* DequeueNode(LinkedList* ls) {
	return PullNode(ls, GetNodeAtIndex(ls, 0));
}

node_t* PeekNode(LinkedList* ls) {
	return GetNodeAtIndex(ls, 0);
}

//Appends to end of list
void Enqueue(LinkedList* ls, void* value) {
	EnqueueNode(ls, CreateNewNode(value));
}

//Pull from start of list.
void* Dequeue(LinkedList* ls) {
	return PullElementAtIndex(ls, 0);
}

//View first element
void* Peek(LinkedList* ls) {
	return GetElementAtIndex(ls, 0);
}













