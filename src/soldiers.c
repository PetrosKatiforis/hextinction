#include "soldiers.h"
#include "engine/utils.h"
#include "engine/audio.h"
#include "context.h"
#include "hex_utils.h"

// Assigns the soldiers' current_tile and moves the units label accordingly
void place_soldiers(soldiers_t* soldiers, tile_t* tile)
{
    tile->soldiers = soldiers;
    soldiers->current_tile = tile;

    set_transform_position(&soldiers->units_label.sprite.transform, tile->dest_rect.x + TILE_WIDTH - 8, tile->dest_rect.y + TILE_HEIGHT - 8);
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
        // TODO: Make the texture change depending on the units (10-30 soldiers, 30-60 cannon or something like that)
        soldiers->source_rect.x = 0;
    }
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

// Captures tile and adjusts total territories
void capture_tile(tile_t* dest, int sender_id)
{
    ctx.players[sender_id].total_territories++;

    if (dest->owner_id >= 0)
        ctx.players[dest->owner_id].total_territories--;

    dest->owner_id = sender_id;
}

soldiers_t* create_soldiers(int tile_x, int tile_y)
{
    // Soldiers are malloced so that they can be passed through tiles without copying them
    soldiers_t* soldiers = malloc(sizeof(soldiers_t));

    soldiers->source_rect = (SDL_Rect) {0, 0, TILE_WIDTH, TILE_HEIGHT};

    // Have to do this before place_soldiers because we must first be able to initialize the text
    soldiers->current_tile = &ctx.tilemap[tile_y][tile_x];

    // When soldiers are created, they need to wait for the next turn to be used
    soldiers->remaining_moves = 0;

    create_label(&soldiers->units_label, ctx.font, 0);
    set_soldier_units(soldiers, UNITS_PER_TRAIN);

    // Will position the text too
    place_soldiers(soldiers, &ctx.tilemap[tile_y][tile_x]);

    return soldiers;
}

// Unrelated function that creates an explosion
void activate_explosion(int pos_x, int pos_y)
{
    set_transform_position(&ctx.explosion.sprite.transform, pos_x - 16, pos_y - 16);
    play_animated_sprite(&ctx.explosion);// Making it into a constant because it might change in the future
    
    play_audio(ctx.cannon_sfx);
}

void capture_empty_neighbours(int sender_id, int tile_x, int tile_y)
{
    // Capture empty neighbouring tiles
    FOREACH_OFFSET(tile_y, TOTAL_NEIGHBOURS, i) 
    {
        int x = tile_x + neighbours_offsets[i][0];
        int y = tile_y + neighbours_offsets[i][1];

        if (!is_valid_tile(x, y)) continue;

        tile_t* tile = &ctx.tilemap[y][x];

        if (!tile->soldiers && tile->kind != TILE_CITY && !is_water(x, y) && tile->kind != TILE_FARM)
        {
            capture_tile(tile, sender_id);
        }
    }
}

void conquer_player(int attacker_id, int loser_id)
{
    ctx.players[loser_id].is_dead = true;

    MAP_FOREACH(x, y)
    {
        tile_t* tile = &ctx.tilemap[y][x];

        if (tile->owner_id == loser_id && !is_water(x, y))
        {
            capture_tile(tile, attacker_id);
        }
    }
}

