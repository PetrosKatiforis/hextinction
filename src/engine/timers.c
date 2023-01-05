#include "timers.h"
#include <SDL.h>

// Just update's the timer's starting_time nothing fancy
void start_timer(simple_timer_t* timer)
{
    timer->starting_time = SDL_GetTicks();
}

uint32_t restart_timer(simple_timer_t* timer)
{
    uint32_t now = SDL_GetTicks();
    uint32_t elapsed = now - timer->starting_time;

    // Restart the timer by making it count time since now
    timer->starting_time = now;

    return elapsed;
}


void initialize_interval(interval_t* timer, uint32_t interval)
{
    // This sounds weird but I couldn't think of a better property name
    timer->interval = interval;

    timer->starting_time = SDL_GetTicks();
}

bool has_reached_interval(interval_t* interval)
{
    if (SDL_GetTicks() - interval->starting_time > interval->interval)
    {
        // If the interval has been reached, restart the timer and return true
        interval->starting_time = SDL_GetTicks();

        return true;
    }

    return false;
}


