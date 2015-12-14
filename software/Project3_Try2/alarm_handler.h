
#ifndef ALARM_HANDLER_H_
#define ALARM_HANDLER_H_
#include <stdint.h>


uint8_t is_alarmflag_set();
void reset_alarmflag();
void set_alarmflag();

uint8_t start_alarm(uint32_t millis, void (*callback) (void* context));
void forceInterrupt();

#endif /* ALARM_HANDLER_H_ */
