#include "soldiers.h"
#include "context.h"
#include "hex_utils.h"

// Assigns the soldiers' current_tile and moves the units label accordingly
void place_soldiers(soldiers_t* soldiers, tile_t* tile)
{
    tile->soldiers = soldiers;
    soldiers->current_tile = tile;

    set_transform_position(&soldiers->units_label.sprite.transform, tile->dest_rect.x + TILE_WIDTH - 8, tile->dest_rect.y + TILE_HEIGHT - 8);
}

// Unrelated function that creates an explosion
void activate_explosion(int pos_x, int pos_y)
{
    set_transform_position(&ctx.explosion.sprite.transform, pos_x - 16, pos_y - 16);
    play_animated_sprite(&ctx.explosion);

    play_audio(ctx.explosion_sfx);
}

void move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    int sender_id = soldiers->current_tile->owner_id;

    soldiers->current_tile->soldiers = NULL;

    place_soldiers(soldiers, tile);

    // TODO: Make this actually check for enemies and capture neighbouring tiles
    FOREACH_NEIGHBOUR
    {
        int x = GET_NEIGHBOUR_X_FROM(tile_x, tile_y);
        int y = GET_NEIGHBOUR_Y_FROM(tile_x, tile_y);

        ctx.tilemap[y][x].owner_id = sender_id;
    }

    tile->owner_id = sender_id;
    activate_explosion(tile->dest_rect.x, tile->dest_rect.y);
}

soldiers_t* create_soldiers(int tile_x, int tile_y)
{
    soldiers_t* soldiers = malloc(sizeof(soldiers_t));

    // Default soldier units
    soldiers->units = 10;

    create_label(&soldiers->units_label, ctx.game.renderer, ctx.font, "10");
    place_soldiers(soldiers, &ctx.tilemap[tile_y][tile_x]);

    return soldiers;
}

void set_soldier_units(soldiers_t* soldiers, unsigned int units) 
{
    // Converting unsigned int to a c string
    char new_content[10];
    sprintf(new_content, "%u", units);

    soldiers->units = units;
    set_label_content(&soldiers->units_label, ctx.game.renderer, new_content);
}

void select_soldiers(soldiers_t* soldiers)
{
    // Check if the player owns the soldiers and then update the UI 
    if (soldiers->current_tile->owner_id != 0) return;

    ctx.selected_soldiers = soldiers;
    set_label_color(&soldiers->units_label, ctx.game.renderer, selected_soldiers_color);
}

void clear_selected_soldiers()
{
    set_label_color(&ctx.selected_soldiers->units_label, ctx.game.renderer, (SDL_Color) {255, 255, 255, 255});
    ctx.selected_soldiers = NULL;
}


