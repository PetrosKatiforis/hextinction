#include "hex_utils.h"
#include "engine/utils.h"

bool is_neighbouring_tile(int source_x, int source_y, int dest_x, int dest_y)
{
    // Check check every neighbours, they are only nine so it's fast
    FOREACH_NEIGHBOUR
    {
        if (dest_x == GET_NEIGHBOUR_X_FROM(source_x, source_y) && dest_y == GET_NEIGHBOUR_Y_FROM(source_x, source_y))
            return true;
    }

    return false;
}

void window_to_tile_position(int* tile_x, int* tile_y, int x, int y)
{
    // If you're wondering where these came from, I just solved the tile->dest_rect.x, y formula for its index (see create_tile function)
    *tile_y = y / (TILE_HEIGHT / 2);

    int offset = *tile_y % 2 == 1 ? 25 : 0;
    *tile_x = (x - offset) / (TILE_WIDTH + 16);

    // Finding coords relative to box for a more precise answer
    int relative_x = x - (offset + *tile_x * (TILE_WIDTH + 16));

    if (relative_x > TILE_WIDTH)
    {
        *tile_y = MAX(*tile_y - 1, 0);

        // Exclusive to odd rows (discovered from play testing)
        if (offset > 0)
            *tile_x = MIN(TILEMAP_WIDTH, *tile_x + 1);
    }
}

bool is_valid_tile(int tile_x, int tile_y)
{
    return tile_x >= 0 && tile_x < TILEMAP_WIDTH && tile_y >= 0 && tile_y < TILEMAP_HEIGHT;
}

