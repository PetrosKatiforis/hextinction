#include <stdbool.h>
#include "tile.h"
#include "hex_utils.h"
#include "context.h"
#include "engine/sprite.h"

// Texture index refers to x position in tilemap texture
void create_tile(int tile_x, int tile_y, tile_kind_e kind)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    // Picking the tile from the tilemap texture and saving its position
    set_tile_kind(tile_x, tile_y, kind);
    assign_tile_position(tile_x, tile_y);

    tile->owner_id = -1;
}

// Will update texture as well
void set_tile_kind(int tile_x, int tile_y, tile_kind_e kind)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    tile->kind = kind;

    if (kind != TILE_WATER)
        tile->source_rect = (SDL_Rect) {kind * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
}

void create_city(int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    set_tile_kind(tile_x, tile_y, TILE_CITY);
            
    // Creates the text label, loops through the pool instead of a dynamic array because it's small
    for (int i = 0; i < TOTAL_LABELS; i++)
    {
        label_t* label = &ctx.city_labels[i];

        // Ignore it if it's already used
        if (label->sprite.texture != NULL) continue;

        create_label(label, ctx.font, 0);
        set_label_content(label, ctx.game.renderer, city_names[rand() % (sizeof(city_names) / CITY_NAME_LEN)]);

        // Labels at the top right should be left-aligned
        int x = tile->dest_rect.x + (tile_x > TILEMAP_WIDTH - 3 ? -label->sprite.transform.rect.w : 40);

        set_transform_position(&label->sprite.transform, x, tile->dest_rect.y + 4);

        tile->label_index = i;

        break;
    }

    ctx.total_cities++;
}

bool is_water(int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    // Creating a special function for these because I might want more water tiles later on
    return tile->kind == TILE_WATER || tile->kind == TILE_FISH;
}
