#ifndef _TILE_H
#define _TILE_H

#include <SDL.h>

// Forward decleration
struct soldiers_t;

// These must be in the same order as in the texture
typedef enum
{
    TILE_GRASS,
    TILE_FOREST,
    TILE_COAST,
    TILE_CITY,
    TILE_PORT,
    TILE_FARM,
    TILE_FISH,
    TILE_WATER,
} tile_kind_e;

// A tile represents a hex inside the game grid, it can be water or occupied land
typedef struct tile_t
{
    SDL_Rect source_rect;
    SDL_Rect dest_rect;

    tile_kind_e kind;
    bool is_capital;

    // Index of city label in ctx.city_labels
    int label_index;
    
    // Again, left to -1 if unclaimed
    int owner_id;
    struct soldiers_t* soldiers;
} tile_t;

void create_tile(int tile_x, int tile_y, tile_kind_e kind);

void set_tile_kind(int tile_x, int tile_y, tile_kind_e kind);
void create_city(int tile_x, int tile_y);

bool is_water(int tile_x, int tile_y);

#endif
