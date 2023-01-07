#include "custom_math.h"
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

bool is_point_inside(SDL_Rect* rect, int x, int y)
{
    // Just a basic algorithm for rectangles
    return x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h;
}

int get_distance(int x, int y, int other_x, int other_y)
{
    // Basically getting the magnitude of their connecting vector
    return sqrt(pow(other_x - x, 2) + pow(other_y - y, 2));
}


