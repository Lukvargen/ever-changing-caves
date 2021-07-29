#include "graphics.h"
#include "ui.h"
#include "../data/tile_data.c"
#include "../data/screen_quad_data.c"
#include "../data/particle_data.c"
#include "../data/entity_data.c"

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>


gs_asset_texture_t circle_16px; // not the best way...
gs_asset_texture_t turret_png;
gs_asset_texture_t turret_barrel_png;
gs_asset_texture_t worm_png;

gs_asset_font_t font_large;
gs_asset_font_t font_medium;
gs_asset_font_t font_small;




// Forward Declares
void draw_tiles(game_data_t* gd, gs_command_buffer_t* gcb);
void draw_screen(game_data_t* gd, gs_command_buffer_t* gcb);
void draw_particles(game_data_t* gd, gs_command_buffer_t* gcb);
void draw_entities(game_data_t* gd, gs_command_buffer_t* gcb);
void init_particles(game_data_t* gd);
void init_entities(game_data_t* gd);


void graphics_init(game_data_t* gd)
{
	

    gd->gcb = gs_command_buffer_new();
	gd->gsi = gs_immediate_draw_new();

	//gs_graphics_texture_desc_t desc = {0}
	gs_asset_texture_load_from_file("./assets/Circle16.png", &circle_16px, NULL, true, false);
	gs_asset_texture_load_from_file("./assets/Turret.png", &turret_png, NULL, true, false);
	gs_asset_texture_load_from_file("./assets/TurretBarrel.png", &turret_barrel_png, NULL, true, false);
	gs_asset_texture_load_from_file("./assets/Worm.png", &worm_png, NULL, true, false);
	gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &font_large, 32);
	gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &font_medium, 16);
	gs_asset_font_load_from_file("./assets/fff-forward.regular.ttf", &font_small, 12);

    init_tiles(gd);
    init_screen_quad(gd);
    init_framebuffer(gd);
	init_particles(gd);
	init_entities(gd);
	

}

