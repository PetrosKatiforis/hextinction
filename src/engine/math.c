#include "math.h"
#include <math.h>

// This will be used for normalization
float get_vector_magnitude(vec2* source)
{
    return sqrt(source->x * source->x + source->y * source->y);
}

void normalize_vector(vec2* source, vec2* target)
{
    float magnitude = get_vector_magnitude(source);
    
    target->x = source->x / magnitude;
    target->y = source->y / magnitude;
}

// Operators between vectors
// C does not support any operation overloading
void add_vectors(vec2* first, vec2* second, vec2* result)
{
    result->x = first->x + second->x;
    result->y = first->y + second->y;
}

void subtract_vectors(vec2* first, vec2* second, vec2* result)
{
    result->x = first->x - second->x;
    result->y = first->y - second->y;
}

void multiply_vectors(vec2* first, vec2* second, vec2* result)
{
    result->x = first->x * second->x;
    result->y = first->y * second->y;
}

void divide_vectors(vec2* first, vec2* second, vec2* result)
{
    result->x = first->x / second->x;
    result->y = first->y / second->y;
}


