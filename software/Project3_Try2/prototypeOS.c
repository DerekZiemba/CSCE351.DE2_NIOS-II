#include "prototypeOS.h"
#include "alarm_handler.h"
#include "ThreadHandler.h"
#include "Semaphore.h"
#include "HoneyPot.h"

/* a delay time used to  adjust the frequency of printf messages */
#define MAX 60000
#define HONEY_DELAY MAX/2
#define BEES 10
#define BEARS 1
#define HONEY_POT_CAPACITY 30
#define BEE_MAX_HONEY_PRODUCTION 15
#define BEARS_MAX_HONEYPOT_CONSUMPTION 5

static HoneyPot lsHoneyPot = {HONEY_POT_CAPACITY,0,NULL,NULL};

static MySem *full = NULL;
static MySem *notFull = NULL;
static MySem *mutex = NULL;


void HoneyBee(char threadID) {
	uint32_t i, j;

	for(i=0; i < BEE_MAX_HONEY_PRODUCTION; i++){

		mysem_down(notFull);
		mysem_down(mutex);

		printf("Honeybee %c, makes deposit #%02lu into ", threadID, i);
		ProduceHoney(&lsHoneyPot,  threadID);

		char* byteStream = HoneyPot_ToArray(&lsHoneyPot);
		printf("Honeypot(%lu): %.*s\n", lsHoneyPot.count, (int)lsHoneyPot.count, byteStream);
		free(byteStream);

		if(lsHoneyPot.count>= HONEY_POT_CAPACITY){
			mysem_up(full);
		}
		 mysem_up(mutex);
		 for (j = 0; j < MAX; j++);
	}
}

void Bear(char threadID) {
	uint32_t i, j;

	for(i=0; i < BEARS_MAX_HONEYPOT_CONSUMPTION; i++){
		 mysem_down(full);
		 mysem_down(mutex);

		 printf("This is bear %c and it's his %lu springtime.  Watch as he snarfs down a pot of honey. \n", threadID, i);
		 while(lsHoneyPot.count > 0){
			 char atePortionFromHoneyBee = ConsumeHoney(&lsHoneyPot);
			 char* byteStream = HoneyPot_ToArray(&lsHoneyPot);
			 printf("Ate: %c, Honeypot(%lu) Left: %.*s\n", atePortionFromHoneyBee,lsHoneyPot.count, (int)lsHoneyPot.count, byteStream);
			 free(byteStream);
			 for (j = 0; j < MAX; j++);
			 mysem_up(notFull);
		 }
		 mysem_up(mutex);
	}
}


void OnInterruptHandler(void* context) {
	if(GetActiveThreadCount() == 1){
	    printf("Interrupted by the DE2 timer!\n");
	}
}

void prototypeOS(){

	InitializeThreadHandler();

	full =  mysem_create(0, "full");
	notFull = mysem_create(30, "notFull");
	mutex = mysem_create(1, "mutex");

	uint32_t i;

	ThreadControlBlock *thread_pointer;

	for (i = 0; i < BEARS; i++) {
		thread_pointer = CreateThread(64000,"Bear",  Bear);
		printf("Created Bear with id: %c\n",thread_pointer->threadID);
		StartThread(thread_pointer);
		JoinThread(thread_pointer);
	}
    for (i = 0; i < BEES; i++) {
		thread_pointer = CreateThread(64000,"HoneyBee", HoneyBee);
		printf("Created HoneyBee with id: %c\n", thread_pointer->threadID);
		StartThread(thread_pointer);
		JoinThread(thread_pointer);
    }
    

    CheckForError(start_alarm(QUANTUM_LENGTH, OnInterruptHandler), "Unable to start the alarm\n");

    /* an endless while loop */
    while (1){
        printf ("This is the OS primitive for my exciting CSE351 course projects!\n");
        
        /* delay printf for a while */
        for (i = 0; i < 10*MAX; i++);
        mysem_delete(full);
        mysem_delete(notFull);
        mysem_delete(mutex);
    }
}



