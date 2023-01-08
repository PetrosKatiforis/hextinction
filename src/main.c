#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "context.h"
#include "hex_utils.h"
#include "engine/utils.h"
#include "libs/open-simplex/noise.h"

// Places grass at the specified position and removes visual glitches
void place_grass(int tile_x, int tile_y)
{
    create_tile(tile_x, tile_y, TILE_GRASS);

    if (is_valid_tile(tile_x, tile_y - 2) && ctx.tilemap[tile_y - 2][tile_x].kind == TILE_COAST)
    {
        set_tile_kind(tile_x, tile_y - 2, TILE_GRASS);
    }
}

void create_interface()
{
    // Creating the info panel at the right side of the screen
    create_label(&ctx.player_name, ctx.font, 0);
    set_transform_position(&ctx.player_name.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 20);

    create_label(&ctx.player_description, ctx.font, 200);
    set_transform_position(&ctx.player_description.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 210);
    
    create_label(&ctx.player_territories, ctx.font, 0);
    set_transform_position(&ctx.player_territories.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 400);

    create_label(&ctx.player_coins, ctx.font, 0);
    set_transform_position(&ctx.player_coins.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 425);

    create_label(&ctx.player_income, ctx.font, 0);
    set_transform_position(&ctx.player_income.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 450);

    create_label(&ctx.player_moves, ctx.font, 0);
    // +2 because for some reason M appears a bit off in this font
    set_transform_position(&ctx.player_moves.sprite.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING + 2, 475);

    create_sprite(&ctx.player_profile, load_texture("res/profiles.png"));
    set_transform_position(&ctx.player_profile.transform, TOTAL_TILEMAP_WIDTH + PANEL_PADDING, 50);

    // Manually setting scale and source rect dimensions
    ctx.player_profile.source_rect.w = 64;
    ctx.player_profile.transform.rect.w = ctx.player_profile.transform.rect.h = 128;

    ctx.panel_rect = (SDL_Rect) {TOTAL_TILEMAP_WIDTH, 0, PANEL_WIDTH, TOTAL_TILEMAP_HEIGHT};

    // Creating the dropdowns
    char farm_text[30];
    sprintf(farm_text, "Build Farm (%d Coins)", FARM_COST);

    create_dropdown(&ctx.build_dropdown, ctx.game.renderer, ctx.font, 1, farm_text);

    char knight_text[40], saboteur_text[40];
    sprintf(knight_text, "Train Knight (%d Coins)", SOLDIER_COSTS[SOLDIER_KNIGHT]);
    sprintf(saboteur_text, "Train Saboteur (%d Coins)", SOLDIER_COSTS[SOLDIER_SABOTEUR]);

    create_dropdown(&ctx.train_dropdown, ctx.game.renderer, ctx.font, 2, knight_text, saboteur_text);
}

void update_stats()
{
    player_t* player = &ctx.players[ctx.current_player_id];

    // Calculating income here
    player->income = FARM_INCOME * player->total_farms + CITY_INCOME * player->total_cities
        - (player->total_units / 10) * COST_PER_10_UNITS + player->total_territories / TERRITORIES_PER_COIN;

    // Collecting coins and territories to strings
    char territories[30], coins[20], income[20];
    sprintf(coins, "Coins: %d", ctx.players[ctx.current_player_id].coins);
    sprintf(territories, "Territories: %d", ctx.players[ctx.current_player_id].total_territories);
    sprintf(income, "Income: %d", ctx.players[ctx.current_player_id].income);

    set_label_content(&ctx.player_coins, ctx.game.renderer, coins);
    set_label_content(&ctx.player_territories, ctx.game.renderer, territories);
    set_label_content(&ctx.player_income, ctx.game.renderer, income);
}

void update_interface()
{
    set_label_content(&ctx.player_name, ctx.game.renderer, player_names[ctx.current_player_id]);
    set_label_content(&ctx.player_description, ctx.game.renderer, player_descriptions[ctx.current_player_id]);

    ctx.player_profile.source_rect.x = 64 * ctx.current_player_id;
    update_stats();
}

void update_moves_label()
{
    char moves_text[20];
    sprintf(moves_text, "Moves Left: %d", ctx.remaining_moves);

    set_label_content(&ctx.player_moves, ctx.game.renderer, moves_text);
}

