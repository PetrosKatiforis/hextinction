#include "hex_utils.h"
#include "engine/utils.h"

bool is_neighbouring_tile(int source_x, int source_y, int dest_x, int dest_y)
{
    // Check check every neighbours, they are only nine so it's fast
    FOREACH_OFFSET(source_y, TOTAL_HIGHLIGHTED, i) 
    {
        int x = source_x + highlighted_offsets[i][0];
        int y = source_y + highlighted_offsets[i][1];

        if (dest_x == x && dest_y == y)
            return true;
    }

    return false;
}

bool window_to_tile_position(int* tile_x, int* tile_y, int x, int y)
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

    return is_valid_tile(*tile_x, *tile_y);
}

bool is_valid_tile(int tile_x, int tile_y)
{
    return tile_x >= 0 && tile_x < TILEMAP_WIDTH && tile_y >= 0 && tile_y < TILEMAP_HEIGHT;
}

double get_noise_value(int tile_x, int tile_y)
{
    return open_simplex_noise2(ctx.noise_context, tile_x / 8.0, tile_y / 8.0);
}

// Determines position based on grid coordinates
void assign_tile_position(int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    // Some magic numbers here, I didn't want to find a precise formula for it so I'm just doing it manually with trial and error
    int offset = (tile_y % 2 == 1) ? 25 : 0;

    tile->dest_rect = (SDL_Rect) {offset + tile_x * (TILE_WIDTH + 16), tile_y * TILE_HEIGHT / 2, TILE_WIDTH, TILE_HEIGHT};
}

void process_hex_dropdown(dropdown_t* dropdown, dropdown_handler handler)
{
    int choice = get_dropdown_choice(&ctx.game, dropdown);

    if (choice < 0)
        return;

    int tile_x, tile_y;
                
    // Getting the coordinates of the tile that was right clicked when the dropdown showed up
    if (!window_to_tile_position(&tile_x, &tile_y, dropdown->background.x, dropdown->background.y))
        return;

    handler(tile_x, tile_y, choice);
}
