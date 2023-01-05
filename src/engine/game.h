#ifndef _WINDOW_H
#define _WINDOW_H

#include <SDL.h>

typedef struct
{
    // SDL-specific properties
    SDL_Window* window;
    SDL_Renderer* renderer;

    unsigned int width, height;
} game_t;

// Creates the window and initializes the SDL components
void create_game(game_t* game, const char* title, unsigned int width, unsigned int height);

void free_game(game_t* game);

#endif
