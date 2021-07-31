#ifndef MAIN_H
#define MAIN_H
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include "shop.h"


/*======================
// Constants And Defines
======================*/
#define PLAYER_SPEED	150.f
#define PLAYER_ACCEL	8.f

#define PLAYER_BASE_PROJECTILE_LIFETIME 0.5f
#define PLAYER_BASE_PROJECITLE_SPEED	200.f
#define PLAYER_BASE_PROJECTILE_ACCELL 100.f
#define PLAYER_BASE_EXPLOSION_RADIUS 2
#define PLAYER_BASE_SHOOT_DELAY 0.3f
#define PLAYER_BASE_DMG 1

#define WORM_MAX_VELOCITY	200.f
#define WORM_MAX_FORCE		6.f
#define WORM_MAX_SPEED		150.f
#define WORM_REPEL_AMOUNT	100.f

#define TURRET_BURST_COUNT	3
#define TURRET_BURST_SHOT_DELAY	0.2f
#define TURRET_ANIMATION_SPAWN_TIME 1.f

#define PROJECITLE_MAX_LIFE_TIME 3.f
#define EXPLODE_DMG 5.f

#define TILES_SIZE_X	80 // RESOLUTION_X / TILE_SIZE
#define TILES_SIZE_Y	45 // RESOLUTION_Y / TILE_SIZE
#define TILE_SIZE		8
#define TILE_AIR_THRESHOLD 0.4f



#define window_size(...)	gs_platform_window_sizev(gs_platform_main_window())
#define RESOLUTION_X	640.f
#define RESOLUTION_Y	360.f




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


typedef enum entity_type_t
{
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_WORM,
	ENTITY_TYPE_TURRET,
	ENTITY_TYPE_ORB
} entity_type_t;


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

typedef struct crystal_t
{
	gs_vec2 position;
	gs_vec2 velocity;
	float radius;
	float time_alive;
	particle_emitter_t* particle_emitter;
} crystal_t;

typedef enum orb_type_t
{
	ORB_TYPE_PINK,
	ORB_TYPE_BLUE,
	ORB_TYPE_SIZE
} orb_type_t;

typedef enum turret_type_t
{
	TURRET_TYPE_NORMAL,
	TURRET_TYPE_SPIN,
	TURRET_TYPE_WORM_SPAWN
} turret_type_t;

typedef enum worm_type_t
{
	WORM_TYPE_NORMAL,
	WORM_TYPE_SINUS
} worm_type_t;

typedef struct entity_t
{
	entity_type_t type;
	int hp;
	int max_hp;
	int dmg;
	float radius;
	bool dead;
	float dead_timer;
	float flash;
	float time_alive;
	gs_vec2 position;
	gs_vec2 velocity;
	gs_vec4 color;

	union
	{
		struct // player
		{
			float player_shoot_time; 
			float player_projectile_lifetime;
			float player_projectile_speed;
			float player_projectile_accel;
			int player_explosion_radius;
			float player_shoot_delay;
			float player_projectile_reflect_chance;
			int player_projectile_reflect_amount;
			int player_laser_lvl;
			
			particle_emitter_t* player_particle_emitter;
			
		};
		struct // worm
		{
			struct entity_t* worm_segment;
			particle_emitter_t* worm_particle_emitter;
			worm_type_t worm_type;
		};
		struct // turret
		{
			float turret_shoot_time;
			float turret_shoot_delay;
			float turret_burst_shoot_delay;
			float turret_angle;
			int turret_shot_count;
			gs_vec2 turret_dir;
			float turret_time_since_spawn;
			bool turret_shooting;
			turret_type_t turret_type;
			

		};
		struct // orb
		{
			particle_emitter_t* orb_particle_emitter;
			orb_type_t orb_type;
			float target_speed;
			float charge_timer;
		};
		
	};
} entity_t;

typedef struct laser_t
{
	gs_dyn_array(gs_vec2) points;
	float time_alive;
	float max_time_alive;
	gs_color_t color;
	int max_targets;
} laser_t;

typedef struct projectile_t
{
	gs_vec2 position;
	gs_vec2 velocity;
	float accell;
	float radius;
	float life_time;
	float max_life_time;
	int dmg;
	int explode_radius;
	entity_t* entity_ignore;
	particle_emitter_t* particle_emitter;
	bool enemy_created;
	bool go_through_walls;
} projectile_t;

typedef struct game_data_t 
{
	gs_command_buffer_t gcb;
	gs_immediate_draw_t gsi;

	// audio
	gs_handle(gs_audio_source_t) hit_sound_hndl;
	gs_handle(gs_audio_source_t) crystal_pickup_sound_hndl;
	gs_handle(gs_audio_source_t) hit_wall_sound_hndl;
	gs_handle(gs_audio_source_t) buy_positive_sound_hndl;
	gs_handle(gs_audio_source_t) buy_negative_sound_hndl;

	entity_t player;
	gs_dyn_array(projectile_t) projecitles;
	gs_dyn_array(laser_t) lasers;
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

	// entity
	gs_handle(gs_graphics_vertex_buffer_t) entity_vbo;
	gs_handle(gs_graphics_index_buffer_t) entity_ibo;
	gs_handle(gs_graphics_pipeline_t) entity_pip;
	gs_handle(gs_graphics_shader_t) entity_shader;
	gs_handle(gs_graphics_uniform_t) u_entity_texture;
	gs_handle(gs_graphics_uniform_t) u_entity_color;
	gs_handle(gs_graphics_uniform_t) u_entity_flash;
	gs_handle(gs_graphics_uniform_t) u_entity_mvp;

	gs_dyn_array(entity_t*) worms;

	gs_dyn_array(powerup_t*) powerups;

	gs_dyn_array(entity_t*) turrets;
	
	gs_dyn_array(entity_t*) orbs;

	gs_dyn_array(crystal_t) crystals;
	int crystals_currency;

	gs_mat4 projection;
	float shake_time;

	gs_dyn_array(particle_emitter_t*) particle_emitters;

	gs_dyn_array(entity_t**) enemies; // collection of all enemy arrays

	struct shop_t shop;
	// wave stuff
	int wave;
	float spawn_timer;
	gs_dyn_array(entity_type_t) enemies_to_spawn;
	float open_shop_timer;

	float time;
	bool paused;
	bool game_over;
	bool restart;
	bool mute;
	float volume;

} game_data_t;


gs_vec2 get_world_mouse_pos();
void next_wave(game_data_t* gd);
void restart_game(game_data_t* gd);


#endif