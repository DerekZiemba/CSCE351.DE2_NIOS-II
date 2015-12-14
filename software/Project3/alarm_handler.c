#include <stdio.h>
#include <sys/alt_alarm.h>
#include "alarm_handler.h"

/* The resolution is 100ms but it accepts input in single millis */
#define ALARMTICKS(x) ((alt_ticks_per_second()*x)/1000)

/* a local alarm instance */
static alt_alarm alarm;
static alt_alarm forcedAlarm;

static uint32_t tickrate;
static uint8_t alarmflag = 0;

uint8_t is_alarmflag_set(){ return alarmflag; }
void reset_alarmflag(){ alarmflag = 0; }

void (*onInterruptCallback) (void* context);

/* the alarm interrupt handler */
uint32_t myinterrupt_handler(void* context){
    alarmflag = 1;
    onInterruptCallback(context);
    return tickrate;
}

uint8_t start_alarm(uint32_t millis, void (*callback) (void* context)){
	tickrate = ALARMTICKS(millis);
	onInterruptCallback = callback;
    return alt_alarm_start(&alarm, tickrate , myinterrupt_handler, NULL) >= 0;
}



uint32_t forceInterruptHandler(void* context){
	alarmflag = 0;
	alt_alarm_stop(&forcedAlarm);
    return 0;
}

void forceInterrupt(){
	alarmflag = 1;
    alt_alarm_start(&forcedAlarm, 1 , forceInterruptHandler, NULL);
}

