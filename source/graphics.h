#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "main.h"
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>


void draw_game(game_data_t* gd);
void graphics_init(game_data_t* gd);
void init_tiles(game_data_t* gd);
void init_particles(game_data_t* gd);
void init_screen_quad(game_data_t* gd);
void init_framebuffer(game_data_t* gd);



#endif