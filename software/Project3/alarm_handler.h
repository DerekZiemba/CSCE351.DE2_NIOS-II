/* 
    Name:   alarm_handler.h
    Author: Dongyuan Zhan
    Date:   11/20/2010

Description:
    Functions used to handle things related to alarm interrupts
*/
#include <stdint.h>

uint32_t is_alarmflag_set();

void reset_alarmflag();

uint32_t myinterrupt_handler(void* context);

uint32_t start_alarm_succeed(uint32_t millis);
