/*
 * prototypeOS.h
 *
 *  Created on: Dec 10, 2015
 *      Author: Derek
 */

#ifndef PROTOTYPEOS_H_
#define PROTOTYPEOS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <nios2.h>

#include "alarm_handler.h"

#define PRINT_THREAD_HANDLER_MESSAGES 1

/***************************************************************************
* Interrupts
****************************************************************************/
#define DISABLE_INTERRUPTS() asm("wrctl status, zero");
#define ENABLE_INTERRUPTS() asm("movi et, 1"); asm("wrctl status, et");


/*Disables interrupts if they were not already disabled*/
#define CONDITIONALLY_DISABLE_INTERRUPTS  uint8_t nCondDisable = 0; if(bCheckInterruptsEnabled()){ DISABLE_INTERRUPTS(); nCondDisable = 1; }
/*Enables interrupts only if the prior call to CONDITIONALLY_DISABLE_INTERRUPTS disabled interrupts, else maintain current interrupt status. */
#define CONDITIONALLY_ENABLE_INTERRUPTS if(nCondDisable){ENABLE_INTERRUPTS();}


/***************************************************************************
* Debugging
****************************************************************************/
//#define NDEBUG //Comment in to disable assert messages
#define CHECKMALLOC(x, msg) {					\
	if(x == NULL) {								\
        printf("\nUnable to allocate space!\n");	\
        printf(msg);							\
        exit(1);								\
	}											\
}

#ifdef NDEBUG
#define SHOW_DEBUG_MESSAGES 0
#define CheckForError(bTestPassed, msg)
#else
#define SHOW_DEBUG_MESSAGES 1
#define CheckForError(bTestPassed, msg) if (!bTestPassed && SHOW_DEBUG_MESSAGES) { printf(msg); };
#endif


/***************************************************************************
* PrototypeOS
****************************************************************************/
void prototype_os(void);

/***************************************************************************
* HelperFunctions
****************************************************************************/
int main();
uint8_t bCheckInterruptsEnabled();
void PrintThreadMessage(const char* format, ...);

#endif /* PROTOTYPEOS_H_ */
