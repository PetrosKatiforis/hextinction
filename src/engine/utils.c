#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

bool is_point_inside(SDL_Rect* rect, int x, int y)
{
    // Just a basic algorithm for rectangles
    return x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h;
}

// Just a simple functio to exit if a condition is met and display a message while doing it 
void assert_panic(bool condition, const char* message)
{
    if (condition)
    {
        puts(message);
        exit(EXIT_FAILURE);
    }
}

bool chance_one_in(int max)
{
    // 0 is equally likely
    return rand() % max == 0;
}
