#include <stdio.h>
#include <sys/alt_alarm.h>
#include "alarm_handler.h"
#include "thread_handler.h"


/* a local alarm instance */
static alt_alarm alarm;
static uint32_t tickrate;

/* a global flag for the alarm interrupt */
uint32_t alarmflag = 0;

/* test if "alarmflag" is set */
uint32_t is_alarmflag_set(){
    return alarmflag != 0;
}

/* reset "alarmflag" */
void reset_alarmflag(){
    alarmflag = 0;
}

/* the alarm interrupt handler */
uint32_t myinterrupt_handler(void* context){
    alarmflag = 1;
    if(mythread_isQempty()){
        printf("Interrupted by the DE2 timer!\n");
    }
    return tickrate;
}

uint32_t start_alarm_succeed(uint32_t millis){
	tickrate = ((alt_ticks_per_second()*millis)/1000);
    return alt_alarm_start(&alarm, tickrate , myinterrupt_handler, NULL) >= 0;
}
