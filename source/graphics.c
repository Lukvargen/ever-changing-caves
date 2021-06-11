#include "graphics.h"
#include "../data/tile_data.c"
#include "../data/screen_quad_data.c"
#include "../data/particle_data.c"


// Forward Declares
void draw_tiles(game_data_t* gd, gs_command_buffer_t* gcb);
void draw_screen(game_data_t* gd, gs_command_buffer_t* gcb);

void graphics_init(game_data_t* gd)
{
    gd->gcb = gs_command_buffer_new();
	gd->gsi = gs_immediate_draw_new();


    init_tiles(gd);
    init_screen_quad(gd);
    init_framebuffer(gd);
}

void draw_game(game_data_t* gd)
{
	gs_command_buffer_t* gcb = &gd->gcb;
	gs_immediate_draw_t* gsi = &gd->gsi;

	gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.8f, 0.1f, 1.f}}
	};

	gs_graphics_clear_desc_t fb_clear = (gs_graphics_clear_desc_t) {
		.actions = &(gs_graphics_clear_action_t){.color = {0.8f, 0.1f, 0.1f, 1.f}}
	};

	gsi_camera2D(gsi);
	gs_vec2 ws = window_size();
	gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

	float diff_x = ws.x / RESOLUTION_X;
	float diff_y = ws.y / RESOLUTION_Y;
	gd->projection = gs_mat4_ortho(0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100); // hur stor del av världen som ska tas med
	gsi_ortho(gsi, 0, RESOLUTION_X, RESOLUTION_Y, 0, 0, 100);


	// Player
	{
		gs_vec2 pos = gd->player.entity.position;
		gsi_circle(gsi, pos.x, pos.y, gd->player.entity.radius, 16,
			50, 150, 50, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}

	// Powerups
	for (int i = 0; i < gs_dyn_array_size(gd->powerups); i++) {
		powerup_t *p = gd->powerups[i];
		gsi_circle(gsi, p->position.x, p->position.y, p->radius, 4,
		50, 50, 200, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}


	// Worms
	for (int i = 0; i < gs_dyn_array_size(gd->worms); i++) {
		worm_t* head = gd->worms[i];
		gs_vec2 pos = head->entity.position;
		int alpha = 255;
		float dead_timer = head->entity.dead_timer;
		if (head->entity.dead) {
			alpha = 255 * (1-(dead_timer/0.25));
			alpha = max(alpha, 0);
		}
		gsi_circle(gsi, pos.x, pos.y, head->entity.radius, 8,
			200, 20, 20, alpha, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
		worm_t* parent = head;
		int i = 0;
		while (parent->segment) {
			worm_t* child = parent->segment;
			pos = child->entity.position;
			alpha = 255 * (1-(max(dead_timer - 0.05*i, 0)/0.25)); 
			alpha = max(alpha, 0);
			gsi_circle(gsi, pos.x, pos.y, child->entity.radius-i, 8,
				150 - 10 * i, 20, 20, alpha, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

			i++;
			parent = child;
		}
	}

	
	// Projectiles
	for (int i = 0; i < gs_dyn_array_size(gd->projecitles); i++) {
		projectile_t* p = &gd->projecitles[i];
		gs_vec2 pos = p->position;
		gsi_circle(gsi, pos.x, pos.y, p->radius * (1-(p->life_time / PROJECITLE_MAX_LIFE_TIME*1.f)), 8,
			200 + 50 * (p->life_time / PROJECITLE_MAX_LIFE_TIME*1.f), 50, 50, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	}
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
	

	gsi_text(gsi, RESOLUTION_X/2, 50, "Some text!", NULL, false, 255,255,255,255);
	// render to frame buffer
	gs_graphics_begin_render_pass(gcb, gd->rp);
		gs_graphics_set_viewport(gcb, 0, 0, RESOLUTION_X, RESOLUTION_Y); // fit the texture
		gs_graphics_clear(gcb, &fb_clear);
		draw_tiles(gd, gcb);
		gsi_draw(gsi, gcb); 
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
	gd->u_particle_res = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_res",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC2}
		}
	);
	gd->u_particle_color = gs_graphics_uniform_create(
		&(gs_graphics_uniform_desc_t) {
			.stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
			.name = "u_color",
			.layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC3}
		}
	);

	gd->particle_vbo = gs_graphics_vertex_buffer_create(
		&(gs_graphics_vertex_buffer_desc_t) {
			.data = particle_v_data,
			.size = sizeof(particle_v_data)
		}
	);
	gd->particle_ibo = gs_graphics_index_buffer_create(
		&(gs_graphics_index_buffer_desc_t) {
			.data = particle_i_data,
			.size = sizeof(particle_i_data)
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
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"}
                },
                .size = 1 * sizeof(gs_graphics_vertex_attribute_desc_t)
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
			.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, // GS_GRAPHICS_TEXTURE_FILTER_NEAREST
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


void draw_screen(game_data_t* gd, gs_command_buffer_t* gcb)
{

	gs_vec2 res = {RESOLUTION_X, RESOLUTION_Y};

	gs_graphics_bind_uniform_desc_t uniforms[] = {
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



