#ifndef _SOLDIERS_H
#define _SOLDIERS_H

#include "engine/sprite.h"

// Forward decleration
struct soldiers_t;

// A tile represents a hex inside the game grid, it can be water or occupied land
typedef struct
{
    SDL_Rect source_rect;
    SDL_Rect dest_rect;

    bool is_walkable;

    // Index of city_names, left -1 if the tile is not a city
    int city_index;
    
    // Again, left to -1 if unclaimed
    int owner_id;
    struct soldiers_t* soldiers;
} tile_t;

typedef struct soldiers_t
{
    unsigned int units;
    label_t units_label;

    tile_t* current_tile;
} soldiers_t;

soldiers_t* create_soldiers(int tile_x, int tile_y);
void set_soldier_units(soldiers_t* soldiers, unsigned int units);

// Just places the soldiers on an empty tile (used by initialization)
void place_soldiers(soldiers_t* soldiers, tile_t* tile);

// Moves soldiers and calculates remaining troops from battle if there's any
void move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y);

// Includes validation too
void select_soldiers(soldiers_t* soldiers);

void clear_selected_soldiers();

#endif
