#ifndef __MAIN__
#define __MAIN__
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>


/*======================
// Constants And Defines
======================*/
#define PLAYER_SPEED	150.f
#define PLAYER_ACCEL	8.f
#define PLAYER_SHOOT_TIMER	0.25f

#define WORM_MAX_VELOCITY	200.f
#define WORM_MAX_FORCE		6.f
#define WORM_MAX_SPEED		150.f
#define WORM_REPEL_AMOUNT	100.f

#define PROJECITLE_MAX_LIFE_TIME 3.f
#define EXPLODE_DMG 5.f

#define TILES_SIZE_X	80
#define TILES_SIZE_Y	45
#define TILE_SIZE		8
#define TILE_AIR_THRESHOLD 0.4f


#define window_size(...)	gs_platform_window_sizev(gs_platform_main_window())
#define RESOLUTION_X	640.f // 480
#define RESOLUTION_Y	360.f // 270


typedef enum powerup
{
	POWERUP_SPEED,
	POWERUP_ATTACK_RATE
} powerup;

typedef struct tile_t
{
	float value;
	float noise_value;
	float destruction_value;
} tile_t;

typedef struct entity_t
{
	int hp;
	int radius;
	bool dead;
	float dead_timer;
	gs_vec2 position;
	gs_vec2 velocity;
} entity_t;

typedef struct player_t
{
	entity_t entity;
	float shoot_time;
} player_t;


typedef struct powerup_t
{
	powerup type;
	gs_vec2 position;
	float radius;
	float lifespan;
} powerup_t;

typedef struct particle_t
{
	gs_vec2 position;
	gs_vec2 velocity;
	gs_vec2 size;
	gs_vec4 color;
	float life_time;
	bool dead;
} particle_t;

typedef struct particle_emitter_t
{
	gs_vec2 position;
	bool emitting;
	bool explode;
	bool one_shot;
	bool should_delete;
	float spawn_timer;
	float spawn_delay;
	int particle_amount;
	int particles_alive;
	// particle properties
	float particle_life_time;
	gs_vec2 particle_velocity;
	float rand_rotation_range;
	float rand_velocity_range;
	gs_vec2 particle_size;
	gs_vec4 particle_color;
	bool particle_shrink_out;
	
	gs_dyn_array(particle_t) particles;
	int last_dead_index;
} particle_emitter_t;

typedef struct particle_emitter_desc_t
{
	gs_vec2 position;
	int particle_amount;
	float particle_lifetime;
	gs_vec4 particle_color;
	gs_vec2 particle_size;
	gs_vec2 particle_velocity;
	float rand_rotation_range;
	float rand_velocity_range;
	bool particle_shink_out;
	bool explode;
	bool one_shot;
} particle_emitter_desc_t;

typedef struct worm_t
{
	entity_t entity;
	struct worm_t* segment;
	particle_emitter_t* particle_emitter;
} worm_t;

typedef struct projectile_t
{
	gs_vec2 position;
	gs_vec2 velocity;
	int radius;
	float life_time;
} projectile_t;



typedef struct game_data_t 
{
	gs_command_buffer_t gcb;
	gs_immediate_draw_t gsi;
	player_t player;
	gs_dyn_array(projectile_t) projecitles;
	// tile
	tile_t tiles[TILES_SIZE_X][TILES_SIZE_Y];
	gs_handle(gs_graphics_vertex_buffer_t) tile_vbo;
	gs_handle(gs_graphics_index_buffer_t)  tile_ibo;
	gs_handle(gs_graphics_pipeline_t) tile_pip;
	gs_handle(gs_graphics_shader_t)	tile_shader;
	gs_handle(gs_graphics_uniform_t) u_tile_color;
	gs_handle(gs_graphics_uniform_t) u_tile_model;
	gs_handle(gs_graphics_uniform_t) u_tile_projection;
	// framebuffer
	gs_handle(gs_graphics_render_pass_t) rp;
	gs_handle(gs_graphics_framebuffer_t) fbo;
	gs_handle(gs_graphics_texture_t)	rt;
	// screen quad
	gs_handle(gs_graphics_vertex_buffer_t) screen_vbo;
	gs_handle(gs_graphics_index_buffer_t) screen_ibo;
	gs_handle(gs_graphics_pipeline_t) screen_pip;
	gs_handle(gs_graphics_shader_t) screen_shader;
	gs_handle(gs_graphics_uniform_t) u_screen_texture;
	gs_handle(gs_graphics_uniform_t) u_screen_res;
	gs_handle(gs_graphics_uniform_t) u_screen_shake;
	gs_handle(gs_graphics_uniform_t) u_screen_time;



	// particle
	gs_handle(gs_graphics_vertex_buffer_t) particle_vbo;
	gs_handle(gs_graphics_index_buffer_t) particle_ibo;
	gs_handle(gs_graphics_pipeline_t) particle_pip;
	gs_handle(gs_graphics_shader_t) particle_shader;
	gs_handle(gs_graphics_uniform_t) u_particle_texture;
	gs_handle(gs_graphics_uniform_t) u_particle_color;
	gs_handle(gs_graphics_uniform_t) u_particle_mvp;

	gs_dyn_array(worm_t*) worms;
	float spawn_worm_timer;
	float spawn_worm_time;
	gs_dyn_array(powerup_t*) powerups;
	float spawn_powerup_timer;
	float spawn_powerup_time;
	gs_mat4 projection;
	float shake_time;

	gs_dyn_array(particle_emitter_t*) particle_emitters;

	

} game_data_t;

#endif