void draw_game(game_data_t* gd)
{
	gs_command_buffer_t* gcb = &gd->gcb;
	gs_immediate_draw_t* gsi = &gd->gsi;

	

	gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05f, 0.05f, 1.f}}
	};

	gs_graphics_clear_desc_t fb_clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.8f, 0.1f, 0.1f, 1.f}}
	};

	gs_vec2 ws = window_size();
	gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

	float diff_x = ws.x / RESOLUTION_X;
	float diff_y = ws.y / RESOLUTION_Y;
	gd->projection = gs_mat4_ortho(0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100); // hur stor del av världen som ska tas med
	gsi_ortho(gsi, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100);


	

	
	// Hpbar
	int hp_bar_end = TILE_SIZE* 20;
	int hp_bar_size = TILE_SIZE * 4;
	{
		float l = TILE_SIZE;
		float b = hp_bar_size;
		float r = hp_bar_end;
		float t = TILE_SIZE;
		gsi_rect(gsi, l, b, r, t,
		0, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		float padding = 2;
		l += padding;
		b -= padding;
		r -= padding;
		t += padding;
		gsi_rect(gsi, l, b, r, t,
		200, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		float percent = gd->player.hp / (gd->player.max_hp*1.0f);
		if (percent < 0)
			percent = 0.f;
		r = l + (r-l) * percent;
		gsi_rect(gsi, l, b, r, t,
		0, 200, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		gsi_rect(gsi, l, b, r, t,
		150, 200, 150, 255, GS_GRAPHICS_PRIMITIVE_LINES);
	}
	char str1[100] = "CRYSTALS: ";
	int length = snprintf(NULL, 0, "%d", gd->crystals_currency);
	char* crystals_str = malloc(length+1);
	snprintf(crystals_str, length+1, "%d", gd->crystals_currency);
	strcat(str1, crystals_str);
	gsi_text(gsi, 16, hp_bar_size + 12, str1, &font_medium, false, 255,255,255,255);
	free(crystals_str);

	char wave_str[100] = "\n";
	snprintf(wave_str, 100, "WAVE:%i", gd->wave);
	gsi_text(gsi, RESOLUTION_X-80, 20, wave_str, &font_medium, false, 255, 255, 255, 255);
	
	gsi_defaults(gsi);

	
	
	
	
	if (gd->shop.visible) {
		int center_x = RESOLUTION_X / 2;
		int center_y = RESOLUTION_Y / 2;
		int upgrade_panel_size_x = RESOLUTION_X/4;
		int upgrade_panel_size_y = 25;
		int upgrade_btn_size_x = 50;
		int upgrade_btn_size_y = 25;
		//int upgrade_panel_x = center_x
		
		// gsi_text(gsi, 50, 300, "some\ntext", &font_small, false, 200, 200, 200, 255);
		gs_vec2 panel_pos = gs_v2(upgrade_panel_size_x / 2 - 10, 150);
		for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
			#define FONT_HEIGHT 14
			#define TEXT_SIZE 128
			char text[TEXT_SIZE];
			text[0] = '\0';
			upgrade_t* upgrade = &gd->shop.upgrades_available[i];
			bool visible = true;
			
 
			
			switch(upgrade->type) {
				case (UPGRADE_TYPE_DMG):
					gs_snprintf(text, TEXT_SIZE, "DMG+%i\n(%i->%i)", upgrade->ivalue, gd->player.dmg, gd->player.dmg + upgrade->ivalue);
					break;
				case (UPGRADE_TYPE_HP):
					gs_snprintf(text, TEXT_SIZE, "HP+%i\n(%i->%i)", upgrade->ivalue, gd->player.max_hp, gd->player.max_hp + upgrade->ivalue);
					break;
				case (UPGRADE_TYPE_LIFETIME):
					gs_snprintf(text, TEXT_SIZE, "P LIFETIME+%.1f\n(%.1f->%.1f)", upgrade->fvalue, gd->player.player_projectile_lifetime, gd->player.player_projectile_lifetime + upgrade->fvalue);
					break;
				case (UPGRADE_TYPE_SPEED):
					gs_snprintf(text, TEXT_SIZE, "P SPEED+%.0f\n(%.0f->%.0f)", upgrade->fvalue, gd->player.player_projectile_speed, gd->player.player_projectile_speed + upgrade->fvalue);
					break;
				case (UPGRADE_TYPE_ACCELL):
					gs_snprintf(text, TEXT_SIZE, "P ACCEL+%.0f\n(%.0f->%.0f)", upgrade->fvalue, gd->player.player_projectile_accel, gd->player.player_projectile_accel + upgrade->fvalue);
					break;
				case (UPGRADE_TYPE_EXPLODE):
					gs_snprintf(text, TEXT_SIZE, "P EXPLODE+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_explosion_radius, gd->player.player_explosion_radius + upgrade->ivalue);
					break;
				case (UPGRADE_TYPE_SHOOT_DELAY):
					gs_snprintf(text, TEXT_SIZE, "SHOOT DELAY-%.2f\n(%.2f->%.2f)", upgrade->fvalue, gd->player.player_shoot_delay, gd->player.player_shoot_delay - upgrade->fvalue);
					break;
				case (UPGRADE_TYPE_SHOOT_REFLECT):
					gs_snprintf(text, TEXT_SIZE, "SHOT REFLECT+%.0f %%\n(%.0f%%->%.0f%%)", upgrade->fvalue*100, gd->player.player_projectile_reflect_chance*100, 100*(gd->player.player_projectile_reflect_chance - upgrade->fvalue));
					break;
				case (UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT):
					gs_snprintf(text, TEXT_SIZE, "SHOT REFLECT AMOUNT+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_projectile_reflect_amount, gd->player.player_projectile_reflect_amount + upgrade->ivalue);
					break;
				case (UPGRADE_TYPE_LASER):
					gs_snprintf(text, TEXT_SIZE, "LASER TARGETS+%i \n(%i->%i)", upgrade->ivalue, gd->player.player_laser_lvl, gd->player.player_laser_lvl + upgrade->ivalue);
					break;
				case (UPGRADE_TYPE_NULL):
					visible = false;
					break;
				default:
					gs_snprintf(text, TEXT_SIZE, "ERROR");
					gs_println("NO UPGRADE TYPE");
					break;


			}
			

			//gs_println("cost: %d", upgrade->cost);
			gs_snprintfc(cost_text, TEXT_SIZE, "\n \nCost %i", upgrade->cost);
			strcat(text, cost_text);

			gsi_ortho(gsi, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100);
			ui_control_t* panel = ui_panel(gsi, &(ui_control_t){
				.visible = visible,
				.font = font_small,
				.color = gs_color(20, 20, 20, 255),
				.pos = panel_pos,
				.center_x = false,
				.padding = gs_v2(5,5),
				.text = text,
				.font_height = FONT_HEIGHT,
				.border = true,
				.border_color = gs_color(200, 200, 200, 255),
				.size = gs_v2(RESOLUTION_X/4, 0)
				//.size = gs_v2(100, 100)
			});
			panel_pos.x += panel->size.x+10;
			gs_vec2 button_pos;
			button_pos.x = panel->pos.x + panel->size.x / 2;
			button_pos.y = panel->pos.y + panel->size.y;
			gs_color_t btn_color = gs_color(60, 20, 20, 255);
			if (gd->crystals_currency >= upgrade->cost) {
				btn_color = gs_color(20, 60, 20, 255);
			};

			if (ui_button(gsi, &(ui_control_t){
				.visible = visible,
				.font = font_small,
				.color = btn_color,
				.pos = button_pos,
				.padding = gs_v2(5,5),
				.text = "BUY",
				.font_height = FONT_HEIGHT,
				.border = true,
				.center_x = true,
				.border_color = gs_color(200, 200, 200, 255),
			})) {
				printf("pressed button\n");

				if (gd->crystals_currency >= upgrade->cost) {
					gd->crystals_currency -= upgrade->cost;
					gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
				} else {
					gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
					continue;
				}

				switch(upgrade->type) {
					case (UPGRADE_TYPE_DMG):
						gd->player.dmg += upgrade->ivalue;
						break;
					case (UPGRADE_TYPE_HP):
						gd->player.max_hp += upgrade->ivalue;
						gd->player.hp += upgrade->ivalue;
						break;
					case (UPGRADE_TYPE_LIFETIME):
						gd->player.player_projectile_lifetime += upgrade->fvalue;
						break;
					case (UPGRADE_TYPE_SPEED):
						gd->player.player_projectile_speed += upgrade->fvalue;
						break;
					case (UPGRADE_TYPE_ACCELL):
						gd->player.player_projectile_accel += upgrade->fvalue;
						break;
					case (UPGRADE_TYPE_EXPLODE):
						gd->player.player_explosion_radius += upgrade->ivalue;
						break;
					case (UPGRADE_TYPE_SHOOT_DELAY):
						gd->player.player_shoot_delay -= upgrade->fvalue;
						break;
					case (UPGRADE_TYPE_SHOOT_REFLECT):
						gd->player.player_projectile_reflect_chance += upgrade->fvalue;
						break;
					case (UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT):
						gd->player.player_projectile_reflect_amount += upgrade->ivalue;
						break;
					case (UPGRADE_TYPE_LASER):
						gd->player.player_laser_lvl += upgrade->ivalue;
						break;
					default:
						break;
				}

				upgrade->type = UPGRADE_TYPE_NULL;
			}
		}
		int heal_cost = 10;
		gs_color_t btn_color = gs_color(60, 20, 20, 255);
		if (gd->crystals_currency >= heal_cost) {
			btn_color = gs_color(20, 60, 20, 255);
		};
		gs_snprintfc(heal_text, TEXT_SIZE, "HEAL\nCOST %i", heal_cost);
		if (ui_button(gsi, &(ui_control_t){
			.visible = true,
			.text = heal_text,
			.pos = gs_v2(RESOLUTION_X/2 - 50, 100),
			.size = gs_v2(100, 0),
			.padding = gs_v2(5,5),
			.font_height = FONT_HEIGHT,
			.font = font_small,
			.color = btn_color,
			.border = true,
			.center_x = true,
			.border_color = gs_color(200, 200, 200, 255)
		})) {
			if (gd->crystals_currency >= heal_cost) {
				gd->crystals_currency -= heal_cost;
				gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
				printf("Heal!\n");
				gd->player.hp = gd->player.max_hp;
			} else {
				gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
				
			}
		}
		int reroll_cost = 5 * pow(1.1, gd->wave-1);
		btn_color = gs_color(60, 20, 20, 255);
		if (gd->crystals_currency >= reroll_cost) {
			btn_color = gs_color(20, 60, 20, 255);
		};
		gs_snprintfc(reroll_text, TEXT_SIZE, "REROLL\nCOST %i", reroll_cost);
		if (ui_button(gsi, &(ui_control_t){
			.visible = true,
			.text = reroll_text,
			.pos = gs_v2(RESOLUTION_X/2 + 50 + 15, 100),
			.size = gs_v2(100, 0),
			.padding = gs_v2(5,5),
			.font_height = FONT_HEIGHT,
			.font = font_small,
			.color = btn_color,
			.border = true,
			.center_x = true,
			.border_color = gs_color(200, 200, 200, 255)
		})) {
			if (gd->crystals_currency >= reroll_cost) {
				gd->crystals_currency -= reroll_cost;
				gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
				get_available_upgrades(gd);
			} else {
				gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
				
			}
			
		}

		if (ui_button(gsi, &(ui_control_t){
			.visible = true,
			.text = "NEXT WAVE",
			.pos = gs_v2(RESOLUTION_X/2, 250),
			.padding = gs_v2(5,5),
			.font_height = FONT_HEIGHT,
			.font = font_small,
			.color = gs_color(30, 30, 30, 255),
			.border = true,
			.center_x = true,
			.border_color = gs_color(200, 200, 200, 255)
		})) {
			printf("Next Wave!\n");
			gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
			shop_hide(gd);
		}
	}


	//gsi_blend_enabled(gsi, true);
	for (int i = 0; i < gs_dyn_array_size(gd->lasers); i++) {
		laser_t* laser = &gd->lasers[i];
		gs_color_t color = laser->color;
		color.a = 255*(1-(laser->time_alive / laser->max_time_alive));
		for (int j = 0; j < gs_dyn_array_size(laser->points) -1; j++) {
			gs_vec2 p1 = laser->points[j];
			gs_vec2 p2 = laser->points[j+1];
			gsi_linev(gsi, p1, p2, color);
		}
	}

	
	if (gd->game_over) {
		gs_vec2 dims = gs_asset_font_get_text_dimensions(&font_large, "GAME OVER");
		int y = (RESOLUTION_Y - dims.y) / 2;
		gsi_text(gsi, (RESOLUTION_X - dims.x) / 2, y, "GAME OVER", &font_large, false, 255, 255, 255, 255);
		gsi_defaults(gsi);
		if (ui_button(gsi, &(ui_control_t){
			.visible = true,
			.text = "RESTART",
			.pos = gs_v2(RESOLUTION_X/2, y + dims.y +1 ),
			.padding = gs_v2(5,5),
			.font_height = FONT_HEIGHT,
			.font = font_medium,
			.color = gs_color(30, 30, 30, 255),
			.border = true,
			.center_x = true,
			.border_color = gs_color(200, 200, 200, 255)
		})) {
			printf("RESTART!\n");
			gd->restart = true;
		}
	}

	//gsi_line(gsi, 20, 20, 200, 200, 200, 200, 50, 255);

	// render to frame buffer
	gs_graphics_begin_render_pass(gcb, gd->rp);
		gs_graphics_set_viewport(gcb, 0, 0, RESOLUTION_X, RESOLUTION_Y); // fit the texture
		gs_graphics_clear(gcb, &fb_clear);
		draw_tiles(gd, gcb);
		draw_particles(gd, gcb);
		draw_entities(gd, gcb);
		gsi_draw(gsi, gcb);
	gs_graphics_end_render_pass(gcb);
	

	// render to backbuffer
	gs_graphics_begin_render_pass(gcb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
		gs_graphics_set_viewport(gcb, 0, 0, ws.x, ws.y); // where to draw
		gs_graphics_clear(gcb, &clear);
		// draws quad with frame buffer image
		draw_screen(gd, gcb);
		//gsi_draw(gsi, gcb);
	gs_graphics_end_render_pass(gcb);

	
	// graphic backend command buffer submit
	gs_graphics_submit_command_buffer(gcb);
}

void init_tiles(game_data_t* gd)
{
    // Tiles
	gd->tile_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = tile_v_data,
			.size = sizeof(tile_v_data)
		}
	);
	// vertex_buffer / index_buffer request update
	// skulle kunna sätta ihop hörnen och lägga till color attribute som en mix av topleft topright bottomleft bottomright
	// blir det ens snyggt med gradient?
	// lika bra att göra ändå för att få det till en drawcall...

	// attrib pos 1 = 16
	// model scale 16 transate 0
	// måla topleft 0,0 scale 16
	
	gd->tile_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = tile_i_data,
			.size = sizeof(tile_i_data)
		}
	);
	//gs_graphics_index_buffer_create
	gd->tile_shader = gs_graphics_shader_create(
		&(gs_graphics_shader_desc_t) {
			.sources = (gs_graphics_shader_source_desc_t[]) {
				{.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = tile_v_src},
				{.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = tile_f_src},
			},
			.size = 2 * sizeof(gs_graphics_shader_source_desc_t),
			.name = "tile_shader"
		}
	);

	gd->u_tile_color = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.name = "u_color",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC3}
		}
	);
	gd->u_tile_model = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.name = "u_model",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
		}
	);
	gd->u_tile_projection = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.name = "u_projection",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
		}
	);
	
	gd->tile_pip = gs_graphics_pipeline_create(
		&(gs_graphics_pipeline_desc_t) {
			.raster = {
				.shader = gd->tile_shader
			},
			.blend = { // hur den ska hantera alpha?
				.func = GS_GRAPHICS_BLEND_EQUATION_ADD,
				.src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
				.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA
			},
			.layout = {
				.attrs = (gs_graphics_vertex_attribute_desc_t[]) {
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"}
				},
				.size = 1 * sizeof(gs_graphics_vertex_attribute_desc_t)
			}
		}
	);


}

