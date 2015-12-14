/*
 * LinkedList.c
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek
 */

#include "HoneyPot.h"

/*******************************************************************************************************
* These are internal macros for the purpose of making code easier to read
*******************************************************************************************************/
#define NodeType 	HoneyPotNode_t
#define ElementType char
#define ListType 	HoneyPot

#define bIsListUnlimitedSize(ls) (ls->maxsize==0)
#define bListHasNodes(ls) (ls->count > 0)
/*If the pointer is invalid usually these will be wrong*/
#define bIsValidList(ls) ((ls->count >= 0 && ls->maxsize >= 0) && (bIsListUnlimitedSize(ls) ? 1 : ls->count <= ls->maxsize) )
#define bListFull(ls) (!bIsListUnlimitedSize(ls) && ls->count >= ls->maxsize)
#define bIsCloserToBeginning(ls,index) (index <= (ls->count - index))
#define bIsValidInsertAtIndex(ls,index) ((index >= 0 && index <= ls->count) && (bIsListUnlimitedSize(ls) ? 1 : index <= ls->maxsize))
#define bIsValidGetAtIndex(ls,index) (index >= 0 && index < ls->count)

uint8_t CheckErrorAndPrint(uint8_t bTestPassed, char* msg) {
	if (!bTestPassed) { printf(msg); }
	return bTestPassed;
}

uint8_t VerifyViableInsertOperation(ListType* ls, int32_t index) {
	return CheckErrorAndPrint(bIsValidList(ls), "ERROR: List is CORRUPT\n") ? (
		CheckErrorAndPrint(!bListFull(ls), "ERROR: List is Full\n") ?
			CheckErrorAndPrint(bIsValidInsertAtIndex(ls, index), "ERROR: List is Full\n") : 0) : 0;
}

uint8_t VerifyViableGetOperation(ListType* ls, int32_t index) {
	return CheckErrorAndPrint(bIsValidList(ls), "ERROR: List is CORRUPT\n") ? (
		CheckErrorAndPrint(bListHasNodes(ls), "ERROR: List is Empty.  Null Ptr Returned\n") ?
			CheckErrorAndPrint(bIsValidGetAtIndex(ls, index), "ERROR: No node exists at index\n") : 0) : 0;
}




/*******************************************************************************************************
* Node Functions
*******************************************************************************************************/
NodeType* CreateNewThreadNode(ElementType* data) {
	NodeType  *n = malloc(sizeof(NodeType));
	n->data = data;
	n->childNode = NULL;
	return n;
}

ElementType* FreeNode(NodeType* node) {
	ElementType* thread = node->data;
	free(node);
	node->data = NULL;
	node->childNode = NULL;
	node->parentNode = NULL;
	return thread;
}

NodeType* GetNodeAtIndex(ListType* ls, const int32_t index) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		if (VerifyViableGetOperation(ls, index)) {
			NodeType* node = NULL;
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

NodeType* GetNodeByElement(ListType* ls,  ElementType* elementRef) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		NodeType* node = ls->firstNode;
		while (node != NULL) {
			if (node->data == elementRef) {
				return node;
			}
			node = node->childNode;
		}
	}
	return NULL;
}

int32_t GetNodeIndexByElement(NodeType* rootNode,  ElementType* elementRef) {
	NodeType* node = rootNode;
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

NodeType* InsertNode(ListType* ls, int32_t index, NodeType* newNode) {
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
			NodeType* victim = GetNodeAtIndex(ls, index);
			newNode->parentNode = victim->parentNode;
			newNode->parentNode->childNode = newNode;
			newNode->childNode = victim;
			victim->parentNode = newNode;
		}
		ls->count++;
	}
	return newNode;
}





//Removes the node and splices the list. Remember to free the node if no longer used.
NodeType* PullNode(ListType* ls, NodeType* node) {
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

ElementType* PullElementAndFreeNode(ListType* ls, NodeType* node){
	ElementType* elementRef = NULL;
	if(node != NULL){
		elementRef = node->data;
		node = PullNode(ls, node);
		free(node);
		node = NULL;
	}
	return elementRef;
}

ElementType* PullElementByReference(ListType* ls, ElementType* element) {
	return PullElementAndFreeNode(ls, GetNodeByElement(ls, element) );
}

ElementType* HoneyPot_ToArray(const ListType* ls) {
	if (bIsValidList(ls) && bListHasNodes(ls)) {
		uint16_t elementByteSize = sizeof(ElementType);
		uint8_t* buffer = malloc(ls->count * elementByteSize);
		NodeType* node = ls->firstNode;
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
* List Functions
*******************************************************************************************************/
ListType* HoneyPot_CreateNew(int32_t max_size){
	ListType *ls = malloc(sizeof(ListType));
	ls->maxsize = max_size;
	ls->count = 0;
	ls->firstNode = NULL;
	ls->lastNode = NULL;
	return ls;
}

void ProduceHoney(ListType* ls, ElementType* value){
	InsertNode(ls, ls->count, CreateNewNode(value));
}

ElementType* ConsumeHoney(ListType* q) {
	return PullElementAndFreeNode(q, GetNodeAtIndex(q, 0));
}














