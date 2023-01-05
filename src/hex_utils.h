#ifndef _HEX_UTILS_H
#define _HEX_UTILS_H

#include "context.h"

/*
 * Although I could place all these in tile.h I created a unique file for them
 * These are methods that have to do with the mathematically part of the hex map
 * Things like initialization and tile appearance are defined in tile.h
 */

// Instructions for finding neighbours relative to tile (even or odd)
// Definiing the manually because I won't need more than the 18 surrounding tiles
static int neighbours_offsets[TOTAL_NEIGHBOURS * 2][2] = {
    // This is for odd tiles
    {0, 2}, {0, -2}, {0, 1}, {0, -1}, {-1, -1}, {-1, 1},

    {0, 2}, {0, -2}, {0, 1}, {0, -1}, {1, -1}, {1, 1},
};

static int highlighted_offsets[TOTAL_HIGHLIGHTED * 2][2] = {
    {0, 2}, {0, -2}, {0, 1}, {0, -1}, {-1, -1}, {-1, 1},
    {0, 4}, {-1, -3}, {1, -2}, {1, 0}, {1, 2}, {-1, 3},
    {0, -4}, {-1, -2}, {-1, 0}, {-1, 2}, {0, 3}, {0, -3},

    // For even tiles
    {0, 2}, {0, -2}, {0, 1}, {0, -1}, {1, -1}, {1, 1},
    {0, 4}, {0, -4}, {0, 3}, {0, -3}, {1, -2}, {-1, -2},
    {1, 0}, {-1, 0}, {1, 2}, {-1, 2}, {1, 3}, {1, -3},
 };

// I know, this is really hacky but I'm ok with it for now
#define FOREACH_OFFSET(tile_y, total_tiles, i) \
    for (int i = ((tile_y % 2 == 0) ? 0 : total_tiles); i < ((tile_y % 2) + 1) * total_tiles; i++) \

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