void init_screen_quad(game_data_t* gd)
{
/*
"uniform bool u_shake;\n"
"uniform float u_time;\n"
*/

	gd->u_screen_shake = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
			.name = "u_shake",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_INT} // finns inte UNIFORM_BOOL..
		}
	);
	gd->u_screen_time = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
			.name = "u_time",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_FLOAT}
		}
	);

	gd->u_screen_texture = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_tex",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}
		}
	);
	gd->u_screen_res = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_res",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC2}
		}
	);

	gd->screen_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = screen_v_data,
			.size = sizeof(screen_v_data)
		}
	);
	gd->screen_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = screen_i_data,
			.size = sizeof(screen_i_data)
		}
	);

	gd->screen_shader = gs_graphics_shader_create(
		&(gs_graphics_shader_desc_t) {
			.sources = (gs_graphics_shader_source_desc_t[]) {
				{.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = screen_v_src},
				{.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = screen_f_src},
			},
			.size = 2 * sizeof(gs_graphics_shader_source_desc_t),
			.name = "screen_shader"
		}
	);

	gd->screen_pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = gd->screen_shader,
                .index_buffer_element_size = sizeof(uint32_t) 
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"},
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"}
                },
                .size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void init_particles(game_data_t* gd)
{
	gd->u_particle_texture = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_tex",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}
		}
	);
	gd->u_particle_mvp = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
			.name = "u_mvp",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
		}
	);
	gd->u_particle_color = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_color",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC4}
		}
	);

	gd->particle_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = quad_v_data,
			.size = sizeof(quad_v_data)
		}
	);
	gd->particle_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = quad_i_data,
			.size = sizeof(quad_i_data)
		}
	);

	gd->particle_shader = gs_graphics_shader_create(
		&(gs_graphics_shader_desc_t) {
			.sources = (gs_graphics_shader_source_desc_t[]) {
				{.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = particle_v_src},
				{.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = particle_f_src},
			},
			.size = 2 * sizeof(gs_graphics_shader_source_desc_t),
			.name = "particle_shader"
		}
	);

	gd->particle_pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = gd->particle_shader,
                .index_buffer_element_size = sizeof(uint32_t) 
            },
			.blend = {
				.func = GS_GRAPHICS_BLEND_EQUATION_ADD,
				.src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
				.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA
			},
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"},
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"}
                },
                .size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
	
}
void init_entities(game_data_t* gd)
{
	gd->u_entity_texture = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_tex",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}
		}
	);
	gd->u_entity_mvp = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
			.name = "u_mvp",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
		}
	);
	gd->u_entity_color = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_color",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC4}
		}
	);
	gd->u_entity_flash = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_flash",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_FLOAT}
		}
	);


	

	gd->entity_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = quad_v_data,
			.size = sizeof(quad_v_data)
		}
	);
	gd->entity_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = quad_i_data,
			.size = sizeof(quad_i_data)
		}
	);

	gd->entity_shader = gs_graphics_shader_create(
		&(gs_graphics_shader_desc_t) {
			.sources = (gs_graphics_shader_source_desc_t[]) {
				{.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = entity_v_src},
				{.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = entity_f_src},
			},
			.size = 2 * sizeof(gs_graphics_shader_source_desc_t),
			.name = "entity_shader"
		}
	);

	gd->entity_pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = gd->entity_shader,
                .index_buffer_element_size = sizeof(uint32_t) 
            },
			.blend = {
				.func = GS_GRAPHICS_BLEND_EQUATION_ADD,
				.src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
				.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA
			},
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"},
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"}
                },
                .size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void init_framebuffer(game_data_t* gd)

