#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"
#include <gs/gs.h>





/*
int tile_i_data[] = {
    0, 3, 2,
    0, 1, 3
};
*/

/*
float tile_v_data[] = {
    0.0f, 0.0f, // Top Left
    1.0f, 0.0f, // Top Right
    0.0f, 1.0f, // Bottom Left
    1.0f, 1.0f  // Bottom Right
};*/

void draw_game(game_data_t* gd);
void graphics_init(game_data_t* gd);
void init_tiles(game_data_t* gd);
void init_screen_quad(game_data_t* gd);
void init_framebuffer(game_data_t* gd);



#endif