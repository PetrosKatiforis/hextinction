#include "game.h"
#include "utils.h"

#include <stdio.h>
#include <time.h>

#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

void initialize_sdl_components()
{
    assert_panic(SDL_Init(SDL_INIT_EVERYTHING) < 0, "Something went wrong, couldn't initialize SDL2");
    assert_panic(IMG_Init(IMG_INIT_PNG) < 0, "Failed to initialize SDL_image for whatever reason");
    assert_panic(TTF_Init() < 0, "Failed to initialize TTF_font");

    // Opening the default audio device with standard configuration
    assert_panic(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0, "Failed to open audio device!");
}

void create_game(game_t* game, const char* title, unsigned int width, unsigned int height)
{
    initialize_sdl_components();
    puts("SDL has been initialized. Now trying to create the window and the renderer.");

    // Creating some randomness
    srand(time(NULL));

    // Creating the window at the center of the screen by default
    game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    game->width = width;
    game->height = height;
}

void free_game(game_t* game)
{
    SDL_DestroyWindow(game->window);
    SDL_DestroyRenderer(game->renderer);

    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

