#ifndef _HEX_UTILS_H
#define _HEX_UTILS_H

#include "context.h"

/*
 * Although I could place all these in tile.h I created a unique file for them
 * These are methods that have to do with the mathematically part of the hex map
 * Things like initialization and tile appearance are defined in tile.h
 */

// Instructions for finding neighbours relative to tile
static int offsets_odd[NEIGHBOURING_TILES][2] = {
    {0, 2}, {0, -2}, {0, 1}, {0, -1},
    {1, -1}, {1, 1},
};

static int offsets_even[NEIGHBOURING_TILES][2] = {
    {0, 2}, {0, -2}, {0, 1}, {0, -1},
    {-1, -1}, {-1, 1}
};

// I use macros at my own risk
#define FOREACH_NEIGHBOUR for (int offset_i = 0; offset_i < NEIGHBOURING_TILES; offset_i++)

#define GET_NEIGHBOUR_Y_FROM(x, y) y + (y % 2 == 0 ? offsets_even : offsets_odd)[offset_i][1]
#define GET_NEIGHBOUR_X_FROM(x, y) x + (y % 2 == 0 ? offsets_even : offsets_odd)[offset_i][0]
#define GET_NEIGHBOUR(x, y) &ctx.tilemap[GET_NEIGHBOUR_Y_FROM(y)][GET_NEIGHBOUR_X_FROM(x)] 

#define MAP_FOREACH(x, y) \
    for (int y = 0; y < TILEMAP_HEIGHT; y++) \
        for (int x = 0; x < TILEMAP_WIDTH; x++) \

bool is_neighbouring_tile(int source_x, int source_y, int dest_x, int dest_y);
void window_to_tile_position(int* tile_x, int* tile_y, int x, int y);

// Checks if the tile with the specified coordinates is inside the map range
bool is_valid_tile(int tile_x, int tile_y);

double get_noise_value(int tile_x, int tile_y);
void assign_tile_position(int tile_x, int tile_y);

#endif
