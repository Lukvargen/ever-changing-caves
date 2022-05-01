#include "graphics.h"
#include "ui.h"
#include "../data/tile_data.c"
#include "../data/screen_quad_data.c"
#include "../data/particle_data.c"
#include "../data/entity_data.c"

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

#include <gs/util/gs_idraw.h>

#include <gs/util/gs_gui.h>


//#include "style_data.h"

#include "gui.h"

gs_asset_texture_t circle_16px;
gs_asset_texture_t player_body_png;
gs_asset_texture_t player_hands_png;
gs_asset_texture_t turret_png;
gs_asset_texture_t turret_barrel_png;

gs_asset_texture_t turret_boss_feet_png;
gs_asset_texture_t turret_boss_base_png;
gs_asset_texture_t turret_boss_barrel_png;
gs_asset_texture_t turret_boss_top_png;

gs_asset_texture_t worm_png;
gs_asset_texture_t worm_boss_png;
gs_asset_texture_t worm_boss_head_png;
gs_asset_texture_t worm_boss_tail_png;

gs_asset_texture_t final_boss_png;

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
	

	
	

	gs_graphics_texture_desc_t desc = (gs_graphics_texture_desc_t){
		.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
		.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
		.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
		.wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
		.wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT
	};
	gs_asset_texture_load_from_file("./assets/Circle16.png", &circle_16px, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/PlayerBody.png", &player_body_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/PlayerHands.png", &player_hands_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/Turret.png", &turret_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/TurretBarrel.png", &turret_barrel_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/Worm.png", &worm_png, &desc, true, false);
	
	
	gs_asset_texture_load_from_file("./assets/WormBoss.png", &worm_boss_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/WormBossHead.png", &worm_boss_head_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/WormBossTail.png", &worm_boss_tail_png, &desc, true, false);

	gs_asset_texture_load_from_file("./assets/TurretBossFeet.png", &turret_boss_feet_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/TurretBossBase.png", &turret_boss_base_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/TurretBossBarrel.png", &turret_boss_barrel_png, &desc, true, false);
	gs_asset_texture_load_from_file("./assets/TurretBossTop.png", &turret_boss_top_png, &desc, true, false);

	gs_asset_texture_load_from_file("./assets/FinalBoss.png", &final_boss_png, &desc, true, false);


	gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &font_large, 32);
	gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &font_medium, 16);
	gs_asset_font_load_from_file("./assets/fff-forward.regular.ttf", &font_small, 12);

    init_tiles(gd);
    init_screen_quad(gd);
    init_framebuffer(gd);
	init_particles(gd);
	init_entities(gd);
	
	//g_app.fonts[GUI_FONT_LABEL] = font_medium;
	//g_app.fonts[GUI_FONT_BUTTON] = font_medium;
	//g_app.fonts[GUI_FONT_BUTTONFOCUS] = font_medium;
	//gd->stylesheet = gs_gui_style_sheet_create(&gd->gs_gui, &menu_style_sheet_desc);

	gui_init(gd);

	//gs_vec2 fbs = gs_platform_framebuffer_sizev(gd->fbo.id);
	//gs_println("gs_platform_framebuffer_sizev %f %f", fbs.x, fbs.y);
	
	
	
}

