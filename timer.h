#ifndef TIMER_H
#define TIMER_H

#include <time.h>

typedef struct {
    clock_t start;
    clock_t end;
} Timer;

Timer timer_start();
double timer_stop(Timer* timer);

#endif // TIMER_H