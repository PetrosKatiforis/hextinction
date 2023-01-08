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
        soldiers->source_rect.x = (soldiers->current_tile->owner_id + 2) * TILE_WIDTH;
    }
    else
    {
        soldiers->source_rect.x = soldiers->kind * TILE_WIDTH;
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

bool try_to_train_soldiers(int tile_x, int tile_y, soldier_kind_e choice)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    player_t* current_player = &ctx.players[ctx.current_player_id];
    
    int cost = SOLDIER_COSTS[choice];
    if (current_player->coins < cost) return false;

    switch (choice)
    {
        case SOLDIER_SABOTEUR:
            if (tile->soldiers) return false;

            tile->soldiers = create_soldiers(tile_x, tile_y, choice);

            break;

        case SOLDIER_KNIGHT:
            if (!tile->soldiers)
                create_soldiers(tile_x, tile_y, choice);
            else
                set_soldier_units(tile->soldiers, tile->soldiers->units + KNIGHTS_PER_TRAIN);

            current_player->total_units += KNIGHTS_PER_TRAIN;

            break;
    }

    current_player->coins -= cost;    
    tile->soldiers->remaining_moves = 0;
    
    return true;
}

// Captures tile and adjusts total territories
void capture_tile(int tile_x, int tile_y, int sender_id)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    ctx.players[sender_id].total_territories++;
    bool was_neutral = tile->owner_id < 0;

    if (!was_neutral)
        ctx.players[tile->owner_id].total_territories--; 

    // Handle some special cases
    switch (tile->kind)
    {
        case TILE_FARM:
        case TILE_BROKEN_FARM:
            // I don't need to check if it's owned, but just in case
            if (!was_neutral) ctx.players[tile->owner_id].total_farms--;
        
            set_tile_kind(tile_x, tile_y, TILE_GRASS);

            break;

        case TILE_FISH:
            set_tile_kind(tile_x, tile_y, TILE_WATER);
            ctx.players[sender_id].coins += FISH_INCOME;
            
            break;

        case TILE_CITY:
            if (!was_neutral) ctx.players[tile->owner_id].total_cities--;
            ctx.players[sender_id].total_cities++;
            
            break;
    }

    tile->owner_id = sender_id;
}

soldiers_t* create_soldiers(int tile_x, int tile_y, soldier_kind_e kind)
{
    // Soldiers are malloced so that they can be passed through tiles without copying them
    soldiers_t* soldiers = malloc(sizeof(soldiers_t));

    soldiers->kind = kind;
    soldiers->source_rect = (SDL_Rect) {kind * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};

    // Have to do this before place_soldiers because we must first be able to initialize the text
    soldiers->current_tile = &ctx.tilemap[tile_y][tile_x];

    // When soldiers are created, they need to wait for the next turn to be used
    soldiers->remaining_moves = 0;
    soldiers->units = 0;

    if (kind == SOLDIER_KNIGHT)
    {
        create_label(&soldiers->units_label, ctx.font, 0);
        set_soldier_units(soldiers, KNIGHTS_PER_TRAIN);
    }

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
            capture_tile(x, y, sender_id);
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
            capture_tile(x, y, attacker_id);
        }
    }
}

bool move_soldiers(soldiers_t* soldiers, int tile_x, int tile_y)
{
    if (soldiers->remaining_moves == 0) return false;

    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    int sender_id = soldiers->current_tile->owner_id;
    int enemy_id = tile->owner_id;

    // Check if the soldiers are trying to move to a sea tile without being on a port when on land
    if (is_water(tile_x, tile_y) && (soldiers->current_tile->kind != TILE_PORT && soldiers->current_tile->kind != TILE_WATER))
        return false;

    // Stores if the soldiers go from ship to land or from land to ship so it can update the texture later (XOR operator)
    bool will_change_surface = is_water(tile_x, tile_y) ^ (soldiers->current_tile->kind == TILE_WATER);

    unsigned int new_remaining_moves = soldiers->remaining_moves - 1;

    if (!tile->soldiers)
    {
        if (enemy_id != sender_id)
        {
            // Saboteurs can only claim empty land and farms
            if (soldiers->kind == SOLDIER_SABOTEUR && tile->kind == TILE_CITY) return false;
            

            if (tile->is_capital)
                conquer_player(sender_id, tile->owner_id);
            else
                capture_tile(tile_x, tile_y, sender_id);
        }

        soldiers->current_tile->soldiers = NULL;

        place_soldiers(soldiers, tile);
        soldiers->remaining_moves = new_remaining_moves;

        if (soldiers->kind != SOLDIER_SABOTEUR)
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
    if (enemy_id == sender_id)
    {
        // Can only combine soldiers of the same type
        if (tile->soldiers->kind != soldiers->kind) return false;

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

            soldiers->remaining_moves = new_remaining_moves;
        }

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
    }
    // This is a victory
    else
    {    
        // Conquering a capital should kill the player and make his kingdom part of the attacker's
        if (tile->is_capital)
        {
            tile->is_capital = false;
            conquer_player(sender_id, enemy_id);
        }
        else
            capture_tile(tile_x, tile_y, sender_id);

        // If the enemy was a saboteur, make turn him into a knight
        if (tile->soldiers->kind == SOLDIER_SABOTEUR)
        {
            create_label(&tile->soldiers->units_label, ctx.font, 0);
            tile->soldiers->kind = SOLDIER_KNIGHT;
        }

        set_soldier_units(tile->soldiers, -attack_result);
        tile->soldiers->remaining_moves = new_remaining_moves;

        capture_empty_neighbours(sender_id, tile_x, tile_y);
    }

    ctx.players[sender_id].total_units -= attack_result;
    ctx.players[enemy_id].total_units += attack_result;

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

void render_soldiers(soldiers_t* soldiers, SDL_Renderer* renderer, SDL_Texture* soldiers_texture)
{
    // Visual indication that the soldier cannot be moved
    if (soldiers->remaining_moves == 0)
        SDL_SetTextureColorMod(soldiers_texture, 160, 160, 160);
    else
        SDL_SetTextureColorMod(soldiers_texture, 255, 255, 255);

    SDL_RenderCopy(renderer, soldiers_texture, &soldiers->source_rect, &soldiers->current_tile->dest_rect);

    if (soldiers->kind == SOLDIER_KNIGHT)
        render_sprite(&soldiers->units_label.sprite, renderer);
}

void clear_selected_soldiers()
{
    // Clearing highlighted tiles
    memset(&ctx.highlighted_tiles, 0, TOTAL_HIGHLIGHTED * sizeof(tile_t*));
    ctx.selected_soldiers = NULL;
}

void destroy_soldiers(soldiers_t* soldiers)
{
    if (soldiers->kind == SOLDIER_KNIGHT)
        destroy_label(&soldiers->units_label);

    soldiers->current_tile->soldiers = NULL;

    free(soldiers);
}

