#include <stdio.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "context.h"
#include "hex_utils.h"
#include "engine/timers.h"
#include "engine/utils.h"
#include "libs/open-simplex/noise.h"

#define FRAME_DELAY 1000 / FRAMES_PER_SECOND

void assign_tile_position(int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    // Some magic numbers here, I didn't want to find a precise formula for it so I'm just doing it manually with trial and error
    int offset = (tile_y % 2 == 1) ? 25 : 0;

    tile->dest_rect = (SDL_Rect) {offset + tile_x * (TILE_WIDTH + 16), tile_y * TILE_HEIGHT / 2, TILE_WIDTH, TILE_HEIGHT};
}

// Texture index refers to x position in tilemap texture
void create_tile(int x, int y, int texture_index)
{
    tile_t* tile = &ctx.tilemap[y][x];

    // Make it likely that a port will spawn if it's a coast
    if (texture_index == 2 && y < TILEMAP_HEIGHT - 2 && (rand() % 7 == 0))
    {
        tile->is_port = true;
        texture_index = 4;
    }

    // Picking the tile from the tilemap texture and saving its position
    tile->source_rect = (SDL_Rect) {texture_index * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
    assign_tile_position(x, y);

    tile->is_walkable = true;
    tile->city_index = -1;
    tile->owner_id = -1;
}

void create_city(tile_t* tile)
{
    // Picking a random name
    tile->city_index = rand() % (sizeof(city_names) / CITY_NAME_LEN);
    tile->source_rect.x = 3 * TILE_WIDTH;
            
    // Creating the text label, loops through the pool instead of a dynamic array because it's small
    for (int i = 0; i < TOTAL_PLAYERS + TOTAL_CITIES; i++)
    {
        label_t* label = &ctx.city_labels[i];

        // Ignore it if it's already used
        if (label->sprite.texture != NULL) continue;

        create_label(label, ctx.font);
        set_label_content(label, ctx.game.renderer, city_names[tile->city_index]);
        set_transform_position(&label->sprite.transform, tile->dest_rect.x + 40, tile->dest_rect.y + 4);

        break;
    }
}

double get_noise_value(int tile_x, int tile_y)
{
    return open_simplex_noise2(ctx.noise_context, tile_x / 8.0, tile_y / 8.0);
}

void create_tilemap()
{
    for (int i = 0; i < TILEMAP_HEIGHT; i++)
    {
        for (int j = 0; j < TILEMAP_WIDTH; j++)
        {
            // Using simplex noise to find out what the tile will be
            double value = get_noise_value(j, i);
            double bottom = get_noise_value(j, i + 2);

            if (value > LAND_THRESHOLD && bottom < LAND_THRESHOLD)
            {
                create_tile(j, i, 2);
            }
            // Forests
            else if (value > FOREST_THRESHOLD)
            {
                create_tile(j, i, 1);
            }
            // Threshold for dirt placement
            else if (value > LAND_THRESHOLD)
            {
                bool is_forest = rand() % 8 == 0;

                create_tile(j, i, is_forest ? 1 : 0);
            }
            else
            {
                // Make sure that water tiles have a dest_rect position too
                assign_tile_position(j, i);
                ctx.tilemap[i][j].city_index = -1;
            }
        }
    }

    // Generating the capitals at the four map edges (if no land is there, it will be generated with a port too)
    for (int i = 0; i < TOTAL_PLAYERS; i++)
    {
        int position_x = capital_positions[i][0];
        int position_y = capital_positions[i][1];

        tile_t* capital = &ctx.tilemap[position_y][position_x];

        // Making sure that it's a grass tile
        if (!capital->is_walkable)
            create_tile(position_x, position_y, 0);

        capital->owner_id = i;
        capital->is_capital = true;

        create_city(capital);
        capital->soldiers = create_soldiers(position_x, position_y);

        bool has_created_port = false;

        FOREACH_NEIGHBOUR
        {
            int x = GET_NEIGHBOUR_X_FROM(position_x, position_y);
            int y = GET_NEIGHBOUR_Y_FROM(position_x, position_y);

            if (!is_valid_tile(x, y)) continue;

            tile_t* tile = &ctx.tilemap[y][x];

            // Make sure that land exists
            if (!tile->is_walkable)
            {
                create_tile(x, y, 0);

                if (!has_created_port && get_noise_value(x, y + 1) < LAND_THRESHOLD)
                {
                    tile->is_port = true;
                    tile->source_rect.x = 4 * TILE_WIDTH;

                    has_created_port = true;
                }
            }

            tile->owner_id = i;
        }
    }
}

void generate_unclaimed_cities()
{
    for (int i = 0; i < TOTAL_CITIES; i++)
    {
        tile_t* random_tile;

        do
        {
            random_tile = &ctx.tilemap[rand() % TILEMAP_HEIGHT][rand() % TILEMAP_WIDTH];
        
        // Trying until it finds a suitable tile
        } while (random_tile->city_index != -1 || !random_tile->is_walkable);

        create_city(random_tile);
    }
}

void initialize_context()
{
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
    create_game(&ctx.game, "Hextinction - Early Development Stage", 25 + TILEMAP_WIDTH * (TILE_WIDTH + 16), TILEMAP_HEIGHT * 16 + 16);

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
        for (int i = 0; i < TILEMAP_WIDTH; i++)
        {
            for (int j = 0; j < TILEMAP_HEIGHT; j++)
            {
                tile_t* tile = &ctx.tilemap[j][i];

                if (tile->is_walkable)
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
        }

        // Drawing soldiers on top of tiles
        for (int i = 0; i < TILEMAP_WIDTH; i++)
        {
            for (int j = 0; j < TILEMAP_HEIGHT; j++)
            {
                tile_t* tile = &ctx.tilemap[j][i];

                if (tile->soldiers)
                {
                    SDL_RenderCopy(ctx.game.renderer, ctx.soldiers_texture, &tile->soldiers->source_rect, &tile->dest_rect);
                    render_sprite(&tile->soldiers->units_label.sprite, ctx.game.renderer);
                }
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
        {
            render_sprite(&ctx.city_labels[i].sprite, ctx.game.renderer);
        }

        if (ctx.explosion.is_active)
            render_animated_sprite(&ctx.explosion, ctx.game.renderer);

        render_sprite(&ctx.turn_arrow, ctx.game.renderer);

        SDL_RenderPresent(ctx.game.renderer);
        SDL_Delay(FRAME_DELAY);
    }

finish_game:
    free_game(&ctx.game);
}