bool move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    if (soldiers->remaining_moves == 0) return false;

    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    int sender_id = soldiers->current_tile->owner_id;

    // Check if the soldiers are trying to move to a sea tile without being on a port when on land
    if (is_water(tile_x, tile_y) && (soldiers->current_tile->kind != TILE_PORT && soldiers->current_tile->kind != TILE_WATER))
        return false;

    // Stores if the soldiers go from ship to land or from land to ship so it can update the texture later (XOR operator)
    bool will_change_surface = is_water(tile_x, tile_y) ^ (soldiers->current_tile->kind == TILE_WATER);

    unsigned int new_remaining_moves = soldiers->remaining_moves - 1;

    if (!tile->soldiers)
    {
        soldiers->current_tile->soldiers = NULL;

        if (tile->owner_id != sender_id)
        {
            // Destroy enemy farm on capture
            if (tile->kind == TILE_FARM)
                set_tile_kind(tile_x, tile_y, TILE_GRASS);
            
            else if (tile->kind == TILE_FISH)
            {
                set_tile_kind(tile_x, tile_y, TILE_WATER);
                ctx.players[sender_id].coins += FISH_INCOME;
            }

            if (tile->is_capital)
                conquer_player(sender_id, tile->owner_id);

            else
                capture_tile(tile, sender_id);
        }

        place_soldiers(soldiers, tile);
        soldiers->remaining_moves = new_remaining_moves;

        capture_empty_neighbours(sender_id, tile_x, tile_y);

        if (will_change_surface)
        {
            // Play the shipbell sound when a new ship is created
            if (soldiers->current_tile->kind == TILE_WATER)
            {
                play_audio(ctx.shipbell_sfx);
            }

            update_soldiers_texture(soldiers);
        }
        else
        {
            play_audio(ctx.dirt_sfx);
        }

        return true;
    }

     // Check if it's just a move between soldiers of the same player
    if (tile->owner_id == sender_id)
    {
        if (tile->soldiers->units == MAX_UNITS) return false;

        unsigned int new_units = tile->soldiers->units + soldiers->units;

        if (new_units <= MAX_UNITS)
        {
            set_soldier_units(tile->soldiers, new_units);
            destroy_soldiers(soldiers);
        }
        else
        {
            set_soldier_units(tile->soldiers, MAX_UNITS);
            set_soldier_units(soldiers, new_units - MAX_UNITS);
        }

        soldiers->remaining_moves = new_remaining_moves;

        return true;
    }

    // Handling battles
    int attack_result = tile->soldiers->units - soldiers->units;
    destroy_soldiers(soldiers);
         
    if (attack_result == 0)
    {
        destroy_soldiers(tile->soldiers);
    }
    else if (attack_result > 0)
    {
        set_soldier_units(tile->soldiers, attack_result);
        tile->soldiers->remaining_moves = new_remaining_moves;
    }
    // This is a victory
    else
    {    
        if (tile->kind == TILE_FARM)
            set_tile_kind(tile_x, tile_y, TILE_GRASS);

        // Conquering a capital should kill the player and make his kingdom part of the attacker's
        if (tile->is_capital)
        {
            tile->is_capital = false;
            conquer_player(sender_id, tile->owner_id);
        }
        else
            capture_tile(tile, sender_id);

        set_soldier_units(tile->soldiers, -attack_result);
        tile->soldiers->remaining_moves = new_remaining_moves;

        capture_empty_neighbours(sender_id, tile_x, tile_y);
    }

    activate_explosion(tile->dest_rect.x, tile->dest_rect.y);
    
    return true;
}

void select_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    // Check if the player owns the soldiers and then update the UI 
    if (soldiers->current_tile->owner_id != ctx.current_player_id) return;

    ctx.selected_soldiers = soldiers;

    // Highlighting neighbour tiles
    memset(&ctx.highlighted_tiles, 0, TOTAL_HIGHLIGHTED * sizeof(tile_t*));

    tile_t* source = &ctx.tilemap[tile_y][tile_x];
    bool can_move_to_sea = soldiers->current_tile->kind == TILE_WATER || soldiers->current_tile->kind == TILE_PORT;

    // Highlighting neighbours
    unsigned int total_highlighted = 0;

    FOREACH_OFFSET(tile_y, TOTAL_HIGHLIGHTED, i)
    {
        int x = tile_x + highlighted_offsets[i][0];
        int y = tile_y + highlighted_offsets[i][1];

        if (is_valid_tile(x, y))
        {
            tile_t* tile = &ctx.tilemap[y][x];

            if (!is_water(x, y) || can_move_to_sea)
                ctx.highlighted_tiles[total_highlighted++] = &ctx.tilemap[y][x];
        }
    }
}

void clear_selected_soldiers()
{
    // Clearing highlighted tiles
    memset(&ctx.highlighted_tiles, 0, TOTAL_HIGHLIGHTED * sizeof(tile_t*));
    ctx.selected_soldiers = NULL;
}

void destroy_soldiers(soldiers_t* soldiers)
{
    destroy_label(&soldiers->units_label);
    soldiers->current_tile->soldiers = NULL;

    free(soldiers);
}

