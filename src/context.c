#include "context.h"

// Initializing the global variable
context_t ctx;

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// These functions are some utilities that can only be defined with the context in mind
// They are usually brief versions of other functionality

SDL_Texture* load_texture(const char* file)
{
    return IMG_LoadTexture(ctx.game.renderer, file);
}
