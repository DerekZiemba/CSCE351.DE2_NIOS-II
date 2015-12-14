/*
 * HelperFunctions.c
 *
 *  Created on: Dec 14, 2015
 *      Author: Derek
 */


#include "prototypeOS.h"


uint8_t bCheckInterruptsEnabled(){
	uint8_t status = 0;
	NIOS2_READ_STATUS(status);
	return status;
}



void PrintThreadMessage(const char* format, ...){
#if PRINT_THREAD_HANDLER_MESSAGES == 1
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
#endif
}

/***********************************************************************
* Entry Point
***********************************************************************/
int main() {
	printf("PrototypeOS ");
	prototypeOS();
	return 0;
}