void create_tilemap()
{
    MAP_FOREACH(x, y)
    {
        // Using simplex noise to find out what the tile will be
        double value = get_noise_value(x, y);
        double bottom = get_noise_value(x, y + 2);
        double top = get_noise_value(x, y - 2);

        // All bottom tiles should look like coasts
        if (value > LAND_START && y > TILEMAP_HEIGHT - 3)
        {
            create_tile(x, y, TILE_COAST);
        }
        else if (value > LAND_START && bottom < LAND_START)
        {
            create_tile(x, y, chance_one_in(10) ? TILE_PORT : TILE_COAST);
        }
        else if (value > LAND_START && top < LAND_START && chance_one_in(10))
        {
            create_tile(x, y, TILE_PORT);
        }
        // Forests
        else if (value > FOREST_START)
        {
            create_tile(x, y, TILE_FOREST);
        }
        // Threshold for dirt placement
        else if (value > LAND_START)
        {
            create_tile(x, y, chance_one_in(7) ? TILE_FOREST : TILE_GRASS);
        }
        else if (chance_one_in(10))
        {
            create_tile(x, y, TILE_FISH);
        }
        else
        {
            // Make sure that water tiles have a dest_rect position too
            assign_tile_position(x, y);
            set_tile_kind(x, y, TILE_WATER);
        }
    }

    // Generating the capitals at the four map edges (if no land is there, it will be generated with a port too)
    for (int player_id = 0; player_id < ctx.starting_players; player_id++)
    {
        int tile_x = capital_positions[player_id][0];
        int tile_y = capital_positions[player_id][1];

        tile_t* capital = &ctx.tilemap[tile_y][tile_x];

        // Making sure that it's a grass tile
        if (is_water(tile_x, tile_y))
            place_grass(tile_x, tile_y);

        capture_tile(capital, player_id);
        create_city(tile_x, tile_y);

        capital->is_capital = true;
        capital->soldiers = create_soldiers(tile_x, tile_y, SOLDIER_KNIGHT);
        capital->soldiers->remaining_moves = SOLDIER_MOVES[SOLDIER_KNIGHT];
        
        ctx.players[player_id].total_units = KNIGHTS_PER_TRAIN;
        ctx.players[player_id].coins = STARTING_COINS;

        FOREACH_OFFSET(tile_y, TOTAL_NEIGHBOURS, j) 
        {
            int x = tile_x + neighbours_offsets[j][0];
            int y = tile_y + neighbours_offsets[j][1];

            if (!is_valid_tile(x, y)) continue;
            tile_t* neighbour = &ctx.tilemap[y][x];

            // Make sure that land exists
            if (is_water(x, y))
            {
                place_grass(x, y);

                // Fix some visual glitches
                if (y > TILEMAP_HEIGHT - 3 || y > tile_y && is_valid_tile(x, y + 2) && is_water(x, y + 2))
                    set_tile_kind(x, y, TILE_COAST);
            }

            capture_tile(neighbour, player_id);
        }

        // Generating a port at the bottom and at the top tile if the player needs a way to exit his isolated island 
        // NOTE: The player can still be blocked, but the probability is really low
        if (is_valid_tile(tile_x, tile_y + 4) && is_water(tile_x, tile_y + 4))
            set_tile_kind(tile_x, tile_y + 2, TILE_PORT);
    
        if (is_valid_tile(tile_x, tile_y - 4) && is_water(tile_x, tile_y - 4))
            set_tile_kind(tile_x, tile_y - 2, TILE_PORT);
    }
}

void next_turn()
{
    // Applying old player's income
    player_t* old_player = &ctx.players[ctx.current_player_id];
    old_player->coins += old_player->income;

    // Resetting soldier moves
    MAP_FOREACH(x, y)
    {
        tile_t* tile = &ctx.tilemap[y][x];

        if (tile->soldiers && tile->owner_id == ctx.current_player_id)
            tile->soldiers->remaining_moves = SOLDIER_MOVES[tile->soldiers->kind];
    }

    // Going to the next player
    ctx.current_player_id++;
    ctx.selected_soldiers = NULL;
    ctx.remaining_moves = MOVES_PER_TURN;

    update_moves_label();

    if (ctx.current_player_id > ctx.starting_players - 1)
        ctx.current_player_id = 0;

    // Skip if the player is dead
    if (ctx.players[ctx.current_player_id].is_dead) return next_turn();

    // Check if the player is bankrupt
    if (ctx.players[ctx.current_player_id].coins < 0)
    {
        ctx.players[ctx.current_player_id].coins = 0;

        MAP_FOREACH(x, y)
        {
            tile_t* tile = &ctx.tilemap[y][x];

            if (tile->owner_id == ctx.current_player_id && tile->soldiers != NULL)
                destroy_soldiers(tile->soldiers);
        }
    }

    update_interface();

    // Positioning turn arrow
    int* capital_position = capital_positions[ctx.current_player_id];

    tile_t* capital = &ctx.tilemap[capital_position[1]][capital_position[0]];

    set_transform_position(&ctx.turn_arrow.transform, capital->dest_rect.x, capital->dest_rect.y - TILE_HEIGHT);
}

