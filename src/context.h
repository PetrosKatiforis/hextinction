#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <stdbool.h>
#include "soldiers.h"
#include "tile.h"
#include "engine/game.h"
#include "engine/sprite.h"
#include "engine/audio.h"
#include "libs/open-simplex/noise.h"
#include "hex_utils.h"

#define TILE_WIDTH 34
#define TILE_HEIGHT 32
#define CITY_NAME_LEN 30

// Constants that should be configured by the programmer
#define FRAMES_PER_SECOND 20
#define TILEMAP_WIDTH 20
#define TILEMAP_HEIGHT 34 
#define MOVES_PER_TURN 50
#define TOTAL_PLAYERS 2
#define UNITS_PER_TURN 10
#define MAX_UNITS 100
#define PANEL_WIDTH 220
#define PANEL_PADDING 20

#define FOREST_THRESHOLD 0.3
#define LAND_THRESHOLD -0.3

// It's ok if it's a bit more than the real value
#define TOTAL_LABELS 3 * TILEMAP_HEIGHT

static SDL_Color highlight_color = {255, 255, 255, 80};
static SDL_Color panel_color = {30, 30, 30, 255};

static char city_names[][CITY_NAME_LEN] = {
    "Athens", "Thessaloniki", "Ankara", "London", "Paris", "Islamabad",
    "New Delhi", "New York", "Moscow", "Rome", "Napoli", "Berlin", "Stockholm",
    "Brazilia", "Buenos Aires", "Istanbul", "Tripoli", "Madrid", "Patra", "Bordeaux",
    "Bursa", "Tirana", "Sofia", "Beijin", "Tokyo", "Saint Petersburg", "Jerusalem",
    "Liverpool", "Dublin", "Warsaw", "Manchester", "Essen", "Los Angeles", "Cairo",
};

static SDL_Color player_colors[TOTAL_PLAYERS] = {
    {0, 0, 255, 100},
    {0, 255, 0, 100},
    //{255, 0, 0, 100},
    //{255, 0, 255, 100},
};

static char player_names[TOTAL_PLAYERS][30] = {
    "Blunited Kingdom", "Greenborg", //"Redulke", "Purplevania",
};

static char player_descriptions[TOTAL_PLAYERS][200] = {
    "A powerful kingdom located at the top west. They are ruled by a young monarch whose expertise are naval battles",
    "A rather hostile barbaric society of gigantic orcs. Their army mostly consists of hostage humans",
    //"A fearless empire located on the far east. They passionately want to make themselves stand out from the Arabs",
    //"A mysterious witch town that started explanding with the intension of imporving their living conditions."
};

static int capital_positions[TOTAL_PLAYERS][2] = {
    {0, 3},
    {TILEMAP_WIDTH - 1, TILEMAP_HEIGHT - 4},
    //{TILEMAP_WIDTH - 1, 2},
    //{0, TILEMAP_HEIGHT - 3},
};

// Using the formula of hex_utils.c, these are in pixels
#define TOTAL_TILEMAP_HEIGHT TILEMAP_HEIGHT * 16 + 16
#define TOTAL_TILEMAP_WIDTH TILEMAP_WIDTH * (TILE_WIDTH + 16) + TILE_WIDTH

typedef struct
{
    bool is_dead;

    unsigned int total_territories;
    unsigned int coins;
} player_t;

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
    audio_t soldiers_sfx;
    tile_t* highlighted_tiles[TOTAL_HIGHLIGHTED];

    // player capitals + cities
    label_t city_labels[TOTAL_LABELS];
    unsigned int total_cities;

    animated_sprite_t explosion;
    audio_t explosion_sfx;

    // properties needed for turn-based gameplay
    player_t players[TOTAL_PLAYERS];
    int current_player_id;
    int remaining_moves;

    sprite_t turn_arrow;

    // user interface
    label_t player_name;
    label_t player_description;
    label_t player_coins;
    label_t player_territories;

    sprite_t player_profile;
    SDL_Rect panel_rect;
} context_t;

// This is its globally accessible instance
extern context_t ctx;

// Some utility functions
SDL_Texture* load_texture(const char* file);

#endif
