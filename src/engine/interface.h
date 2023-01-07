#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <stdbool.h>
#include "sprite.h"

// Labels will be defined as sprites as well
typedef struct
{
    sprite_t sprite;

    TTF_Font* font;
    SDL_Color color;

    char* content;
    unsigned int wrap_length;
} label_t;

// Leave wrap_length to 0 for default rendering
void create_label(label_t* label, TTF_Font* font, unsigned int wrap_length);

void set_label_content(label_t* label, SDL_Renderer* renderer, const char* content);
void set_label_color(label_t* label, SDL_Renderer* renderer, SDL_Color color);
void destroy_label(label_t* label);

// Dropdown menus
static SDL_Color dropdown_background = {20, 20, 20, 255};
static SDL_Color dropdown_highlight = {70, 70, 70, 255};

#define DROPDOWN_PADDING 6

typedef struct
{
    label_t* items;
    unsigned int length, width, height;
    int selected_index;

    SDL_Rect background;
    SDL_Rect highlight;
} dropdown_t;

// ... is a va_list containing all label text strings
void create_dropdown(dropdown_t* dropdown, SDL_Renderer* renderer, TTF_Font* font, unsigned int total_items, ...);

void set_dropdown_position(dropdown_t* dropdown, int x, int y);
void on_dropdown_mouse_move(dropdown_t* dropdown, int x, int y);

void render_dropdown(dropdown_t* dropdown, SDL_Renderer* renderer);

#endif