void generate_unclaimed_cities()
{
    int current_x = 0;
    int current_y = 0;

    while (current_y < TILEMAP_HEIGHT - 3)
    {
        int offset_x = rand() % 2;
        int offset_y = rand() % 3;

        tile_t* tile = &ctx.tilemap[current_y + offset_y][current_x + offset_x];

        // Spawn only in empty grass/forest tiles
        if (tile->owner_id < 0 && tile->kind == TILE_GRASS || tile->kind == TILE_FOREST)
        {
            create_city(current_x + offset_x, current_y + offset_y);
        }

        // This algorithm moves in "chunks" and just determines some offset for a more organic result
        current_x += 3;

        if (current_x > TILEMAP_WIDTH - 2)
        {
            current_x = 0;
            current_y += 5;
        }
    }
}

void decrement_move()
{
    ctx.remaining_moves--;

    // Check if the turn has ended
    if (ctx.remaining_moves == 0)
        next_turn();

    update_moves_label();
}

// Handles soldier selection and attacks
void handle_click(int tile_x, int tile_y)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];

    // Deselect if the player presses the selected soldiers
    if (tile->soldiers == ctx.selected_soldiers)
    {
        clear_selected_soldiers();

        return;
    }

    if (!ctx.selected_soldiers)
    {
        if (tile->soldiers->remaining_moves == 0) return;

        select_soldiers(tile->soldiers, tile_x, tile_y);
    }
    // Move soldiers
    else
    {
        if (ctx.selected_soldiers->current_tile->owner_id != ctx.current_player_id) return;

        // First check if the tile is actually accessible
        tile_t* source_tile = ctx.selected_soldiers->current_tile;
        int source_x, source_y;

        window_to_tile_position(&source_x, &source_y, source_tile->dest_rect.x, source_tile->dest_rect.y);
        
        /*if (!is_neighbouring_tile(source_x, source_y, tile_x, tile_y))
        {
            clear_selected_soldiers();
            
            return;
        }*/

        if (move_soldiers(ctx.selected_soldiers, tile_x, tile_y))
        {
            update_stats();
            decrement_move();
        }
        
        clear_selected_soldiers();
    }
}

void handle_build(int tile_x, int tile_y, int choice)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    if (tile->owner_id != ctx.current_player_id) return;
    
    player_t* current_player = &ctx.players[ctx.current_player_id];

    switch (choice)
    {
        // Farm creation
        case 0:
            if (tile->kind == TILE_GRASS && current_player->coins >= FARM_COST)
            {
                current_player->coins -= FARM_COST;
                current_player->total_farms++;

                set_tile_kind(tile_x, tile_y, TILE_FARM);
                update_stats();
                decrement_move();
            }

            break;
    }
}

void handle_train(int tile_x, int tile_y, int choice)
{
    tile_t* tile = &ctx.tilemap[tile_y][tile_x];
    if (tile->owner_id != ctx.current_player_id) return;
    
    player_t* current_player = &ctx.players[ctx.current_player_id];

    // Using choice index as enum
    if (try_to_train_soldiers(tile_x, tile_y, choice))
    {
        play_audio(ctx.military_sfx);
        decrement_move();
        update_stats();
    }
}

void initialize_context()
{
    // Settings the frame rate to just 20, big performance boost
    create_game(&ctx.game, "Hextinction", TOTAL_TILEMAP_WIDTH + PANEL_WIDTH, TOTAL_TILEMAP_HEIGHT, 20);
    ctx.font = TTF_OpenFont("res/free_mono.ttf", 18);

    // Loading textures and audio
    ctx.tilemap_texture = load_texture("res/tilemap.png");
    ctx.border_texture = load_texture("res/border.png");
    ctx.soldiers_texture = load_texture("res/soldiers.png");

    ctx.dirt_sfx = load_audio("res/dirt.wav");
    ctx.cannon_sfx = load_audio("res/cannon.wav");
    ctx.shipbell_sfx = load_audio("res/shipbell.wav");
    ctx.military_sfx = load_audio("res/military.wav");

    create_animated_sprite(&ctx.explosion, load_texture("res/explosion.png"), 9, 100);
    set_transform_scale(&ctx.explosion.sprite.transform, 2);

    ctx.current_player_id = -1; // Will be set to 0 after initial next turn
    create_sprite(&ctx.turn_arrow, load_texture("res/arrow.png"));

    create_tilemap();
    generate_unclaimed_cities();
    create_interface();
    next_turn();
}

