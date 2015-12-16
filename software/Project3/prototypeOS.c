#include "prototypeOS.h"
#include "alarm_handler.h"
#include "ThreadHandler.h"
#include "LinkedList.h"
#include "Semaphore.h"
#include <sys/alt_alarm.h>


/* a delay time used to  adjust the frequency of printf messages */
#define MAX 70000
//#define HONEY_DELAY MAX/2
#define BEES 10
#define BEARS 1
#define HONEY_POT_CAPACITY 30
#define BEE_MAX_HONEY_PRODUCTION 15
#define BEARS_MAX_HONEYPOT_CONSUMPTION 5

static LinkedList lsHoneyPot = {HONEY_POT_CAPACITY,0,NULL,NULL};

static MySem *full = NULL;
static MySem *notFull = NULL;
static MySem *mutex = NULL;


uint8_t bCheckInterruptsEnabled(){
	uint8_t status = 0;
	NIOS2_READ_STATUS(status);
	return status;
}

void HoneyBee(char threadID) {
	uint32_t i, j;

	for(i=0; i < BEE_MAX_HONEY_PRODUCTION; i++){

		mysem_down(notFull);
		mysem_down(mutex);

		EnqueueElement(&lsHoneyPot, (void*)(intptr_t) threadID);

		char* byteStream = (char*) LinkedList_ToArray(&lsHoneyPot, 1);
		printf("\n  HoneyBee_%c makes deposit #%02lu into the Honeypot(%02lu):     %-35.*s", threadID, i, lsHoneyPot.count, (int)lsHoneyPot.count, byteStream);
		free(byteStream);

		if(lsHoneyPot.count>= HONEY_POT_CAPACITY){
			mysem_up(full);
		}
		 mysem_up(mutex);
		 for (j = 0; j < MAX ; j++);
	}
}

void Bear(char threadID) {
	uint32_t i, j;

	for(i=0; i < BEARS_MAX_HONEYPOT_CONSUMPTION; i++){
		 mysem_down(full);
		 mysem_down(mutex);

		 printf("\nBear_%c wakes up to eat his %lu Honeypot.  Watch as he snarfs down a pot of honey.", threadID, i);
		 while(lsHoneyPot.count > 0){
			 char atePortionFromHoneyBee = DequeueElement(&lsHoneyPot);
			 char* byteStream = (char*)  LinkedList_ToArray(&lsHoneyPot, 1);
			 printf("\nBear_%c (#%02lu yrs old) eats Bee_%c's Honey! Honeypot(%02lu):    %-35.*s", threadID,i, atePortionFromHoneyBee,lsHoneyPot.count, lsHoneyPot.count, byteStream);
			 free(byteStream);

			 for (j = 0; j < MAX ; j++);

		 }
		 notFull->count=29;
		 mysem_up(notFull);
		 mysem_up(mutex);
	}
}



void OnInterruptHandler(void* context) {
	if(GetActiveThreadCount() == 1){
	    printf("\nInterrupted by the DE2 timer!\n");
	}
}


void prototypeOS(){

	InitializeThreadHandler();

	full =  mysem_create(0, "full");
	notFull = mysem_create(30, "notFull");
	mutex = mysem_create(1, "mutex");

	uint32_t i;
	ThreadControlBlock* thread;

    for (i = 0; i < BEES; i++) {
    	thread = CreateThread(160000, "HoneyBee", HoneyBee);
		printf("Created HoneyBee with id: %c\n", thread->threadID);
		StartThread(thread);
		JoinThread(thread);
    }
	for (i = 0; i < BEARS; i++) {
		thread =CreateThread(160000, "Bear",  Bear);
		printf("Created Bear with id: %c\n", thread->threadID);
		StartThread(thread);
		JoinThread(thread);
	}


   start_alarm(1000, OnInterruptHandler);

    /* an endless while loop */
uint8_t deleted = 0;
    while (1){
        printf ("\nThis is the prototypeOS for my exciting CSE351 course projects! ");
        
        /* delay printf for a while */
        for (i = 0; i < 5*MAX; i++);
        if(GetActiveThreadCount() == 1 && !deleted){
        	deleted++;
            mysem_delete(full);
            mysem_delete(notFull);
            mysem_delete(mutex);
        }
    }
}



/***********************************************************************
* Entry Point
***********************************************************************/
int main() {
	printf("PrototypeOS \n");
	prototypeOS();
	return 0;
}