void draw_game(game_data_t* gd)
{
	gs_command_buffer_t* gcb = &gd->gcb;
	gs_immediate_draw_t* gsi = &gd->gsi;
	

	gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05f, 0.05f, 1.f}}
	};

	gs_graphics_clear_desc_t fb_clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.8f, 0.1f, 0.8f, 1.f}}
	};

	gs_vec2 ws = window_size();
	gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

	gs_vec2 p_pos = gd->player.position;
	gs_vec2 half_res = {RESOLUTION_X/2, RESOLUTION_Y/2};
	gd->camera_pos = (gs_vec2){p_pos.x - half_res.x, p_pos.y - half_res.y};
	gs_vec2 mouse_dif = gs_vec2_sub(get_world_mouse_pos(gd), gd->player.position);
	gd->camera_pos.x += mouse_dif.x * 0.1;
	gd->camera_pos.y += mouse_dif.y * 0.1;

	if (gd->camera_pos.x < 0) gd->camera_pos.x = 0;
	if (gd->camera_pos.x + RESOLUTION_X > WORLD_SIZE_X) gd->camera_pos.x = WORLD_SIZE_X - RESOLUTION_X;
	if (gd->camera_pos.y < 0) gd->camera_pos.y = 0;
	if (gd->camera_pos.y + RESOLUTION_Y > WORLD_SIZE_Y) gd->camera_pos.y = WORLD_SIZE_Y - RESOLUTION_Y;

	gd->projection = gs_mat4_ortho(gd->camera_pos.x, RESOLUTION_X + gd->camera_pos.x, RESOLUTION_Y + gd->camera_pos.y, gd->camera_pos.y, -100, 100);
	gsi_ortho(gsi, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100);

	gui_update(gd);
	/*gs_gui_begin(&gd->gs_gui, gs_v2(RESOLUTION_X, RESOLUTION_Y));
	{
		gs_gui_set_style_sheet(&gd->gs_gui, &gd->stylesheet);


		if (gs_gui_window_begin_ex(&gd->gs_gui, "Demo Window", gs_gui_rect(250, 250, 250, 250), NULL, NULL,GS_GUI_OPT_NOTITLE)) {
			
			
			gs_gui_text(&gd->gs_gui, "TEXT");
		}
		gs_gui_window_end(&gd->gs_gui);




		//gs_gui_demo_window(&gd->gs_gui, gs_gui_rect(0, 0, 500, 500), NULL);

		//gs_gui_begin_panel(&gd->gs_gui, "SomePanel");

		//	gs_gui_text(&gd->gs_gui, "TEXT");

		//gs_gui_end_panel(&gd->gs_gui);
	}
	gs_gui_end(&gd->gs_gui);*/


	// Hpbar
	/*
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
		204, 41, 54, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES); //red
		float percent = gd->player.hp / (gd->player.max_hp*1.0f); 
		if (percent < 0)
			percent = 0.f;
		r = l + (r-l) * percent;
		gsi_rect(gsi, l, b, r, t,
		71, 160, 37, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES); //green
		gsi_rect(gsi, l, b, r, t,
		150, 200, 150, 255, GS_GRAPHICS_PRIMITIVE_LINES);
	}*/
	char str1[100] = "CRYSTALS: ";
	int length = snprintf(NULL, 0, "%d", gd->crystals_currency);
	char* crystals_str = malloc(length+1);
	snprintf(crystals_str, length+1, "%d", gd->crystals_currency);
	strcat(str1, crystals_str);
	//gsi_text(gsi, 16, hp_bar_size + 12, str1, &font_medium, false, 255,255,255,255);
	free(crystals_str);

	char wave_str[100] = "\n";
	snprintf(wave_str, 100, "WAVE:%i", gd->wave);
	//gsi_text(gsi, RESOLUTION_X-80, 20, wave_str, &font_medium, false, 255, 255, 255, 255);
	
	gsi_defaults(gsi);
	gs_vec2 m_pos = get_local_mouse_pos();
	/* use gs_ui now
	if (gd->shop.visible) {
		int center_x = RESOLUTION_X / 2;
		int upgrade_panel_size_x = RESOLUTION_X/4;
		
		
		gs_vec2 panel_pos = gs_v2(upgrade_panel_size_x / 2 - 10, 150);
		for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
			#define FONT_HEIGHT 14
			#define TEXT_SIZE 128
			char text[TEXT_SIZE];
			text[0] = '\0';
			upgrade_t* upgrade = &gd->shop.upgrades_available[i];
			bool visible = true;
			
			
			
			if (upgrade->type == UPGRADE_TYPE_NULL)
				visible = false;
			else
				get_upgrade_string(gd, upgrade, text, TEXT_SIZE);

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
			});
			panel_pos.x += panel->size.x+10;
			gs_vec2 button_pos;
			button_pos.x = panel->pos.x + panel->size.x / 2;
			button_pos.y = panel->pos.y + panel->size.y;
			gs_color_t btn_color = gs_color(60, 20, 20, 255);
			if (gd->crystals_currency >= upgrade->cost) {
				btn_color = gs_color(20, 60, 20, 255);
			};

			if (ui_button(gsi, m_pos, &(ui_control_t){
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

				upgrade_purchase(gd, upgrade);
			}
		}
		int heal_cost = 10;
		gs_color_t btn_color = gs_color(60, 20, 20, 255);
		if (gd->crystals_currency >= heal_cost) {
			btn_color = gs_color(20, 60, 20, 255);
		};
		gs_snprintfc(heal_text, TEXT_SIZE, "HEAL\nCOST %i", heal_cost);
		if (ui_button(gsi,m_pos, &(ui_control_t){
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
				gd->player.hp = gd->player.max_hp;
			} else {
				gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
				
			}
		}
		int reroll_cost = 5 * pow(1.1, gd->wave-1);
		if (gd->shop.free_reroll_left > 0)
			reroll_cost = 0;
		btn_color = gs_color(60, 20, 20, 255);
		if (gd->crystals_currency >= reroll_cost) {
			btn_color = gs_color(20, 60, 20, 255);
		};
		gs_snprintfc(reroll_text, TEXT_SIZE, "REROLL\nCOST %i", reroll_cost);
		if (ui_button(gsi,m_pos, &(ui_control_t){
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
				if (gd->shop.free_reroll_left > 0)
					gd->shop.free_reroll_left--;
				gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
				get_available_upgrades(gd);
			} else {
				gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
				
			}
			
		}

		if (ui_button(gsi,m_pos, &(ui_control_t){
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
			gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
			shop_hide(gd);
		}
	}*/


	/*
	if (gd->game_over) {
		char* game_over = "GAME OVER";
		gs_vec2 dims = gs_asset_font_text_dimensions(&font_large, game_over, strlen(game_over));
		int y = (RESOLUTION_Y - dims.y) / 2;
		gsi_text(gsi, (RESOLUTION_X - dims.x) / 2, y, "GAME OVER", &font_large, false, 255, 255, 255, 255);
		gsi_defaults(gsi);
		if (ui_button(gsi,m_pos, &(ui_control_t){
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
			gd->restart = true;
		}
	}
	*/

	// lasers
	gsi_blend_enabled(gsi, true);
	gsi_ortho(gsi, gd->camera_pos.x, RESOLUTION_X + gd->camera_pos.x, RESOLUTION_Y + gd->camera_pos.y, gd->camera_pos.y, 0, 100);
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

	// render to frame buffer
	gs_graphics_renderpass_begin(gcb, gd->rp);
		gs_graphics_set_viewport(gcb, 0, 0, RESOLUTION_X, RESOLUTION_Y); // fit the texture
		gs_graphics_clear(gcb, &fb_clear);
		draw_tiles(gd, gcb);
		draw_particles(gd, gcb);
		draw_entities(gd, gcb);
		gsi_draw(gsi, gcb);

		
		gs_gui_render(&gd->gs_gui, gcb);

	gs_graphics_renderpass_end(gcb);
	

	// render to backbuffer
	gs_graphics_renderpass_begin(gcb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
		gs_graphics_set_viewport(gcb, 0, 0, ws.x, ws.y); // where to draw
		gs_graphics_clear(gcb, &clear);
		// draws quad with frame buffer image
		draw_screen(gd, gcb);


	gs_graphics_renderpass_end(gcb);

	
	// graphic backend command buffer submit
	gs_graphics_command_buffer_submit(gcb);
}

