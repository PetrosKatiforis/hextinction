#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "context.h"
#include "hex_utils.h"
#include "engine/timers.h"
#include "engine/utils.h"
#include "libs/open-simplex/noise.h"

#define FRAME_DELAY 1000 / FRAMES_PER_SECOND

// Places grass at the specified position and removes visual glitches
void place_grass(int tile_x, int tile_y)
{
    create_tile(tile_x, tile_y, TILE_GRASS);

    if (is_valid_tile(tile_x, tile_y - 2) && ctx.tilemap[tile_y - 2][tile_x].kind == TILE_COAST)
    {
        set_tile_kind(tile_x, tile_y - 2, TILE_GRASS);
    }
}

void create_tilemap()
{
    MAP_FOREACH(x, y)
    {
        // Using simplex noise to find out what the tile will be
        double value = get_noise_value(x, y);
        double bottom = get_noise_value(x, y + 2);

        if (value > LAND_THRESHOLD && bottom < LAND_THRESHOLD)
        {
            create_tile(x, y, chance_one_in(8) ? TILE_PORT : TILE_COAST);
        }
        // Forests
        else if (value > FOREST_THRESHOLD)
        {
            create_tile(x, y, TILE_FOREST);
        }
        // Threshold for dirt placement
        else if (value > LAND_THRESHOLD)
        {
            create_tile(x, y, chance_one_in(8) ? TILE_FOREST : TILE_GRASS);
        }
        else
        {
            // Make sure that water tiles have a dest_rect position too
            assign_tile_position(x, y);
            set_tile_kind(x, y, TILE_WATER);
        }
    }

    // Generating the capitals at the four map edges (if no land is there, it will be generated with a port too)
    for (int i = 0; i < TOTAL_PLAYERS; i++)
    {
        int tile_x = capital_positions[i][0];
        int tile_y = capital_positions[i][1];

        tile_t* capital = &ctx.tilemap[tile_y][tile_x];

        // Making sure that it's a grass tile
        if (capital->kind == TILE_WATER)
            place_grass(tile_x, tile_y);

        capital->owner_id = i;
        capital->is_capital = true;

        create_city(tile_x, tile_y);
        capital->soldiers = create_soldiers(tile_x, tile_y);

        // Generating borders if they don't exist, plus a port
        bool has_created_port = false;

        FOREACH_NEIGHBOUR
        {
            int x = GET_NEIGHBOUR_X_FROM(tile_x, tile_y);
            int y = GET_NEIGHBOUR_Y_FROM(tile_x, tile_y);

            if (!is_valid_tile(x, y)) continue;

            tile_t* tile = &ctx.tilemap[y][x];

            // Make sure that land exists
            if (tile->kind == TILE_WATER)
                place_grass(x, y);

            // Generate a port because this might be an isolated island. The player needs a way to exit from there
            if (!has_created_port && is_valid_tile(x, y + 2) && ctx.tilemap[y + 2][x].kind == TILE_WATER)
            {
                set_tile_kind(x, y, TILE_PORT);
                has_created_port = true;
            }

            tile->owner_id = i;
        }
    }
}

void next_turn()
{
    // Spawn soldiers to every conquered city
    if (ctx.current_player_id >= 0)
    {
        MAP_FOREACH(x, y)
        {
            tile_t* tile = &ctx.tilemap[y][x];

            // Spawns if it's a city owned by the player
            if (tile->owner_id != ctx.current_player_id || tile->kind != TILE_CITY) continue;

            if (!tile->soldiers)
                create_soldiers(x, y);

            else if (tile->soldiers->units != MAX_UNITS)
                set_soldier_units(tile->soldiers, tile->soldiers->units + 10);
        }
    }

    ctx.current_player_id++;
    ctx.remaining_moves = MOVES_PER_TURN;

    if (ctx.current_player_id > TOTAL_PLAYERS - 1)
        ctx.current_player_id = 0;

    // Skip if the player is dead
    if (ctx.is_player_dead[ctx.current_player_id]) return next_turn();

    // Positioning turn arrow
    int capital_position[2];
    memcpy(capital_position, &capital_positions[ctx.current_player_id], 2 * sizeof(int));

    tile_t* capital = &ctx.tilemap[capital_position[1]][capital_position[0]];

    set_transform_position(&ctx.turn_arrow.transform, capital->dest_rect.x, capital->dest_rect.y - TILE_HEIGHT);
}

void generate_unclaimed_cities()
{
    for (int i = 0; i < TOTAL_CITIES; i++)
    {
        int tile_x, tile_y;
        tile_t* random_tile;

        do
        {
            tile_x = rand() % TILEMAP_WIDTH;
            tile_y = rand() % TILEMAP_HEIGHT;

            random_tile = &ctx.tilemap[tile_y][tile_x];
        
        // Trying until it finds a suitable tile
        } while (random_tile->city_index != -1 || random_tile->kind == TILE_WATER);

        create_city(tile_x, tile_y);
    }
}

