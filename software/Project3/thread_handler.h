#include <stdint.h>

/* possible thread states */
enum tstate {NEW, READY, RUNNING, BLOCKED, TERMINATED, DUMMY, NUM_TSTATES};

//typedef struct {
//	uint32_t totalTicks;
//	uint32_t startTicks;
//	uint32_t lastStartTicks;
//} ThreadStats_t;


/* thread control block */
typedef struct ThreadControlBlock {
    char threadID;
    enum tstate  state;
    struct ThreadControlBlock*  ParentThread;
    struct LinkedList *lsChildThreads;
    uint32_t *stack;
    uint32_t *sp;
    //    ThreadStats_t* threadStats;
} ThreadControlBlock;

/* declaration */
ThreadControlBlock *mythread_CreateEmptyThread(char threadID);

ThreadControlBlock *mythread_create(char threadID, uint32_t stack_size, void (*mythread)(char threadID));

ThreadControlBlock* GetCurrentRunningThread();

void mythread_start(ThreadControlBlock *thread_pointer);

void mythread_join(ThreadControlBlock *parent, ThreadControlBlock *thread_pointer);

void mythread_block(ThreadControlBlock *thread_pointer);

void mythread_terminate(ThreadControlBlock *thread_pointer);

void *mythread_schedule(void *context);

unsigned int mythread_isQempty();

void mythread_cleanup();