void init_tiles(game_data_t* gd)
{
	// calculate indices
	for (int i = 0; i < TILES_SIZE_X * TILES_SIZE_Y; i++) {
		int i_value = i*4;
		int index = i*6;
		tile_i_data[index+0] = i_value+0;
		tile_i_data[index+1] = i_value+3;
		tile_i_data[index+2] = i_value+2;

		tile_i_data[index+3] = i_value+0;
		tile_i_data[index+4] = i_value+1;
		tile_i_data[index+5] = i_value+3;
	}
	// init position attributes
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			int index = 8*(x + y*TILES_SIZE_X);
			
			tile_v_positions[index+0] = x+0.f;
			tile_v_positions[index+1] = y+0.f;

			tile_v_positions[index+2] = x+1.f;
			tile_v_positions[index+3] = y+0.f;

			tile_v_positions[index+4] = x+0.f;
			tile_v_positions[index+5] = y+1.f;

			tile_v_positions[index+6] = x+1.f;
			tile_v_positions[index+7] = y+1.f;
			
		}
	}

	gd->tile_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = NULL,
			.size = sizeof(tile_v_positions) + sizeof(tile_v_colors),
			.usage = GS_GRAPHICS_BUFFER_USAGE_DYNAMIC // to update the color of the tiles
		}
	);
	gs_graphics_vertex_buffer_request_update(&gd->gcb, gd->tile_vbo, &(gs_graphics_vertex_buffer_desc_t){
		.data = tile_v_positions,
		.size = sizeof(tile_v_positions),
		.update = {
			.offset = 0,
			.type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
		},
		.usage = GS_GRAPHICS_BUFFER_USAGE_DYNAMIC
	});

	gd->tile_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = tile_i_data,
			.size = sizeof(tile_i_data)
		}
	);
	
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
				.shader = gd->tile_shader,
				.index_buffer_element_size = sizeof(uint16_t)
			},
			.layout = {
				.attrs = (gs_graphics_vertex_attribute_desc_t[]) {
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos", .buffer_idx = 0},
					{.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_color", .buffer_idx = 1}
				},
				.size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
			}
		}
	);
}
void draw_tiles(game_data_t* gd, gs_command_buffer_t* gcb)
{
	
	// could split into chunks to only draw and update the ones on screen
	
	gs_mat4 model = gs_mat4_scale(TILE_SIZE, TILE_SIZE, 0.0f);
	
	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_tile_model, .data = &model},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_tile_projection, .data = &gd->projection}
	};
	
	gs_graphics_bind_vertex_buffer_desc_t vbos[] = {
		{.buffer = gd->tile_vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0}, 					  // buffer_idx 0 position
		{.buffer = gd->tile_vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = sizeof(tile_v_positions)} // buffer_idx 1 color
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {.desc = vbos, .size = sizeof(vbos)},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->tile_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	
	// set tile color value
	gs_vec3 wall_color = {43/255.0, 22/255.0, 60/255.0};
	gs_vec3 floor_color = {216/255.0, 180/255.0, 226/255.0};
	gs_vec3 col;
	for (int y = 0; y < TILES_SIZE_Y; y++) {
		for (int x = 0; x < TILES_SIZE_X; x++) {
			int x_pos = x * TILE_SIZE;
			int y_pos = y * TILE_SIZE;
			float tile_value = gd->tiles[x][y].value;
			if (tile_value > TILE_AIR_THRESHOLD) {
				tile_value = TILE_AIR_THRESHOLD / tile_value;
				if (tile_value > 1) tile_value = 1;
				col = wall_color;
			} else col = floor_color;

			if (gd->tiles[x][y].lava_value > 0) {
				col = gs_vec3_scale((gs_vec3){0.8, 0.3, 0.2}, gd->tiles[x][y].lava_value);
			}
			
			gs_vec3 t_color = gs_vec3_scale(col, tile_value);

			int index = 4*3*(x + y*TILES_SIZE_X);
			for (int i = 0; i < 4; i++) {
				tile_v_colors[index+i*3+0] = t_color.x;
				tile_v_colors[index+i*3+1] = t_color.y;
				tile_v_colors[index+i*3+2] = t_color.z;
			}
		}
	}
	
	// update the color attributes
	gs_graphics_vertex_buffer_request_update(gcb, gd->tile_vbo, &(gs_graphics_vertex_buffer_desc_t){
		.data = tile_v_colors,
		.size = sizeof(tile_v_colors),
		.usage = GS_GRAPHICS_BUFFER_USAGE_DYNAMIC,
		.update = {
			.type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA,
			.offset = sizeof(tile_v_positions)
		},
	});
	
	gs_graphics_pipeline_bind(gcb, gd->tile_pip);
	gs_graphics_apply_bindings(gcb, &binds);
	gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = TILES_SIZE_X*TILES_SIZE_Y*6});
}