void initialize_context()
{
    create_game(&ctx.game, "Hextinction - Early Development Stage", 25 + TILEMAP_WIDTH * (TILE_WIDTH + 16), TILEMAP_HEIGHT * 16 + 16);
    ctx.font = TTF_OpenFont("res/typewritter.ttf", 18);

    // Loading textures and audio
    ctx.tilemap_texture = load_texture("res/tilemap.png");
    ctx.border_texture = load_texture("res/border.png");
    ctx.soldiers_texture = load_texture("res/soldiers.png");

    create_animated_sprite(&ctx.explosion, load_texture("res/explosion.png"), 9, 100);
    set_transform_scale(&ctx.explosion.sprite.transform, 2);
    ctx.explosion_sfx = load_audio("res/explosion.wav");

    ctx.current_player_id = -1; // Will be set to 0 after next turn
    create_sprite(&ctx.turn_arrow, load_texture("res/arrow.png"));

    create_tilemap();
    generate_unclaimed_cities();
    next_turn();
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    
    // Initializing noise with seed coming from the arguments
    open_simplex_noise(argc > 1 ? atoi(argv[1]) : rand(), &ctx.noise_context);

    initialize_context();

    // Collecting events and defining the game loop
    SDL_Event event;

    simple_timer_t delta_timer;
    start_timer(&delta_timer);

    for (;;)
    {
        while (SDL_PollEvent(&event))
        {
            // Closing the game once an exit even has been received
            if (event.type == SDL_QUIT) goto finish_game;

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int tile_x, tile_y;
                window_to_tile_position(&tile_x, &tile_y, event.button.x, event.button.y);

                if (!is_valid_tile(tile_x, tile_y)) break;

                tile_t* tile = &ctx.tilemap[tile_y][tile_x];

                // Deselect if the player presses the selected soldiers
                if (tile->soldiers == ctx.selected_soldiers)
                {
                    clear_selected_soldiers();
                    break;
                }

                if (!ctx.selected_soldiers)
                {
                    select_soldiers(tile->soldiers, tile_x, tile_y);
                }
                // Move soldiers
                else
                {
                    if (ctx.selected_soldiers->current_tile->owner_id != ctx.current_player_id) break;

                    // First check if the tile is actually accessible
                    tile_t* source_tile = ctx.selected_soldiers->current_tile;
                    int source_x, source_y;

                    window_to_tile_position(&source_x, &source_y, source_tile->dest_rect.x, source_tile->dest_rect.y);
                    
                    if (!is_neighbouring_tile(source_x, source_y, tile_x, tile_y))
                    {
                        clear_selected_soldiers();
                        break;
                    }

                    set_label_color(&ctx.selected_soldiers->units_label, ctx.game.renderer, (SDL_Color) {255, 255, 255, 255});
                    
                    move_soldiers(ctx.selected_soldiers, tile_x, tile_y);
                    
                    // Check if the turn has ended
                    if (ctx.remaining_moves == 0)
                        next_turn();

                    clear_selected_soldiers();
                }
            }

            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    // If the space key is pressed, skip turn
                    case SDLK_SPACE:
                        next_turn();
                        break;
                }
            }
       }

        // Getting delta time in seconds
        float delta = restart_timer(&delta_timer) / 1000.0f;

        SDL_SetRenderDrawColor(ctx.game.renderer, 60, 60, 100, 255);
        SDL_RenderClear(ctx.game.renderer);

        // Rendering the tilemap
        MAP_FOREACH(x, y)
        {
            tile_t* tile = &ctx.tilemap[y][x];

            if (tile->kind != TILE_WATER)
            {
                SDL_RenderCopy(ctx.game.renderer, ctx.tilemap_texture, &tile->source_rect, &tile->dest_rect);

                // Render the border if its conquered on top of the tile
                if (tile->owner_id >= 0)
                {
                    SDL_Color* color = &player_colors[tile->owner_id];
                    
                    SDL_SetTextureColorMod(ctx.border_texture, color->r, color->g, color->b);
                    SDL_SetTextureAlphaMod(ctx.border_texture, color->a);
                    SDL_RenderCopy(ctx.game.renderer, ctx.border_texture, NULL, &tile->dest_rect);
                }
            }
        }

        // Drawing soldiers on top of tiles
        MAP_FOREACH(x, y)
        {
            tile_t* tile = &ctx.tilemap[y][x];

            if (tile->soldiers)
            {
                SDL_RenderCopy(ctx.game.renderer, ctx.soldiers_texture, &tile->soldiers->source_rect, &tile->dest_rect);
                render_sprite(&tile->soldiers->units_label.sprite, ctx.game.renderer);
            }
        }

        // Drawing preview tiles
        for (int i = 0; i < NEIGHBOURING_TILES; i++)
        {
            tile_t* tile = ctx.highlighted_tiles[i];

            if (tile)
            {
                SDL_SetTextureColorMod(ctx.border_texture, highlight_color.r, highlight_color.g, highlight_color.b);
                SDL_SetTextureAlphaMod(ctx.border_texture, highlight_color.a);
                SDL_RenderCopy(ctx.game.renderer, ctx.border_texture, NULL, &tile->dest_rect);
            }
        }

        // Rendering city labels
        for (int i = 0; i < TOTAL_CITIES + TOTAL_PLAYERS; i++)
            render_sprite(&ctx.city_labels[i].sprite, ctx.game.renderer);

        if (ctx.explosion.is_active)
            render_animated_sprite(&ctx.explosion, ctx.game.renderer);

        render_sprite(&ctx.turn_arrow, ctx.game.renderer);

        SDL_RenderPresent(ctx.game.renderer);
        SDL_Delay(FRAME_DELAY);
    }

finish_game:
    free_game(&ctx.game);
}
