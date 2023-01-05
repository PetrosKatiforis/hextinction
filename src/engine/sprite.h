#ifndef _SPRITE_H
#define _SPRITE_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "math.h"
#include "timers.h"

// Transforms are used for things that can be rendered
// They include a float based position, an SDL rectangle for rendering and an origin point
typedef struct
{
    vec2 position;
    SDL_Rect rect;

    // This is used for offsetting the rectangle
    // It's usually just the center of it
    int origin[2];
} transform_t;

void set_transform_position(transform_t* transform, float x, float y);
void set_transform_x(transform_t* transform, float x);
void set_transform_y(transform_t* transform, float y);

void center_origin(transform_t* transform);
void set_transform_scale(transform_t* transform, float scale);


// Sprites are 2D entities that can be rendered using a simple texture
typedef struct
{
    SDL_Texture* texture;
    SDL_Rect source_rect;

    transform_t transform;
} sprite_t;

void create_sprite(sprite_t* sprite, SDL_Texture* texture);
void render_sprite(sprite_t* sprite, SDL_Renderer* renderer);


// Animated sprite
typedef struct
{
    sprite_t sprite;
    simple_timer_t anim_timer;

    unsigned int frame_width, total_frames, frame_duration_ms;
    bool is_active;
} animated_sprite_t;

void create_animated_sprite(animated_sprite_t* anim_sprite, SDL_Texture* texture, unsigned int total_frames, unsigned int frame_duration_ms);

// Starts the animated sprite's animation and timer
void play_animated_sprite(animated_sprite_t* anim_sprite);

void render_animated_sprite(animated_sprite_t* anim_sprite, SDL_Renderer* renderer);


// TODO: Make this a bit more dynamic
#define MAX_LABEL_LEN 70

// Labels will be defined as sprites as well
typedef struct
{
    sprite_t sprite;

    TTF_Font* font;
    SDL_Color color;
    char content[MAX_LABEL_LEN];
} label_t;

void create_label(label_t* label, SDL_Renderer* renderer, TTF_Font* font, const char* content);
void set_label_content(label_t* label, SDL_Renderer* renderer, const char* content);
void set_label_color(label_t* label, SDL_Renderer* renderer, SDL_Color color);
void destroy_label(label_t* label);

#endif
