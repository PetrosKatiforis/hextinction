#ifndef _WINDOW_H
#define _WINDOW_H

#include <SDL2/SDL.h>
#include "interface.h"

/*
 * A struct containing generic properties for any kind of game
 * Created to save time from copying myself over and over again
 */
typedef struct
{
    // SDL-specific properties
    SDL_Window* window;
    SDL_Renderer* renderer;

    unsigned int width, height;

    // How much the game should wait between each frame
    unsigned int frame_delay;

    dropdown_t* active_dropdown;
} game_t;

// Creates the window and initializes the SDL components
void create_game(game_t* game, const char* title, unsigned int width, unsigned int height, unsigned int frames_per_second);

// Helper methods for making dropdown menu handling easier, but of course not limited to that
// This behaviour is generic and will be used throughout all future games too, it's a bit faster performance wise and typing wise
void handle_event(game_t* game, SDL_Event* event);
void activate_dropdown_at(game_t* game, dropdown_t* dropdown, int x, int y);
int get_dropdown_choice(game_t* game, dropdown_t* dropdown);

void finish_game_rendering(game_t* game);
void free_game(game_t* game);

#endif
