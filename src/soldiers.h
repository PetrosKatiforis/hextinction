#ifndef _SOLDIERS_H
#define _SOLDIERS_H

#include "engine/sprite.h"

// Forward decleration
struct tile_t;

typedef struct soldiers_t
{
    unsigned int units;
    label_t units_label;

    // Used to determine soldiers texture
    SDL_Rect source_rect;

    struct tile_t* current_tile;
} soldiers_t;

soldiers_t* create_soldiers(int tile_x, int tile_y);
void set_soldier_units(soldiers_t* soldiers, unsigned int units);

// Just places the soldiers on an empty tile (used by initialization)
void place_soldiers(soldiers_t* soldiers, struct tile_t* tile);

// Moves soldiers and calculates remaining troops from battle if there's any
void move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y);

// Includes validation too
void select_soldiers(soldiers_t* soldiers, int tile_x, int tile_y);

void clear_selected_soldiers();
void destroy_soldiers(soldiers_t* soldiers);

#endif
