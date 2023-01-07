#ifndef _SOLDIERS_H
#define _SOLDIERS_H

#include "engine/interface.h"

// Forward decleration
struct tile_t;

typedef struct soldiers_t
{
    unsigned int units;
    unsigned int remaining_moves;

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
// Returns true if the attacker won territory
bool move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y);

// Includes validation too
void select_soldiers(soldiers_t* soldiers, int tile_x, int tile_y);

// Sets the tile owner to sender_id and adjusts total territories
void capture_tile(struct tile_t* tile, int sender_id);

void clear_selected_soldiers();
void destroy_soldiers(soldiers_t* soldiers);

#endif
