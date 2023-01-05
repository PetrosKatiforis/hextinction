#ifndef _MY_MATH_H
#define _MY_MATH_H

// A vector2f structure with some special methods that will be used throught most games
typedef struct
{
    float x, y;
} vec2;

float get_vector_magnitude(vec2* source);
void normalize_vector(vec2* source, vec2* target);

void add_vectors(vec2* first, vec2* second, vec2* result);
void subtract_vectors(vec2* first, vec2* second, vec2* result);
void multiply_vectors(vec2* first, vec2* second, vec2* result);
void divide_vectors(vec2* first, vec2* second, vec2* result);

#endif