void display_help_and_exit()
{
    printf(
        "Instructions on how to run Hextinction and what arguments it expects:\n"
        "   ./hextiction [1-4 int, total_players] [optional int, map_seed]\n\n"
        "   NOTES:       if no seed is passed, it will pick one randomly\n"
    );

    exit(EXIT_FAILURE);
}

void parse_console_arguments(int argc, char** argv)
{
    // Remember that the first argument is the actual file name
    if (argc < 2)
    {
        display_help_and_exit();
    }

    // Initializing some context state according to the passed arguments
    int number_of_players = atoi(argv[1]);

    if (number_of_players < 1 || number_of_players > TOTAL_PLAYERS)
    {
        display_help_and_exit();
    }

    ctx.starting_players = number_of_players;
    open_simplex_noise(argc > 2 ? atoi(argv[2]) : rand(), &ctx.noise_context);
}

int main(int argc, char** argv)
{
    srand(time(NULL));
    
    parse_console_arguments(argc, argv);
    initialize_context();

    // Collecting events and defining the game loop
    SDL_Event event;

    for (;;)
    {
        while (SDL_PollEvent(&event))
        {
            // Closing the game once an exit event has been received
            if (event.type == SDL_QUIT) goto finish_game;

            handle_event(&ctx.game, &event);

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                // Getting the tile that was clicked
                int tile_x, tile_y;
                
                if (!window_to_tile_position(&tile_x, &tile_y, event.button.x, event.button.y))
                    break;

                tile_t* tile = &ctx.tilemap[tile_y][tile_x];

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    process_hex_dropdown(&ctx.build_dropdown, handle_build);
                    process_hex_dropdown(&ctx.train_dropdown, handle_train);

                    handle_click(tile_x, tile_y);
                }
                // Right click handling
                else
                {
                    if (tile->kind == TILE_CITY)
                        activate_dropdown_at(&ctx.game, &ctx.train_dropdown, event.button.x, event.button.y);
                    
                    else if (tile->kind == TILE_GRASS)
                        activate_dropdown_at(&ctx.game, &ctx.build_dropdown, event.button.x, event.button.y);
                }
            }

            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    // If the space key is pressed, skip turn
                    case SDLK_SPACE:
                        clear_selected_soldiers();
                        next_turn();

                        break;
                }
            }
       }

        SDL_SetRenderDrawColor(ctx.game.renderer, 40, 40, 70, 255);
        SDL_RenderClear(ctx.game.renderer);

        // Rendering the tilemap
        MAP_FOREACH(x, y)
        {
            tile_t* tile = &ctx.tilemap[y][x];

            if (tile->kind != TILE_WATER)
            {
                SDL_RenderCopy(ctx.game.renderer, ctx.tilemap_texture, &tile->source_rect, &tile->dest_rect);

                // Render the border if its conquered on top of the tile
                if (tile->owner_id >= 0 && tile->kind != TILE_FISH)
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
                render_soldiers(tile->soldiers, ctx.game.renderer, ctx.soldiers_texture);
        }

        // Drawing preview tiles
        for (int i = 0; i < TOTAL_HIGHLIGHTED; i++)
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
        for (int i = 0; i < ctx.starting_players; i++)
            render_sprite(&ctx.city_labels[i].sprite, ctx.game.renderer);

        if (ctx.explosion.is_active)
            render_animated_sprite(&ctx.explosion, ctx.game.renderer);

        render_sprite(&ctx.turn_arrow, ctx.game.renderer);

        // Rendering the UI
        SDL_SetRenderDrawColor(ctx.game.renderer, panel_color.r, panel_color.g, panel_color.b, 255);
        SDL_RenderFillRect(ctx.game.renderer, &ctx.panel_rect);

        render_sprite(&ctx.player_name.sprite, ctx.game.renderer);
        render_sprite(&ctx.player_profile, ctx.game.renderer);
        render_sprite(&ctx.player_description.sprite, ctx.game.renderer);
        render_sprite(&ctx.player_territories.sprite, ctx.game.renderer);
        render_sprite(&ctx.player_coins.sprite, ctx.game.renderer);
        render_sprite(&ctx.player_income.sprite, ctx.game.renderer);
        render_sprite(&ctx.player_moves.sprite, ctx.game.renderer);

        finish_game_rendering(&ctx.game);
    }

finish_game:
    free_game(&ctx.game);
}
