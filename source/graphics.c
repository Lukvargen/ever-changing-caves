#include "graphics.h"
#include "../data/tile_data.c"
#include "../data/screen_quad_data.c"
#include "../data/particle_data.c"
#include "../data/entity_data.c"


gs_asset_texture_t circle_16px; // not the best way...
gs_asset_texture_t turret_png;
gs_asset_texture_t turret_barrel_png;
gs_asset_texture_t worm_png;

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


	
	/*
	// Hpbar
	{
		float l = TILE_SIZE;
		float b = TILE_SIZE * 4;
		float r = TILE_SIZE * 20;
		float t = TILE_SIZE;
		gsi_rect(gsi, l, b, r, t,
		0, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		float padding = TILE_SIZE/2;
		l += padding;
		b -= padding;
		r -= padding;
		t += padding;
		gsi_rect(gsi, l, b, r, t,
		200, 0, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		float percent = 0.4;
		r = l + (r-l) * percent;
		gsi_rect(gsi, l, b, r, t,
		0, 200, 0, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
	*/

	// immediate draw submit and render pass
	

	//gsi_text(gsi, RESOLUTION_X/2, 50, "Some text!", NULL, false, 255,255,255,255);
	// render to frame buffer
	gs_graphics_begin_render_pass(gcb, gd->rp);
		gs_graphics_set_viewport(gcb, 0, 0, RESOLUTION_X, RESOLUTION_Y); // fit the texture
		gs_graphics_clear(gcb, &fb_clear);
		draw_tiles(gd, gcb);
		gsi_draw(gsi, gcb);
		draw_particles(gd, gcb);
		draw_entities(gd, gcb);
	gs_graphics_end_render_pass(gcb);


	// render to backbuffer
	gs_graphics_begin_render_pass(gcb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
		gs_graphics_set_viewport(gcb, 0, 0, ws.x, ws.y); // where to draw
		gs_graphics_clear(gcb, &clear);
		// draws quad with frame buffer image
		draw_screen(gd, gcb);
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
	
	gs_vec3 wall_color = {33/255.0, 11/255.0, 44/255.0};
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
		gs_mat4 translation = gs_mat4_translate(gd->player.position.x, gd->player.position.y, 0.0f);
		float size = gd->player.radius * 2;
		gs_mat4 model = gs_mat4_mul(translation, gs_mat4_scale(size, size, 0));
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

		color = gs_v4(200/255.0, 20/255.0, 20/255.0, 1.0);
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
			alpha = max(alpha, 0);
		}
		color.w = alpha;
		gs_graphics_apply_bindings(gcb, &binds); // kolla om man kan ha 2 apply bindings.
		gs_graphics_draw(gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});

		entity_t* parent = head;
		int i = 0;
		while (parent->worm_segment) {
			entity_t* child = parent->worm_segment;
			pos = child->position;
			alpha = 1 * (1-(max(dead_timer - 0.05*i, 0)/0.25)); 
			alpha = max(alpha, 0);


			color = gs_v4(0.6 - 0.04 * i, 20/255.0, 20/255.0, alpha);
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
		float size = 2 * p->radius * (1-(p->life_time / PROJECITLE_MAX_LIFE_TIME*1.f));
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

		color = gs_v4(1.0, 1.0, 1.0, 1.0);
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
		float x_offset = 0;
		float y_offset = 0;
		if (t->turret_shooting) {
			float time_since_shot = t->turret_shoot_time;
			float offset_power = (1- gs_min(time_since_shot/TURRET_BURST_SHOT_DELAY, 1.f));
			x_offset = cos(t->turret_angle) * -5 * offset_power;
			y_offset = sin(t->turret_angle) * -5 * offset_power;
			
		}
		translation = gs_mat4_translate(t->position.x + x_offset, t->position.y + y_offset, 0.0f);
		size = turret_barrel_size * gs_min(1.0, t->turret_time_since_spawn / TURRET_ANIMATION_SPAWN_TIME);
		model = gs_mat4_mul_list(3, translation, gs_mat4_scale(size, size, 0), rot);

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



