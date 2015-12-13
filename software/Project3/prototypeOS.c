#include "prototypeOS.h"
#include "alarm_handler.h"
#include "thread_handler.h"
#include "LinkedList.h"
#include "Semaphore.h"

#define QUANTUM_LENGTH 100 //milliseconds

/* a delay time used to  adjust the frequency of printf messages */
#define MAX 60000
#define HONEY_DELAY MAX/2
#define BEES 10
#define BEARS 1
#define HONEY_POT_CAPACITY 30
#define BEE_MAX_HONEY_PRODUCTION 15
#define BEARS_MAX_HONEYPOT_CONSUMPTION 5


static MySem *full = NULL;
static MySem *notFull = NULL;
static MySem *mutex = NULL;

LinkedList lsHoneyPot = {HONEY_POT_CAPACITY,0,NULL,NULL};

char GetThreadIdentifierChar(uint32_t n) {
  if (n < 10) return '0' + n;
  else if (n < 10 + 26) return 'A' + n - 10;
  else if (n < 10 + 26 + 26) return 'a' + n - 10 - 26;
  return 0;
}


void HoneyBee(char threadID) {
	uint32_t i, j;

	for(i=0; i < BEE_MAX_HONEY_PRODUCTION; i++){

		mysem_down(notFull);
		mysem_down(mutex);

		printf("Honeybee %c, makes deposit #%02lu into ", threadID, i);
		Enqueue(&lsHoneyPot, (void*)(intptr_t) threadID);

		char* byteStream = (char*) LinkedList_ToByteStream(&lsHoneyPot, sizeof(char));
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
			 char atePortionFromHoneyBee = Dequeue(&lsHoneyPot);
			 char* byteStream =(char*) LinkedList_ToByteStream(&lsHoneyPot, sizeof(char));
			 printf("Ate: %c, Honeypot Left: %.*s\n", atePortionFromHoneyBee, lsHoneyPot.count, byteStream);
			 free(byteStream);
			 for (j = 0; j < MAX; j++);
			 mysem_up(notFull);
		 }
		 mysem_up(mutex);
	}
}


void prototypeOS(){
	uint32_t i;

	full =  mysem_create(0, "full");
	notFull = mysem_create(30, "notFull");
	mutex = mysem_create(1, "mutex");

	ThreadControlBlock *thread_pointer;
	ThreadControlBlock *thisThread = mythread_CreateEmptyThread("M");
	thisThread->state = RUNNING;

	uint32_t threadCount = 0;
	char threadID;

	for (i = 0; i < BEARS; i++) {
		threadCount++;
		threadID = GetThreadIdentifierChar(threadCount);
		thread_pointer = mythread_create(threadID, 16000, Bear);
		printf("Created Bear with id: %c\n", thread_pointer->threadID);
        mythread_start(thread_pointer);
        mythread_join(thisThread, thread_pointer);
	}
    for (i = 0; i < BEES; i++) {
    	threadCount++;
    	threadID = GetThreadIdentifierChar(threadCount);
		thread_pointer = mythread_create(threadID, 16000, HoneyBee);
		printf("Created HoneyBee with id: %c\n", thread_pointer->threadID);
        mythread_start(thread_pointer);
        mythread_join(thisThread, thread_pointer);
    }
    
    if ( start_alarm_succeed(QUANTUM_LENGTH) )
        printf ("Start the alarm successfully\n");
    else
        printf ("Unable to start the alarm\n");

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

int main(){
	prototypeOS();
    return 0;
}
