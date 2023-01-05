#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

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
