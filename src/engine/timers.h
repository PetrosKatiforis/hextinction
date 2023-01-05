#ifndef _TIMERS_H
#define _TIMERS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t starting_time;
} simple_timer_t;

void start_timer(simple_timer_t* timer);

// This function restarts the timer and returns the elapsed time in milliseconds
uint32_t restart_timer(simple_timer_t* timer);


typedef struct
{
    uint32_t starting_time;
    
    // How long should it wait between each "fire"
    uint32_t interval;
} interval_t;

void initialize_interval(interval_t* timer, uint32_t interval);
bool has_reached_interval(interval_t* interval);

#endif
