#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <stdbool.h>
#include "soldiers.h"
#include "engine/game.h"
#include "engine/sprite.h"
#include "engine/audio.h"
#include "libs/open-simplex/noise.h"

#define TILE_WIDTH 34
#define TILE_HEIGHT 32
#define CITY_NAME_LEN 30

// Constants that should be configured by the programmer
#define FRAMES_PER_SECOND 20
#define TILEMAP_WIDTH 20
#define TILEMAP_HEIGHT 34 
#define TOTAL_CITIES 5
#define MOVES_PER_TURN 5
#define TOTAL_PLAYERS 2
#define UNITS_PER_TURN 10
#define MAX_UNITS 100

#define FOREST_THRESHOLD 0.3
#define LAND_THRESHOLD -0.3

// Making it into a constant because it might change in the future
// For instance, I might want to make the movement faster
#define NEIGHBOURING_TILES 6

static SDL_Color highlight_color = {255, 255, 255, 80};

static char city_names[][CITY_NAME_LEN] = {
    "Athens", "Thessaloniki", "Ankara", "London", "Paris", "Islamabad",
    "New Delhi", "New York", "Moscow", "Rome", "Napoli", "Berlin", "Stockholm",
    "Brazilia", "Buenos Aires", "Istanbul", "Tripoli", "Madrid", "Patra", "Bordeaux",
    "Bursa", "Tirana", "Sofia", "Beijin", "Tokyo", "Saint Petersburg", "Jerusalem",
    "Liverpool", "Dublin", "Warsaw", "Manchester", "Essen", "Los Angeles", "Cairo",
};

static SDL_Color player_colors[TOTAL_PLAYERS] = {
    {0, 0, 255, 100}, // Physical player color
    {0, 255, 0, 100},
    //{255, 0, 0, 100},
    //{255, 0, 255, 100},
};

static int capital_positions[TOTAL_PLAYERS][2] = {
    {0, 3},
    {TILEMAP_WIDTH - 1, TILEMAP_HEIGHT - 4},
    //{TILEMAP_WIDTH - 1, 2},
    //{0, TILEMAP_HEIGHT - 3},
};

// This is all the game state, accumulated in one big struct
typedef struct
{
    game_t game;
    TTF_Font* font;
    struct osn_context* noise_context;

    SDL_Texture* tilemap_texture;
    SDL_Texture* border_texture;
    SDL_Texture* soldiers_texture;
    tile_t tilemap[TILEMAP_HEIGHT][TILEMAP_WIDTH];

    soldiers_t* selected_soldiers;
    tile_t* highlighted_tiles[NEIGHBOURING_TILES];

    // player capitals + cities
    label_t city_labels[TOTAL_CITIES + TOTAL_PLAYERS];

    animated_sprite_t explosion;
    audio_t explosion_sfx;

    // properties needed for turn-based gameplay
    bool is_player_dead[TOTAL_PLAYERS];
    int current_player_id;
    int remaining_moves;

    sprite_t turn_arrow;
} context_t;

// This is its globally accessible instance
extern context_t ctx;

// Some utility functions
SDL_Texture* load_texture(const char* file);

#endif
