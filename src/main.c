#include <stdio.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "context.h"
#include "hex_utils.h"
#include "engine/timers.h"
#include "engine/utils.h"
#include "libs/open-simplex/noise.h"

#define FRAMES_PER_SECOND 60
#define FRAME_DELAY 1000 / FRAMES_PER_SECOND

// Texture index refers to x position in tilemap texture
void create_tile(int x, int y, int texture_index)
{
    tile_t* tile = &ctx.tilemap[y][x];

    // Some magic numbers here, I didn't want to find a precise formula for it so I'm just doing it manually with trial and error
    int offset = (y % 2 == 1) ? 25 : 0;

    // Picking the tile from the tilemap texture and saving its position
    tile->source_rect = (SDL_Rect) {texture_index * TILE_WIDTH, 0, TILE_WIDTH, TILE_HEIGHT};
    tile->dest_rect = (SDL_Rect) {offset + x * (TILE_WIDTH + 16), y * TILE_HEIGHT / 2, TILE_WIDTH, TILE_HEIGHT};

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

        create_label(label, ctx.game.renderer, ctx.font, city_names[tile->city_index]);
        set_transform_position(&label->sprite.transform, tile->dest_rect.x + 40, tile->dest_rect.y + 4);

        break;
    }
}

void create_tilemap()
{
    for (int i = 0; i < TILEMAP_HEIGHT; i++)
    {
        for (int j = 0; j < TILEMAP_WIDTH; j++)
        {
            // Using simplex noise to find out what the tile will be
            double value = open_simplex_noise2(ctx.noise_context, j / 8.0, i / 8.0);

            // Forests
            if (value > 0.3)
            {
                create_tile(j, i, 1);
            }
            // Threshold for dirt placement
            else if (value > -0.3)
            {
                bool is_forest = rand() % 8 == 0;
                bool is_coast = open_simplex_noise2(ctx.noise_context, j / 8.0, (i + 2) / 8.0) < -0.3;

                create_tile(j, i, is_coast ? 2 : is_forest ? 1 : 0);
            }
        }
    }

    // Generating the capitals at the four map edges (if no land is there, it will be generated)
    for (int i = 0; i < TOTAL_PLAYERS; i++)
    {
        int position_x = capital_positions[i][0];
        int position_y = capital_positions[i][1];

        tile_t* capital = &ctx.tilemap[position_y][position_x];

        // Making sure that it's a grass tile
        if (!capital->is_walkable)
            create_tile(position_x, position_y, 0);

        capital->owner_id = i;

        create_city(capital);
        capital->soldiers = create_soldiers(position_x, position_y);

        FOREACH_NEIGHBOUR
        {
            int x = GET_NEIGHBOUR_X_FROM(position_x, position_y);
            int y = GET_NEIGHBOUR_Y_FROM(position_x, position_y);

            if (!is_valid_tile(x, y)) continue;

            tile_t* tile = &ctx.tilemap[y][x];

            // Make sure that land exists
            if (!tile->is_walkable) create_tile(x, y, 0);

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
    create_game(&ctx.game, "Hextinction - Early Development Stage", 25 + TILEMAP_WIDTH * (TILE_WIDTH + 16), TILEMAP_HEIGHT * 16 + 16);
    ctx.font = TTF_OpenFont("res/typewritter.ttf", 18);

    // Loading textures and audio
    ctx.tilemap_texture = load_texture("res/tilemap.png");
    ctx.border_texture = load_texture("res/border.png");
    ctx.soldier_texture = load_texture("res/soldier.png");

    create_animated_sprite(&ctx.explosion, load_texture("res/explosion.png"), 9, 100);
    set_transform_scale(&ctx.explosion.sprite.transform, 2);
    ctx.explosion_sfx = load_audio("res/explosion.wav");

    create_tilemap();
    generate_unclaimed_cities();
}

int main(int argc, char** argv)
{
    // Initializing noise with seed coming from the arguments
    open_simplex_noise(argc > 1 ? atoi(argv[1]) : 1239310, &ctx.noise_context);

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
                
                if (!tile->is_walkable) break;

                if (!ctx.selected_soldiers)
                {
                    if (tile->soldiers) select_soldiers(tile->soldiers);
                }
                else if (tile->soldiers == ctx.selected_soldiers)
                {
                    clear_selected_soldiers();
                }
                // Move soldiers
                else
                {
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

                    ctx.selected_soldiers = NULL;
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
        for (int i = 0; i < TILEMAP_WIDTH; i++)
        {
            for (int j = 0; j < TILEMAP_HEIGHT; j++)
            {
                tile_t* tile = &ctx.tilemap[j][i];

                if (tile->soldiers)
                {
                    SDL_RenderCopy(ctx.game.renderer, ctx.soldier_texture, NULL, &tile->dest_rect);
                    render_sprite(&tile->soldiers->units_label.sprite, ctx.game.renderer);
                }
            }
        }

        // Rendering city labels
        for (int i = 0; i < TOTAL_CITIES + TOTAL_PLAYERS; i++)
        {
            render_sprite(&ctx.city_labels[i].sprite, ctx.game.renderer);
        }

        if (ctx.explosion.is_active)
            render_animated_sprite(&ctx.explosion, ctx.game.renderer);

        SDL_RenderPresent(ctx.game.renderer);
        SDL_Delay(FRAME_DELAY);
    }

finish_game:
    free_game(&ctx.game);
}
