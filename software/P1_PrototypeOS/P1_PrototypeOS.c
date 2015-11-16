
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys/alt_stdio.h"
#include "sys/alt_alarm.h"
#include "alt_types.h"


/*For some reason these weren't already specified?*/
typedef enum { true = 1, false = 0 } bool;

/* The resolution is 100ms but it accepts input in single millis */
#define ALARMTICKS(x) ((alt_ticks_per_second()*x)/1000)

/***********************************************************************
* Static Variables
***********************************************************************/
/* Note: Made it a variable so I can change it in the debugger.  */
static alt_u32 QUANTUM_LENGTH = 500;
static alt_u32 MAX = 110633;

static alt_alarm alarm;

/***********************************************************************
* Function Declarations
***********************************************************************/
alt_u32 myinterrupt_handler(void* context);
void prototype_os(void);

/***********************************************************************
* Entry Point
***********************************************************************/
int main() { prototype_os(); return 0; }


void prototype_os(void) {
	printf("Started.%lu\n", ALARMTICKS(QUANTUM_LENGTH));
	alt_u32 j = 0;

	// Here: initialize the timer and its interrupt handler
	alt_alarm_start(&alarm, ALARMTICKS(QUANTUM_LENGTH), myinterrupt_handler, (void*)(&j));

	while (true) {
		alt_printf("This is the prototype os for my exciting CSE351 course projects!\n");

		//Here: think about what MAX is used for. Pick up an appropriate value for it experimentally.
		for (j = 0; j < MAX; j++) {
			//With 110633 as the Max and no nop's, the interrupt interrupts right in the middle of printing the above sentence.
			asm("nop"); //One nop reduces collisions
			asm("nop");  //further reduces collisions.
		}
	}
}

/*
* So I originally copied and pasted this directly out of the pdf.  Well apparently one of the characters was unicode or something.
*  I spent 4 hours trying to get past a "myinterrupt_handler is not declared and is first used here" error.
*  I changed the name and it suddenly started working.
*/
alt_u32 myinterrupt_handler(void* context) {
	alt_u32 counter = *(alt_u32 *)context;
	/* This function is called once per second by default.  It is a callback function */
	printf("Interrupted! Elapsed=%lu ticks, Period=%lu, Loop Iterations: %lu.\n", alt_nticks(), ALARMTICKS(QUANTUM_LENGTH), counter);
	//j=0; // It's able to sometimes beat the interrupt.  It averages 205,240 cycles.
	return ALARMTICKS(QUANTUM_LENGTH);
}
