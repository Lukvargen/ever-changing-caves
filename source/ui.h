#ifndef UI_H
#define UI_H

#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include "main.h"


typedef struct ui_control_t
{
    gs_vec2 pos;
    gs_vec2 size;
    gs_vec2 content_pos;
    gs_vec2 content_size;
    gs_vec2 padding;
    char*   text;
    gs_asset_font_t font;
    int font_height;
    gs_color_t color;
    gs_color_t border_color;
    bool border;
    bool center_x;
    bool visible;

} ui_control_t;


void ui_calculate_size(ui_control_t* control)
{
    gs_vec2 text_dims = {0};
    if (control->text != NULL) {

        float longest_width = 0.f;

        char* text_copy = malloc(strlen(control->text)+1);
        strcpy(text_copy, control->text);
        char* token = strtok(text_copy, "\n");
        int i = 0;
        while (token != NULL) {
            gs_vec2 dims = gs_asset_font_get_text_dimensions(&control->font, token);
            if (dims.x > longest_width) {
                longest_width = dims.x;
            }
            token = strtok(NULL, "\n");
            i++;
        
       }
        free(text_copy);
        text_dims.x = longest_width;
        text_dims.y = control->font_height * i;

    }
    
    gs_vec2 total_size;
    total_size.x = gs_max(control->size.x, text_dims.x);
    total_size.y = gs_max(control->size.y, text_dims.y);
    
    if (control->center_x) {
        control->pos.x = control->pos.x - total_size.x * 0.5;
        control->pos.x = gs_round(control->pos.x);
    }
    control->content_pos = control->pos;
    control->content_size = total_size;

    total_size = gs_vec2_add(total_size, gs_vec2_scale(control->padding, 2));
    control->size = total_size;
    control->pos = gs_vec2_sub(control->pos, control->padding);
    control->pos.x = (int)control->pos.x;
    control->pos.y = (int)control->pos.y;
    control->size.x = (int)control->size.x;
    control->size.y = (int)control->size.y;

}

ui_control_t* ui_panel(gs_immediate_draw_t* gsi, ui_control_t* control)
{
    ui_calculate_size(control);
    if (!control->visible)
        return control;
    gsi_defaults(gsi);
    
    if (control->border)
        gsi_rectv(gsi, gs_vec2_sub(control->pos, gs_v2(2,2)), gs_vec2_add(control->pos, gs_vec2_add(control->size, gs_v2(2,2))), control->border_color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_rectv(gsi, control->pos, gs_vec2_add(control->pos, control->size), control->color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);



    
    
    int i = 0;
    char* text_copy = malloc(strlen(control->text)+1);
    strcpy(text_copy, control->text);
    char* token = strtok(text_copy, "\n");
    while (token != NULL) {
        gsi_text(gsi, control->content_pos.x, control->content_pos.y + control->font_height * (1+i), token, &control->font, false, 200, 200, 200, 255);
        token = strtok(NULL, "\n");
        i++;
        
    }
    free(text_copy);

    gsi_defaults(gsi);
    return control;
}

bool ui_button(gs_immediate_draw_t* gsi, gs_vec2 m_pos, ui_control_t* control)
{
    
    ui_calculate_size(control);
    if (!control->visible)
        return false;

    gs_color_t color = control->color;
    bool pressed = false;
    if (m_pos.x >= control->pos.x && m_pos.x < control->pos.x + control->size.x &&
        m_pos.y >= control->pos.y && m_pos.y < control->pos.y + control->size.y) {
            float brightness = 1.5;
            if (gs_platform_mouse_pressed(GS_MOUSE_LBUTTON)) {
                pressed = true;
                brightness = 0.5;
            }
            color.r = gs_clamp(color.r*brightness,0, 255);
            color.g = gs_clamp(color.g*brightness,0, 255);
            color.b = gs_clamp(color.b*brightness,0, 255);
        }
    

    if (control->border)
        gsi_rectv(gsi, gs_vec2_sub(control->pos, gs_v2(2,2)), gs_vec2_add(control->pos, gs_vec2_add(control->size, gs_v2(2,2))), control->border_color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_rectv(gsi, control->pos, gs_vec2_add(control->pos, control->size), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
     

    int i = 0;
    char* text_copy = malloc(strlen(control->text)+1);
    strcpy(text_copy, control->text);
    char* token = strtok(text_copy, "\n");
    while (token != NULL) {
        gsi_text(gsi, control->content_pos.x, control->content_pos.y + control->font_height * (1+i), token, &control->font, false, 200, 200, 200, 255);
        token = strtok(NULL, "\n");
        i++;
        
    }
    free(text_copy);

    gsi_defaults(gsi);
    

   return pressed;
}


#endif