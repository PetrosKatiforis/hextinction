#include "sprite.h"

// Transform methods
void set_transform_position(transform_t* transform, float x, float y)
{
    set_transform_x(transform, x);
    set_transform_y(transform, y);
}

// The reason I'm doing this is for more precision. 1.5 + 1.5 pixels should equal to 3 pixels of movement
void set_transform_x(transform_t* transform, float x)
{
    transform->position.x = x;
    transform->rect.x = ((int) x) - transform->origin[0];
}

void set_transform_y(transform_t* transform, float y)
{
    transform->position.y = y;
    transform->rect.y = ((int) y) - transform->origin[1];
}

void center_origin(transform_t* transform)
{
    transform->origin[0] = transform->rect.w / 2;
    transform->origin[1] = transform->rect.h / 2;
}

// TODO: Make scale a struct property so it can be set multiple times without destroying the original data
void set_transform_scale(transform_t* transform, float scale)
{
    transform->rect.w *= scale;
    transform->rect.h *= scale;
}


// Sprite-specific methods
void create_sprite(sprite_t* sprite, SDL_Texture* texture)
{
    sprite->texture = texture;

    // Making sure the origin is set to zero
    sprite->transform.origin[0] = 0;
    sprite->transform.origin[1] = 0;

    sprite->source_rect.x = sprite->source_rect.y = 0;

    // Fetching the texture's dimensions and saving them in the sprite's source rect 
    SDL_QueryTexture(texture, NULL, NULL, &sprite->source_rect.w, &sprite->source_rect.h);

    sprite->transform.rect.w = sprite->source_rect.w;
    sprite->transform.rect.h = sprite->source_rect.h;
}

void render_sprite(sprite_t* sprite, SDL_Renderer* renderer)
{
    SDL_RenderCopy(renderer, sprite->texture, &sprite->source_rect, &sprite->transform.rect);
}

// (Animated sprite)-specific methods
void create_animated_sprite(animated_sprite_t* anim_sprite, SDL_Texture* texture, unsigned int total_frames, unsigned int frame_duration_ms)
{
    create_sprite(&anim_sprite->sprite, texture);

    anim_sprite->is_active = false;
    anim_sprite->total_frames = total_frames;
    anim_sprite->frame_duration_ms = frame_duration_ms;
    anim_sprite->frame_width = anim_sprite->sprite.transform.rect.w / total_frames;

    // Fixing the rect's width so it doesn't count the whole texture
    anim_sprite->sprite.source_rect.w = anim_sprite->sprite.transform.rect.w = anim_sprite->frame_width;
}

void play_animated_sprite(animated_sprite_t* anim_sprite)
{
    start_timer(&anim_sprite->anim_timer);
    anim_sprite->is_active = true;
}

void render_animated_sprite(animated_sprite_t* anim_sprite, SDL_Renderer* renderer)
{
    unsigned int current_frame = 0;

    if (anim_sprite->is_active)
    {
        unsigned int elapsed_time = SDL_GetTicks() - anim_sprite->anim_timer.starting_time;
        current_frame = elapsed_time / anim_sprite->frame_duration_ms;

        // Check if the animation has ended
        if (current_frame == anim_sprite->total_frames - 1)
        {
            anim_sprite->is_active = false;
            current_frame = 0;

            return;
        }
    }

    anim_sprite->sprite.source_rect.x = anim_sprite->frame_width * current_frame;
    render_sprite(&anim_sprite->sprite, renderer);
}



// Text-specific methods
void create_label(label_t* label, TTF_Font* font)
{
    label->font = font;
    label->color = (SDL_Color) {255, 255, 255, 255};
}

void destroy_label(label_t* label)
{
    SDL_DestroyTexture(label->sprite.texture);
}

void update_label_texture(label_t* label, SDL_Renderer* renderer)
{
    SDL_DestroyTexture(label->sprite.texture);

    SDL_Surface* label_surface = TTF_RenderText_Solid(label->font, label->content, label->color);

    create_sprite(&label->sprite, SDL_CreateTextureFromSurface(renderer, label_surface));

    SDL_FreeSurface(label_surface);
}

// Warning: the user must reset the origin if they want the text to be centered again, because the dimensions will change!
void set_label_content(label_t* label, SDL_Renderer* renderer, const char* content)
{
    strcpy(label->content, content);
    update_label_texture(label, renderer);
}

void set_label_color(label_t* label, SDL_Renderer* renderer, SDL_Color color)
{
    label->color = color;
    update_label_texture(label, renderer);
}

