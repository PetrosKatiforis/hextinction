#include "interface.h"
#include "utils.h"

// Text-specific methods
// TODO: Probably create your own version of this because from what I've heard, SDL's way is slow
// (why am I talking to myself using "you"?)
void create_label(label_t* label, TTF_Font* font, unsigned int wrap_length)
{
    label->font = font;
    label->content = NULL;

    label->color = (SDL_Color) {255, 255, 255, 255};
    label->wrap_length = wrap_length;
}

void destroy_label(label_t* label)
{
    SDL_DestroyTexture(label->sprite.texture);
}

void update_label_texture(label_t* label, SDL_Renderer* renderer)
{
    SDL_DestroyTexture(label->sprite.texture);

    SDL_Surface* label_surface = label->wrap_length > 0
        ? TTF_RenderText_Solid_Wrapped(label->font, label->content, label->color, label->wrap_length)
        : TTF_RenderText_Solid(label->font, label->content, label->color);

    create_sprite(&label->sprite, SDL_CreateTextureFromSurface(renderer, label_surface));
    SDL_FreeSurface(label_surface);
}

// Warning: the user must reset the origin if they want the text to be centered again, because the dimensions will change!
void set_label_content(label_t* label, SDL_Renderer* renderer, const char* content)
{
    // I'm not sure if this is the most efficient way of doing it...
    if (label->content)
        free(label->content);

    label->content = strdup(content);

    update_label_texture(label, renderer);
}

void set_label_color(label_t* label, SDL_Renderer* renderer, SDL_Color color)
{
    label->color = color;
    update_label_texture(label, renderer);
}


// Dropdown methods
void create_dropdown(dropdown_t* dropdown, SDL_Renderer* renderer, TTF_Font* font, unsigned int total_items, ...)
{
    dropdown->items = calloc(total_items, sizeof(label_t));
    dropdown->length = total_items;

    va_list contents;
    va_start(contents, total_items);

    // Initializing all labels and calculating max width
    unsigned int max_width = 0;

    for (int i = 0; i < dropdown->length; i++)
    {
        char* content = va_arg(contents, char*);

        create_label(&dropdown->items[i], font, 0);
        set_label_content(&dropdown->items[i], renderer, content);
        
        if (dropdown->items[i].sprite.transform.rect.w > max_width)
            max_width = dropdown->items[i].sprite.transform.rect.w;
    }

    va_end(contents);

    dropdown->height = dropdown->items[0].sprite.transform.rect.h * dropdown->length;
    dropdown->width = max_width;

    // Creating the rects
    dropdown->background = (SDL_Rect) {0, 0, dropdown->width + 2 * DROPDOWN_PADDING, dropdown->height + 2 * DROPDOWN_PADDING};
}

void activate_dropdown(dropdown_t* dropdown, int x, int y)
{
    dropdown->is_active = true;
    dropdown->selected_index = -1;

    dropdown->background.x = x;
    dropdown->background.y = y;

    for (int i = 0; i < dropdown->length; i++)
    {
        // The dropdown assumes that all labels have the same height
        set_transform_position(&dropdown->items[i].sprite.transform,
            DROPDOWN_PADDING + x,
            DROPDOWN_PADDING + y + dropdown->items[0].sprite.transform.rect.h * i
        );
    }
}

int dropdown_get_selected(dropdown_t* dropdown)
{
    dropdown->is_active = false;
    
    // selected_index is set to -1 when no item is selected
    return dropdown->selected_index;
}

void update_dropdown_highlight(dropdown_t* dropdown, int x, int y)
{
    if (!dropdown->is_active) return;

    if (is_point_inside(&dropdown->background, x, y))
    {
        dropdown->selected_index = (y - dropdown->background.y - DROPDOWN_PADDING) / dropdown->items[0].sprite.transform.rect.h;
        
        if (dropdown->selected_index < dropdown->length)
        {
            dropdown->highlight = dropdown->items[dropdown->selected_index].sprite.transform.rect;
            dropdown->highlight.w = dropdown->width;
        }
        else
            dropdown->selected_index = -1;
    }
    else
    {
        dropdown->selected_index = -1;
    }
}

void render_dropdown(dropdown_t* dropdown, SDL_Renderer* renderer)
{
    if (!dropdown->is_active) return;

    SDL_SetRenderDrawColor(renderer, dropdown_background.r, dropdown_background.g, dropdown_background.b, 255);
    SDL_RenderFillRect(renderer, &dropdown->background);

    if (dropdown->selected_index >= 0)
    {
        SDL_SetRenderDrawColor(renderer, dropdown_highlight.r, dropdown_highlight.g, dropdown_highlight.b, 255);
        SDL_RenderFillRect(renderer, &dropdown->highlight);
    }

    for (int i = 0; i < dropdown->length; i++)
    {
        render_sprite(&dropdown->items[i].sprite, renderer);
    }
}


