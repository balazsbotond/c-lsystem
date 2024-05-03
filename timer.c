#include "timer.h"

Timer timer_start() {
    Timer timer;
    timer.start = clock();
    return timer;
}

double timer_stop(Timer* timer) {
    timer->end = clock();
    return (double)(timer->end - timer->start) / CLOCKS_PER_SEC;
}