void init_screen_quad(game_data_t* gd)
{
	gd->u_screen_shake = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
			.name = "u_shake",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_INT} // true / false
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
			.depth = {
				.func = GS_GRAPHICS_DEPTH_FUNC_LESS
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
			.width = RESOLUTION_X,
			.height = RESOLUTION_Y,
			.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
			.wrap_s = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
			.wrap_t = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
			.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
			.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
			//.render_target = true
		}
	);

	// construct render pass
	gd->rp = gs_graphics_renderpass_create(
		&(gs_graphics_renderpass_desc_t) {
			.fbo = gd->fbo,
			.color = &gd->rt, // color buffer array to bind to frame buffer
			.color_size = sizeof(gd->rt)
		}
	);
}



void draw_particles(game_data_t* gd, gs_command_buffer_t* gcb)
{
	gs_vec4 color;
	gs_mat4 mvp;
	
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
	gs_graphics_pipeline_bind(gcb, gd->particle_pip);

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

			
			gs_graphics_apply_bindings(gcb, &binds);
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});


		}
		
	}
}

void draw_entities(game_data_t* gd, gs_command_buffer_t* gcb)
{

	/*
		Should probably have done some sprite component thing | Only worth to change if I decide to add more entities
		sprite_t {
			texture
			pos
			rot
			scale
		}
		and use for entity_list in entities
			for each entity in entity_list
				-set up flash
				color = entity.color
				then can still use switch:

				default:
					draw sprite()
	*/

	gs_vec4 color;
	float flash;
	gs_mat4 mvp;
	gs_asset_texture_t tex = circle_16px;
	// having the entities texture on a atlas would probably be better...
	gs_graphics_bind_uniform_desc_t uniforms[] = {
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_color, .data = &color},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_flash, .data = &flash},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_mvp, .data = &mvp},
		(gs_graphics_bind_uniform_desc_t){.uniform = gd->u_entity_texture, .data = &tex},
	};

	gs_graphics_bind_desc_t binds = {
		.vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gd->entity_vbo}},
		.index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gd->entity_ibo}},
		.uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
	};
	gs_graphics_pipeline_bind(gcb, gd->entity_pip);

	// Projectiles
	tex = circle_16px;
	for (int i = 0; i < gs_dyn_array_size(gd->projecitles); i++) {
		projectile_t* p = &gd->projecitles[i];

		color = p->color;
		flash = 0;
		gs_mat4 translation = gs_mat4_translate(p->position.x, p->position.y, 0.0f);
		float size = 2 * p->radius * (1-(p->life_time / p->max_life_time));
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
	}

	// Player
	{
		tex = player_hands_png;
		//color = gs_v4(50/255.0, 150/255.0, 50/255.0, 1.0);
		flash = gd->player.flash;
		float size = 14;

		color = gs_v4(1, 1, 1, 1);
		gs_mat4 translation = gs_mat4_translate(gd->player.position.x, gd->player.position.y, 0.0f);
		gs_vec2 m_pos = get_world_mouse_pos(gd);
		float angle = atan2f(m_pos.y - gd->player.position.y, m_pos.x - gd->player.position.x);
		if (gs_platform_mouse_down(GS_MOUSE_LBUTTON)) {
			angle += sin(gs_platform_elapsed_time()*0.015) * 0.5;
			size += abs(sin(gs_platform_elapsed_time()*0.02)) * 2;
		}
		gs_mat4 rotation = gs_mat4_rotatev(angle, gs_v3(0,0,1));
		mvp = gs_mat4_mul_list(4,
			gd->projection,
			translation,
			rotation,
			gs_mat4_scale(size, size, 0)
		);
		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
		size = 14 + 4 * sin( 3.14 * (gd->player.player_steps / PLAYER_FOOTSTEP_LENGTH));
		tex = player_body_png;
		angle = atan2f(m_pos.y - gd->player.position.y, m_pos.x - gd->player.position.x);
		rotation = gs_mat4_rotatev(angle, gs_v3(0,0,1));
		mvp = gs_mat4_mul_list(4,
			gd->projection,
			translation,
			rotation,
			gs_mat4_scale(size, size, 0)
		);
		gs_graphics_apply_bindings(gcb, &binds);
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

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

	}


	// Worms
	tex = worm_png;
	for (int i = 0; i < gs_dyn_array_size(gd->worms); i++) {
		entity_t* head = gd->worms[i];
		if (!head->worm_is_head)
			continue;
		gs_vec2 pos = head->position;

		if (head->worm_type == WORM_TYPE_BOSS)
			tex = worm_boss_head_png;
		else tex = worm_png;
		
		color = head->color;
		flash = head->flash;
		gs_mat4 translation = gs_mat4_translate(pos.x, pos.y, 10);
		float size = head->radius * 2;

		
		float angle = atan2f(head->velocity.y, head->velocity.x);
		gs_mat4 rotation = gs_mat4_rotatev(angle, gs_v3(0,0,1));
		mvp = gs_mat4_mul_list(4,
			gd->projection,
			translation,
			rotation,
			gs_mat4_scale(size, size, 0)
		);
		
		float alpha = 1.0;
		float dead_timer = head->dead_timer;
		if (head->dead) {
			alpha = 1 * (1-(dead_timer/0.25));
			alpha = gs_max(alpha, 0);
			
		}
		color.w = alpha;
		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		if (head->worm_type == WORM_TYPE_BOSS)
			tex = worm_boss_png;
		entity_t* parent = head;
		int i = 0;
		while (parent->worm_segment) {
			entity_t* child = parent->worm_segment;
			pos = child->position;
			alpha = 1 * (1-(gs_max(dead_timer - 0.05*i, 0)/0.25)); 
			alpha = gs_max(alpha, 0);
			
			
			color = head->color;
			color.x -= 0.04 * i;
			color.w = alpha;

			gs_mat4 translation = gs_mat4_translate(pos.x, pos.y, -i*0.1);
			size = (child->radius*2);

			float angle = atan2f(parent->position.y - child->position.y, parent->position.x - child->position.x);
			gs_mat4 rotation = gs_mat4_rotatev(angle, gs_v3(0,0,1));
			mvp = gs_mat4_mul_list(4,
				gd->projection,
				translation,
				rotation,
				gs_mat4_scale(size, size, 0)
			);
			if (!child->worm_segment) {
				tex = worm_boss_tail_png;
			}

			gs_graphics_apply_bindings(gcb, &binds);
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

			i++;
			parent = child;
		}
	}

	
	

	// turrets
	int t_size = gs_dyn_array_size(gd->turrets);
	for (int i = 0; i < t_size; i++) {
		entity_t* t = gd->turrets[i];

		color =  t->color;
		flash = t->flash;
		tex = turret_png;
		
		int turret_size = turret_png.desc.width;
		if (t->turret_type == TURRET_TYPE_BOSS) turret_size = turret_boss_base_png.desc.width;
		float size = turret_size * gs_min(1.0, t->turret_time_since_spawn / TURRET_ANIMATION_SPAWN_TIME);

		if (t->turret_type == TURRET_TYPE_BOSS) { // draw  legs
			tex = turret_boss_feet_png;
			for (int i = 0; i < 4; i++) {
				turret_leg_t* leg = &t->turret_legs[i];
				gs_mat4 translation = gs_mat4_translate(leg->position.x, leg->position.y, 0.0f);
				mvp = gs_mat4_mul_list(3,
					gd->projection,
					translation,
					gs_mat4_scale(size, size, 0)
				);

				gs_graphics_apply_bindings(gcb, &binds);
				gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
			} 

		}

		{ // draw base
			tex = turret_png;
			if (t->turret_type == TURRET_TYPE_BOSS) tex = turret_boss_base_png;
			gs_mat4 translation = gs_mat4_translate(t->position.x, t->position.y, 0.0f);
			mvp = gs_mat4_mul_list(3,
				gd->projection,
				translation,
				gs_mat4_scale(size, size, 0)
			);

			gs_graphics_apply_bindings(gcb, &binds);
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
		}

		{ // draw barrel
			tex = turret_barrel_png;
			if (t->turret_type == TURRET_TYPE_BOSS) tex = turret_boss_barrel_png;
			gs_mat4 rot = gs_mat4_rotate(t->turret_angle, 0, 0, 1);

			float time_since_shot = t->turret_shoot_time;
			float offset_power = (1- gs_min(time_since_shot/TURRET_BURST_SHOT_DELAY, 1.f));
			float x_offset = cos(t->turret_angle) * -5 * offset_power;
			float y_offset = sin(t->turret_angle) * -5 * offset_power;
			
			gs_mat4 translation = gs_mat4_translate(t->position.x + x_offset, t->position.y + y_offset, 0.0f);
			mvp = gs_mat4_mul_list(4,
				gd->projection,
				translation,
				rot,
				gs_mat4_scale(size, size, 0)
			);

			gs_graphics_apply_bindings(gcb, &binds);
			gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
		}

		{ // draw top
			if (t->turret_type == TURRET_TYPE_BOSS) {
				tex = turret_boss_top_png;
				gs_mat4 translation = gs_mat4_translate(t->position.x, t->position.y, 0.0f);
				mvp = gs_mat4_mul_list(3,
					gd->projection,
					translation,
					gs_mat4_scale(size, size, 0)
				);

				gs_graphics_apply_bindings(gcb, &binds);
				gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
			}

		}

	}

	// orbs
	int o_size = gs_dyn_array_size(gd->orbs);
	for (int i = 0; i < o_size; i++) {
		entity_t* o = gd->orbs[i];

		color = o->color;
		flash = o->flash;
		tex = circle_16px;

		gs_mat4 translation = gs_mat4_translate(o->position.x, o->position.y, 0.0f);

		float size = o->radius;
		gs_mat4 model = gs_mat4_mul_list(2, translation, gs_mat4_scale(size, size, 0));

		mvp = gs_mat4_mul(gd->projection, model);

		gs_graphics_apply_bindings(gcb, &binds);
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
	}

	// final boss
	int b_size = gs_dyn_array_size(gd->final_bosses);
	for (int i = 0; i < b_size; i++) {
		entity_t* b = gd->final_bosses[i];

		color = b->color;
		flash = b->flash;
		tex = final_boss_png;

		gs_mat4 translation = gs_mat4_translate(b->position.x, b->position.y, 0.0f);

		float size = final_boss_png.desc.width;
		size = size * gs_min(1.0, b->boss_time_since_spawn / FINAL_BOSS_ANIMATION_SPAWN_TIME);
		

		gs_mat4 rot = gs_mat4_rotate(b->boss_angle, 0, 0, 1);

		mvp = gs_mat4_mul_list(4,
			gd->projection,
			translation,
			rot,
			gs_mat4_scale(size, size, 0)
		);



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
	gs_graphics_pipeline_bind(gcb, gd->screen_pip);
	gs_graphics_apply_bindings(gcb, &binds);
	gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

}



