/*
 * HoneyPot.h
 *
 *  Created on: Dec 14, 2015
 *      Author: Derek
 */

#ifndef HONEYPOT_H_
#define HONEYPOT_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <nios2.h>

/***************************************************************************
* HoneyPot
****************************************************************************/
typedef struct HoneyPotNode_t {
	struct HoneyPotNode_t*	parentNode;
	struct HoneyPotNode_t*	childNode;
	void* data;
} HoneyPotNode_t;


typedef struct HoneyPot {
	int32_t	maxsize; //0 for unlimited
	int32_t	count;
	HoneyPotNode_t*		firstNode;
	HoneyPotNode_t*		lastNode;
} HoneyPot;

HoneyPot* HoneyPot_CreateNew(int32_t max_size);
void ProduceHoney(HoneyPot* ls, char* value);
char* ConsumeHoney(HoneyPot* ls);
char* HoneyPot_ToArray(const HoneyPot* ls);

#endif /* HONEYPOT_H_ */