{
    gd->fbo = gs_graphics_framebuffer_create(NULL);
	// construct color render target
	gd->rt = gs_graphics_texture_create(
		&(gs_graphics_texture_desc_t) {
			.width = RESOLUTION_X, // hur ändrar man live? eller gör man bara en stor och sen scalar ner?
			.height = RESOLUTION_Y,
			.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
			.wrap_s = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
			.wrap_t = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
			.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
			.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
			.render_target = true
		}
	);
	// construct render pass
	gd->rp = gs_graphics_render_pass_create(
		&(gs_graphics_render_pass_desc_t) {
			.fbo = gd->fbo,
			.color = &gd->rt, // color buffer array to bind to frame buffer
			.color_size = sizeof(gd->rt)

		}
	);
}

void draw_tiles(game_data_t* gd, gs_command_buffer_t* gcb)
{
	gs_vec3 t_color = gs_v3(1.f, 0.1f, 0.1f);
	gs_mat4 model = gs_mat4_identity();
	model = gs_mat4_scale(TILE_SIZE, TILE_SIZE, 0.0f);
	
	
	// loop through tiles:
	// change uniform
	// apply binds or just use the same struct and update values...
	// draw
	//gs_mat4
	// uniforms
	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_tile_color, .data = &t_color},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_tile_model, .data = &model},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_tile_projection, .data = &gd->projection}
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gd->tile_vbo}},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->tile_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	
	gs_graphics_bind_pipeline(gcb, gd->tile_pip);
	
	gs_vec3 wall_color = {43/255.0, 22/255.0, 60/255.0};//{33/255.0, 11/255.0, 44/255.0};
	gs_vec3 floor_color = {216/255.0, 180/255.0, 226/255.0};
	gs_vec3 col;
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			int x_pos = x * TILE_SIZE;
			int y_pos = y * TILE_SIZE; // coords 0 -> -1 ? projection invert maybe or something
			float tile_value = gd->tiles[x][y].value;
			if (tile_value > TILE_AIR_THRESHOLD) {
				tile_value = TILE_AIR_THRESHOLD / tile_value;
				if (tile_value > 1) tile_value = 1;
				col = wall_color;
			} else col = floor_color;
			//gs_println("col r %f", col.x);
			//t_color = gs_v3((1-tile_value), (1-tile_value), (1-tile_value));
			t_color = gs_vec3_scale(col, tile_value);
			gs_mat4 scale = gs_mat4_scale(TILE_SIZE, TILE_SIZE, 0.0f);
			gs_mat4 translation = gs_mat4_translate(x_pos, y_pos, 0.0f);
			model = gs_mat4_mul(translation, scale);

			gs_graphics_apply_bindings(gcb, &binds); // need apply after change
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		}
	}
}

