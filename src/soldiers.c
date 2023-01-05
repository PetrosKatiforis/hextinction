#include "soldiers.h"
#include "engine/utils.h"
#include "context.h"
#include "hex_utils.h"

// Assigns the soldiers' current_tile and moves the units label accordingly
void place_soldiers(soldiers_t* soldiers, tile_t* tile)
{
    tile->soldiers = soldiers;
    soldiers->current_tile = tile;

    set_transform_position(&soldiers->units_label.sprite.transform, tile->dest_rect.x + TILE_WIDTH - 8, tile->dest_rect.y + TILE_HEIGHT - 8);
}

soldiers_t* create_soldiers(int tile_x, int tile_y)
{
    soldiers_t* soldiers = malloc(sizeof(soldiers_t));

    place_soldiers(soldiers, &ctx.tilemap[tile_y][tile_x]);

    // Default soldier units
    create_label(&soldiers->units_label, ctx.font);
    set_soldier_units(soldiers, UNITS_PER_TURN);

    soldiers->source_rect = (SDL_Rect) {0, 0, TILE_WIDTH, TILE_HEIGHT};

    return soldiers;
}

// Unrelated function that creates an explosion
void activate_explosion(int pos_x, int pos_y)
{
    set_transform_position(&ctx.explosion.sprite.transform, pos_x - 16, pos_y - 16);
    play_animated_sprite(&ctx.explosion);// Making it into a constant because it might change in the future
    
    play_audio(ctx.explosion_sfx);
}

void update_soldiers_texture(soldiers_t* soldiers)
{
    // If it's sea, pick a ship texture
    if (soldiers->current_tile->kind == TILE_WATER)
    {
        soldiers->source_rect.x = (soldiers->current_tile->owner_id + 1) * TILE_WIDTH;
    }
    else
    {
        soldiers->source_rect.x = 0;
    }
}

void capture_empty_neighbours(soldiers_t* soldiers, int tile_x, int tile_y)
{
    // Capture empty neighbouring tiles
    FOREACH_NEIGHBOUR
    {
        int x = GET_NEIGHBOUR_X_FROM(tile_x, tile_y);
        int y = GET_NEIGHBOUR_Y_FROM(tile_x, tile_y);

        if (!is_valid_tile(x, y)) continue;

        tile_t* tile = &ctx.tilemap[y][x];

        if (!tile->soldiers && tile->city_index < 0)
            tile->owner_id = soldiers->current_tile->owner_id;
    }
}

void move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    int sender_id = soldiers->current_tile->owner_id;

    // Check if the soldiers are trying to move to a sea tile without being on a port when on land
    if (tile->kind == TILE_WATER && (soldiers->current_tile->kind != TILE_PORT && soldiers->current_tile->kind != TILE_WATER))
        return;

    // Stores if the soldiers go from ship to land or from land to ship so it can update the texture later (XOR operator)
    bool will_change_surface = (tile->kind == TILE_WATER) ^ (soldiers->current_tile->kind == TILE_WATER);

    if (!tile->soldiers)
    {
        soldiers->current_tile->soldiers = NULL;
        tile->owner_id = sender_id;

        place_soldiers(soldiers, tile);
        capture_empty_neighbours(soldiers, tile_x, tile_y);
    
        if (will_change_surface)
            update_soldiers_texture(soldiers);

        ctx.remaining_moves--;
        return;
    }

    // Check if it's just a move between soldiers of the same player
    if (tile->owner_id == sender_id)
    {
        if (tile->soldiers->units == MAX_UNITS) return;

        unsigned int new_units = tile->soldiers->units + soldiers->units;

        if (new_units < MAX_UNITS)
        {
            set_soldier_units(tile->soldiers, new_units);
            destroy_soldiers(soldiers);
        }
        else
        {
            set_soldier_units(tile->soldiers, MAX_UNITS);
            set_soldier_units(soldiers, new_units - MAX_UNITS);
        }

        ctx.remaining_moves--;
        return;
    }

    if (tile->soldiers->units >= soldiers->units)
    {
        unsigned int remaining_units = tile->soldiers->units - soldiers->units;
        destroy_soldiers(soldiers);

        if (remaining_units == 0)
        {
            destroy_soldiers(tile->soldiers);
        }
        else
        {
            set_soldier_units(tile->soldiers, remaining_units);
        }
    }
    else
    {
        unsigned int remaining_units = soldiers->units - tile->soldiers->units;

        destroy_soldiers(soldiers);
        set_soldier_units(tile->soldiers, remaining_units);
        capture_empty_neighbours(soldiers, tile_x, tile_y);
    
        // Check if it was a capital
        if (tile->is_capital)
        {
            tile->is_capital = false;
            ctx.is_player_dead[tile->owner_id] = true;

            for (int i = 0; i < TILEMAP_WIDTH; i++)
            {
                for (int j = 0; j < TILEMAP_HEIGHT; j++)
                {
                    tile_t* other_tile = &ctx.tilemap[i][j];

                    if (other_tile->owner_id == tile->owner_id)
                        other_tile->owner_id = sender_id; 
                }
            }
        }
    }

    activate_explosion(tile->dest_rect.x, tile->dest_rect.y);
    ctx.remaining_moves--;
}

void set_soldier_units(soldiers_t* soldiers, unsigned int units) 
{
    units = MIN(MAX_UNITS, units);

    // Converting unsigned int to a c string
    char new_content[10];
    sprintf(new_content, "%u", units);

    soldiers->units = units;
    set_label_content(&soldiers->units_label, ctx.game.renderer, new_content);

    update_soldiers_texture(soldiers);
}

void select_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    // Check if the player owns the soldiers and then update the UI 
    if (soldiers->current_tile->owner_id != ctx.current_player_id) return;

    ctx.selected_soldiers = soldiers;

    // Highlighting neighbour tiles
    memset(&ctx.highlighted_tiles, 0, NEIGHBOURING_TILES * sizeof(tile_t*));

    tile_t* source = &ctx.tilemap[tile_y][tile_x];
    bool can_move_to_sea = soldiers->current_tile->kind != TILE_WATER || soldiers->current_tile->kind == TILE_WATER;

    FOREACH_NEIGHBOUR
    {
        int x = GET_NEIGHBOUR_X_FROM(tile_x, tile_y);
        int y = GET_NEIGHBOUR_Y_FROM(tile_x, tile_y);

        if (!is_valid_tile(x, y)) continue;

        tile_t* tile = &ctx.tilemap[y][x];

        // offset_i is coming from the macro! Bad design...
        if (tile->kind != TILE_WATER || can_move_to_sea)
            ctx.highlighted_tiles[offset_i] = &ctx.tilemap[y][x];
    }
}

void clear_selected_soldiers()
{
    // Clearing highlighted tiles
    memset(&ctx.highlighted_tiles, 0, NEIGHBOURING_TILES * sizeof(tile_t*));
    ctx.selected_soldiers = NULL;
}

void destroy_soldiers(soldiers_t* soldiers)
{
    destroy_label(&soldiers->units_label);
    soldiers->current_tile->soldiers = NULL;

    free(soldiers);
}
