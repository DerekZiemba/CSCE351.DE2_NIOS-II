#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

//#define NDEBUG //Comment in to disable assert messages

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* The two macros are extremely useful by turnning on/off interrupts when atomicity is required */
#define DISABLE_INTERRUPTS() {  \
    asm("wrctl status, zero");  \
}

#define ENABLE_INTERRUPTS() {   \
    asm("movi et, 1");          \
    asm("wrctl status, et");    \
}

#define CHECKMALLOC(x, msg) {					\
	if(x == NULL) {								\
        printf("\nUnable to allocate space!\n");	\
        printf(msg);							\
        exit(1);								\
	}											\
}

#ifdef NDEBUG
#define SHOW_DEBUG_MESSAGES 0
#else
#define SHOW_DEBUG_MESSAGES 1
#endif


#define CheckForError(bTestPassed, msg) if (!bTestPassed && SHOW_DEBUG_MESSAGES) { printf(msg); };