void draw_particles(game_data_t* gd, gs_command_buffer_t* gcb)
{

	// set texture

	gs_vec4 color;
	gs_mat4 mvp;
	// just set gs_mat4 mvp model view projection uniform 
	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_particle_color, .data = &color},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_particle_mvp, .data = &mvp},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_particle_texture, .data = &circle_16px},
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gd->particle_vbo}},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->particle_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	gs_graphics_bind_pipeline(gcb, gd->particle_pip);

	int emitters_amount = gs_dyn_array_size(gd->particle_emitters);
	for (int i = 0; i < emitters_amount; i++) {
		particle_emitter_t* emitter = gd->particle_emitters[i];
		for (int j = 0; j < emitter->particle_amount; j++) {
			particle_t p = emitter->particles[j];
			if (p.dead)
				continue;
			// draw
			float life_percent = p.life_time / emitter->particle_life_time;
			gs_mat4 scale;
			if (emitter->particle_shrink_out)
				scale = gs_mat4_scale(p.size.x * (1.0-life_percent), p.size.y * (1.0-life_percent), 0.0f);
			else
				scale = gs_mat4_scale(p.size.x, p.size.y, 0.0f);

			gs_mat4 translation = gs_mat4_translate(p.position.x, p.position.y, 0.0f);
			gs_mat4 model = gs_mat4_mul(translation, scale);
			mvp = gs_mat4_mul(gd->projection, model);
			color = p.color;

			//gs_graphics_apply_bindings(gcb, ); // uppdaterar detta texture också?? inte så bra i så fall 
			gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});


		}
		
	}
}

