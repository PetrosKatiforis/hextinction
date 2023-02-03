#include "game.h"
#include "utils.h"

#include <stdio.h>
#include <time.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

void initialize_sdl_components()
{
    assert_panic(SDL_Init(SDL_INIT_EVERYTHING) < 0, "Something went wrong, couldn't initialize SDL2");
    assert_panic(IMG_Init(IMG_INIT_PNG) < 0, "Failed to initialize SDL_image for whatever reason");
    assert_panic(TTF_Init() < 0, "Failed to initialize TTF_font");

    // Opening the default audio device with standard configuration
    assert_panic(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0, "Failed to open audio device!");
}

void create_game(game_t* game, const char* title, unsigned int width, unsigned int height, unsigned int frames_per_second)
{
    initialize_sdl_components();
    puts("SDL has been initialized. Now trying to create the window and the renderer.");

    // Creating the window at the center of the screen by default
    game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    game->width = width;
    game->height = height;
    game->frame_delay = 1000 / frames_per_second;
}

void handle_event(game_t* game, SDL_Event* event)
{
    if (event->type == SDL_MOUSEMOTION && game->active_dropdown)
    {
        on_dropdown_mouse_move(game->active_dropdown, event->motion.x, event->motion.y);
    }
}

void activate_dropdown_at(game_t* game, dropdown_t* dropdown, int x, int y)
{
    game->active_dropdown = dropdown;
    set_dropdown_position(game->active_dropdown, x, y);
}

// Finishes off the SDL rendering process and draws the active dropdown as well
void finish_game_rendering(game_t* game)
{
    if (game->active_dropdown)
        render_dropdown(game->active_dropdown, game->renderer);

    SDL_RenderPresent(game->renderer);
    SDL_Delay(game->frame_delay);
}

int get_dropdown_choice(game_t* game, dropdown_t* dropdown)
{
    if (dropdown != game->active_dropdown) return -1;

    // Closing the dropdown after the action
    unsigned int selected = game->active_dropdown->selected_index;
    game->active_dropdown = NULL;
    
    return selected;
}

void free_game(game_t* game)
{
    SDL_DestroyWindow(game->window);
    SDL_DestroyRenderer(game->renderer);

    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

