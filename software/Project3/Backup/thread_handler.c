//#include "prototypeOS.h"
//#include "alarm_handler.h"
//#include "thread_handler.h"
//#include "LinkedList.h"
//#include <sys/alt_alarm.h>
//
//
//
//
//static ThreadControlBlock *current_running_thread      = NULL;
//
//static uint32_t *main_stack_pointer = NULL;
//
//static LinkedList lsReadyThreads = {0,0,NULL,NULL};
//
//
///* pointing to the stack/context of main() */
//
//
//ThreadControlBlock* GetCurrentRunningThread(){
//	return current_running_thread;
//}
//
//ThreadControlBlock *mythread_CreateDummyThread(char threadID){
//	ThreadControlBlock *thread	= malloc(sizeof(ThreadControlBlock));
//	CHECKMALLOC(thread, "ThreadControlBlock");
//	thread->threadID = threadID;
//	thread->state = DUMMY;
//	thread->ParentThread = NULL;
//	thread->lsChildThreads = LinkedList_CreateNew(0);
//	thread->stack = NULL;
//	thread->sp = NULL;
//    //    thread->threadStats = malloc(sizeof(ThreadStats_t));
//    //    thread->threadStats->totalTicks = 0;
//    //    thread->threadStats->lastStartTicks = alt_nticks();
//    //    thread->threadStats->startTicks = alt_nticks();
//	return thread;
//}
//ThreadControlBlock *mythread_create(char threadID, uint32_t stackBytes, void (*funcptr)(char threadID)){
//    ThreadControlBlock *thread	= mythread_CreateDummyThread(threadID);
//    thread->state    = NEW;
//    thread->stack = malloc(stackBytes);
//    CHECKMALLOC(thread->stack, "thread->stack");
//
//    thread->sp       = (uint32_t *)(thread->stack + stackBytes/4 - 19);
//    thread->sp[18]   = (uint32_t)funcptr;							// ea
//    thread->sp[17]   = 1;											// estatus
//    thread->sp[5]    = threadID;									// r4
//    thread->sp[0]    = (uint32_t)mythread_cleanup;					// ra
//    thread->sp[-1]   = (uint32_t)(thread->stack + stackBytes/4);	// fp
//    return thread;
//}
//
///* NEW ----> READY */
//void mythread_start(ThreadControlBlock *thread_pointer){
//    thread_pointer->state = READY;
//}
//
///* READY --push into--> readyQ */
//void mythread_join(ThreadControlBlock *parent, ThreadControlBlock *readyThread){
//	DISABLE_INTERRUPTS();
//    CheckForError(readyThread && readyThread->state == READY,"\nNot Ready To Join\n");
//
//    Enqueue(&lsReadyThreads, (void *)readyThread);
//    register void* sp asm ("sp");
//
//    if(readyThread->ParentThread != NULL){
//    	LinkedList* ls = readyThread->ParentThread->lsChildThreads;
//
//    }
//    readyThread->ParentThread = parent;
//    Enqueue(parent->lsChildThreads, (void*) readyThread);
//
//    ENABLE_INTERRUPTS();
//}
//
///* RUNNING ----> BLOCKED */
//void mythread_block(ThreadControlBlock *thread_pointer){
//    CheckForError(thread_pointer && thread_pointer->state == RUNNING,"\nThread Not Running\n");
//    thread_pointer->state = BLOCKED;
//}
//
///* RUNNING ----> TERMINATED */
//void mythread_terminate(ThreadControlBlock *thread_pointer){
//    CheckForError(thread_pointer && thread_pointer->state == RUNNING,"\nThread Not Running\n");
//    thread_pointer->state = TERMINATED;
//}
//
//void *mythread_schedule(void *context){
//
//
//    if (lsReadyThreads.count > 0){
//        if (current_running_thread != NULL){
//            if(current_running_thread->state == RUNNING){
//            	current_running_thread->state = READY;
//            }
//            current_running_thread->sp = (uint32_t *)context;
//            Enqueue(&lsReadyThreads, (void*) current_running_thread);
//        }
//        else if (main_stack_pointer == NULL){
//            main_stack_pointer = (uint32_t *)context;
//        }
//
//        current_running_thread = (ThreadControlBlock *)Dequeue(&lsReadyThreads);
//        int allAreBlockedCounter = lsReadyThreads.count;
//        while(current_running_thread->state == BLOCKED){
//        	if (allAreBlockedCounter < 0) {
//        		printf("we have some problems");
//        	}
//        	Enqueue(&lsReadyThreads, (void*) current_running_thread);
//        	current_running_thread = (ThreadControlBlock *)Dequeue(&lsReadyThreads);
//        	allAreBlockedCounter--;
//        }
//        current_running_thread->state = RUNNING;
//
//        context = (void *)(current_running_thread->sp);
//    }
//    else if (current_running_thread==NULL && main_stack_pointer!=NULL){
//        context = (void *)main_stack_pointer;
//    }
//
//    return context;
//}
//
//unsigned int mythread_isQempty(){
//    return (lsReadyThreads.count == 0) && (current_running_thread == NULL);
//}
//
//void mythread_cleanup(){
//	printf("Completing thread %c\n", current_running_thread->threadID);
//    DISABLE_INTERRUPTS();
//    mythread_terminate(current_running_thread);
//    free(current_running_thread->stack);
//    free(current_running_thread);
//    current_running_thread = NULL;
//    ENABLE_INTERRUPTS();
//    while(1);
//}