void draw_entities(game_data_t* gd, gs_command_buffer_t* gcb)
{

	gs_vec4 color;
	float flash;
	gs_mat4 mvp;
	gs_asset_texture_t tex = circle_16px;
	// just set gs_mat4 mvp model view projection uniform 
	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_color, .data = &color},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_flash, .data = &flash},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_mvp, .data = &mvp},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_texture, .data = &tex}, // texture borde vara atlas och bara ändras 1 gång
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gd->entity_vbo}},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->entity_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	gs_graphics_bind_pipeline(gcb, gd->entity_pip);

	// Player
	{

		color = gs_v4(50/255.0, 150/255.0, 50/255.0, 1.0);
		flash = gd->player.flash;
		float size = gd->player.radius * 2;
		gs_mat4 translation = gs_mat4_translate(gd->player.position.x, gd->player.position.y, 0.0f);
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		size += 2;
		color = gs_v4(1, 1, 0, 0.2);
		translation = gs_mat4_translate(gd->player.position.x, gd->player.position.y, 0.0f);
		model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

	}

	// Powerups
	for (int i = 0; i < gs_dyn_array_size(gd->powerups); i++) {
		powerup_t *p = gd->powerups[i];

		color = gs_v4(50/255.0, 50/255.0, 200/255.0, 1.0);
		flash = 0;
		gs_mat4 translation = gs_mat4_translate(p->position.x, p->position.y, 0.0f);
		float size = p->radius * 2;
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

	}


	// Worms
	tex = worm_png;
	for (int i = 0; i < gs_dyn_array_size(gd->worms); i++) {
		entity_t* head = gd->worms[i];
		gs_vec2 pos = head->position;

		//color = gs_v4(200/255.0, 20/255.0, 20/255.0, 1.0);
		color = head->color;
		flash = head->flash;
		//printf("flash %f \n", flash);
		gs_mat4 translation = gs_mat4_translate(pos.x, pos.y, 0.0f);
		float size = head->radius * 2;
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);


		float alpha = 1.0;
		float dead_timer = head->dead_timer;
		if (head->dead) {
			alpha = 1 * (1-(dead_timer/0.25));
			alpha = gs_max(alpha, 0);
			
		}
		color.w = alpha;
		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		entity_t* parent = head;
		int i = 0;
		while (parent->worm_segment) {
			entity_t* child = parent->worm_segment;
			pos = child->position;
			alpha = 1 * (1-(gs_max(dead_timer - 0.05*i, 0)/0.25)); 
			alpha = gs_max(alpha, 0);
			
			
			//color = gs_v4(0.6 - 0.04 * i, 20/255.0, 20/255.0, alpha);
			color = head->color;
			color.x -= 0.04 * i;
			color.w = alpha;

			gs_mat4 translation = gs_mat4_translate(pos.x, pos.y, 0.0f);
			size = (child->radius*2);
			gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
			mvp = gs_mat4_mul(gd->projection, model);

			gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

			i++;
			parent = child;
		}


		

	}

	
	// Projectiles
	tex = circle_16px;
	for (int i = 0; i < gs_dyn_array_size(gd->projecitles); i++) {
		projectile_t* p = &gd->projecitles[i];

		color = gs_v4(50/255.0, 50/255.0, 50/255.0, 1.0);
		flash = 0;
		gs_mat4 translation = gs_mat4_translate(p->position.x, p->position.y, 0.0f);
		float size = 2 * p->radius * (1-(p->life_time / p->max_life_time));
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
	}

	// turrets
	tex = turret_png;
	int turret_size = turret_png.desc.width;
	int turret_barrel_size = turret_barrel_png.desc.width;
	int t_size = gs_dyn_array_size(gd->turrets);
	for (int i = 0; i < t_size; i++) {
		entity_t* t = gd->turrets[i];

		color =  t->color;//gs_v4(1.0, 1.0, 1.0, 1.0);
		flash = t->flash;
		tex = turret_png;

		//gs_mat4 rot = gs_mat4_rotate(t->angle, 0, 0, 1);
		gs_mat4 translation = gs_mat4_translate(t->position.x, t->position.y, 0.0f);
		//float size = 16;
		float size = turret_size * gs_min(1.0, t->turret_time_since_spawn / TURRET_ANIMATION_SPAWN_TIME);
		gs_mat4 model = gs_mat4_mul_list(2, translation, gs_mat4_scale(size, size, 0));

		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		// barrel
		tex = turret_barrel_png;
		gs_mat4 rot = gs_mat4_rotate(t->turret_angle, 0, 0, 1);

		float time_since_shot = t->turret_shoot_time;
		float offset_power = (1- gs_min(time_since_shot/TURRET_BURST_SHOT_DELAY, 1.f));
		float x_offset = cos(t->turret_angle) * -5 * offset_power;
		float y_offset = sin(t->turret_angle) * -5 * offset_power;
		
		
		translation = gs_mat4_translate(t->position.x + x_offset, t->position.y + y_offset, 0.0f);
		size = turret_barrel_size * gs_min(1.0, t->turret_time_since_spawn / TURRET_ANIMATION_SPAWN_TIME);
		model = gs_mat4_mul_list(3, translation, gs_mat4_scale(size, size, 0), rot);

		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
	}

	// orbs
	int o_size = gs_dyn_array_size(gd->orbs);
	for (int i = 0; i < o_size; i++) {
		entity_t* o = gd->orbs[i];

		color = o->color;
		flash = o->flash;
		tex = circle_16px;

		//gs_mat4 rot = gs_mat4_rotate(t->angle, 0, 0, 1);
		gs_mat4 translation = gs_mat4_translate(o->position.x, o->position.y, 0.0f);
		//float size = 16;
		float size = o->radius;
		gs_mat4 model = gs_mat4_mul_list(2, translation, gs_mat4_scale(size, size, 0));

		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
	}

}


void draw_screen(game_data_t* gd, gs_command_buffer_t* gcb)
{

	gs_vec2 res = {RESOLUTION_X, RESOLUTION_Y};

	float time = gs_platform_elapsed_time() * 0.001;
	int shake = 0;
	if (gd->shake_time > 0)
		shake = 1;

	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_screen_shake, .data = &shake},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_screen_time, .data = &time},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_screen_texture, .data = &gd->rt},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_screen_res, .data = &res},
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gd->screen_vbo}},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->screen_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	gs_graphics_bind_pipeline(gcb, gd->screen_pip);
	gs_graphics_apply_bindings(gcb, &binds);
	gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

}



