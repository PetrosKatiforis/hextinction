#ifndef _MACROS_H
#define _MACROS_H

#include <stdbool.h>
#include <SDL.h>

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

// I initialy had this defined as a macro, but I don't think there is any reason for it to be that way
void assert_panic(bool condition, const char* message);

// Functions for randomness
bool chance_one_in(int max);

#endif
