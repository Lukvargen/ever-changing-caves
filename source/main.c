#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "main.h"
#include "graphics.h"

gs_handle(gs_audio_source_t) hit_sound_hndl = {0}; // move me!



// Forward Declares
void update_player(game_data_t* gd);
void update_projectiles(game_data_t* gd);
void update_tiles(game_data_t* gd);
void update_powerups(game_data_t* gd);
void update_worms(game_data_t* gd);
void update_particle_emitters(game_data_t* gd, float delta);


void spawn_worm(game_data_t* gd, int segments);
void delete_worm(worm_t* worm);
void spawn_powerup(game_data_t* gd, powerup type, gs_vec2 pos);
particle_emitter_t* spawn_particle_emitter(game_data_t* gd, particle_emitter_desc_t* desc);
void delete_particle_emitter(particle_emitter_t* p);

bool is_tile_solid(game_data_t* gd, int tile_x, int tile_y);
void explode_tiles(game_data_t* gd, int tile_x, int tile_y, int radius);
gs_vec2 truncate(gs_vec2 v, float max_length);
bool is_colliding(gs_vec2 p1, gs_vec2 p2, float r1, float r2);
gs_vec2 get_world_mouse_pos();
gs_vec2 steer(gs_vec2 from_pos, gs_vec2 target_pos, gs_vec2 velocity, float max_velocity, float max_force, float max_speed);
float randf();



void init() 
{
	game_data_t* gd = gs_engine_user_data(game_data_t);
	
	hit_sound_hndl = gs_audio_load_from_file("./assets/Hit_Hurt2.wav");
	gs_vec2 ws = window_size();

	// Initialize Game Data
	gd->player.entity.position = gs_vec2_scale(ws, 0.5f);
	gd->player.entity.radius = 8.f;
	gd->projecitles = NULL;
	gd->worms = NULL;
	gd->powerups = NULL;
	gd->particle_emitters = NULL;
	gd->spawn_powerup_time = 0.f;
	gd->spawn_powerup_timer = 10.f;

	gd->spawn_worm_timer = 5.f;
	gd->spawn_worm_time = 0.f;

	

	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			gd->tiles[x][y].destruction_value = 0;
			gd->tiles[x][y].noise_value = 0;
			gd->tiles[x][y].value = 0;
		}
	}

	graphics_init(gd);
}


/*=================
// Update Functions
=================*/

void update()
{
	if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
		gs_engine_quit();
	} else if (gs_platform_key_pressed(GS_KEYCODE_R)) {
		init();
		return;
	}

	gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
	game_data_t* gd = gs_engine_user_data(game_data_t);
	float delta = gs_platform_delta_time();

	gd->shake_time -= delta;


	update_player(gd);
	update_powerups(gd);
	update_worms(gd);
	update_projectiles(gd);
	update_tiles(gd);
	update_particle_emitters(gd, delta);

	
	draw_game(gd);

}

void update_tiles(game_data_t* gd)
{
	float delta = gs_platform_delta_time();
	float time = gs_platform_elapsed_time() * 0.0001f;
	//t += delta * 0.1;
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			tile_t* tile = &gd->tiles[x][y];
			tile->noise_value = (stb_perlin_noise3(x*0.1, y*0.1, time, 0, 0, 0) + 1) / 2.f; // wrap needs to be power of 2... Change perlin noise file maybe
			tile->destruction_value -= 0.1 * delta;
			if (tile->destruction_value < 0) tile->destruction_value = 0;
			tile->value = tile->noise_value + tile->destruction_value;
			
			if (tile->value < 0) tile->value = 0;
			else if (tile->value > 1) tile->value = 1;
		}
	}
}

void update_player(game_data_t* gd)
{
	player_t* p = &gd->player;
	gs_vec2* pos = &gd->player.entity.position;
	gs_vec2* vel = &gd->player.entity.velocity;
	float delta = gs_platform_delta_time();
	gs_vec2 dir = {0};
	if (gs_platform_key_down(GS_KEYCODE_W)) {
		dir.y -= 1;
	}
	if (gs_platform_key_down(GS_KEYCODE_S)) {
		dir.y += 1;
	}
	if (gs_platform_key_down(GS_KEYCODE_A)) {
		dir.x -= 1;
	}
	if (gs_platform_key_down(GS_KEYCODE_D)) {
		dir.x += 1;
	}
	
	dir = gs_vec2_norm(dir);
	
	vel->x += delta * PLAYER_ACCEL * ((dir.x * PLAYER_SPEED) - vel->x);
	vel->y += delta * PLAYER_ACCEL * ((dir.y * PLAYER_SPEED) - vel->y);
	
	pos->x += vel->x * delta;
	pos->y += vel->y * delta;

	// wrap world
	float width = TILES_SIZE_X * TILE_SIZE;
	float height = TILES_SIZE_Y * TILE_SIZE;
	if (pos->x < 0)
		pos->x = width;
	else if (pos->x > width)
		pos->x = 0;
	if (pos->y < 0)
		pos->y = height;
	else if (pos->y > height)
		pos->y = 0;
	

	p->shoot_time += delta;
	if (gs_platform_mouse_down(GS_MOUSE_LBUTTON) && p->shoot_time >= PLAYER_SHOOT_TIMER) {
		p->shoot_time = 0.f;
		gs_vec2 dir = gs_vec2_sub(get_world_mouse_pos(), *pos);
		dir = gs_vec2_norm(dir);
		
		gs_vec2 vel = gs_vec2_scale(dir, 300.f);

		// (float)rand()/(float)(RAND_MAX))
		projectile_t p = {.position = *pos, .velocity = vel, .radius = 5};
		gs_dyn_array_push(gd->projecitles, p);
	}

}

void update_projectiles(game_data_t* gd)
{
	float delta = gs_platform_delta_time();

	int p_size = gs_dyn_array_size(gd->projecitles);	
	bool played_sfx = false;

	for (int i = p_size-1; i >= 0; i--) {
		projectile_t* p = &gd->projecitles[i];
		gs_vec2* pos = &p->position;
		gs_vec2* vel = &p->velocity;
		bool should_delete = false;
		
		gs_vec2 dir = gs_vec2_norm(*vel);
		vel->x += dir.x * 200 * delta;
		vel->y += dir.y * 200 * delta;

		pos->x += vel->x * delta;
		pos->y += vel->y * delta;

		int tile_x = pos->x / TILE_SIZE;
		int tile_y = pos->y / TILE_SIZE;
		if (is_tile_solid(gd ,tile_x, tile_y)) {
			should_delete = true;
			gd->shake_time = 0.05;
			explode_tiles(gd, tile_x, tile_y, 5);
		}

		p->life_time += delta;
		if (p->life_time > PROJECITLE_MAX_LIFE_TIME) {
			should_delete = true;
		}

		// Collisions with worms. Add collision with worm segments??
		int worms_size = gs_dyn_array_size(gd->worms);
		for (int j = 0; j < worms_size; j++) {
			worm_t* worm = gd->worms[j];
			if (worm->entity.dead)
				continue;
			if (is_colliding(*pos, worm->entity.position, p->radius, worm->entity.radius)) {
				//gs_println("Collision with worm");
				should_delete = true;
				gd->shake_time = 0.05;
				explode_tiles(gd, tile_x, tile_y, 5);
				
				worm->entity.dead = true;
				worm->entity.dead_timer = 0.f;
				gs_vec2 particle_vel = gs_vec2_norm(p->velocity);
				particle_vel.x *= 75;
				particle_vel.y *= 75;
				printf("particle vel x %f, y %f \n", particle_vel.x, particle_vel.y);
				// spawn hit particle
				spawn_particle_emitter(gd, &(particle_emitter_desc_t){
					.particle_amount 	= 8,
					.particle_color 	= gs_v4(0.4, 0.1, 0.1, 1.0),
					.particle_lifetime 	= 0.75f,
					.particle_shink_out = true,
					.particle_size 		= gs_v2(12,12),
					.particle_velocity 	= particle_vel,
					.position 			= *pos,
					.explode = true,
					.rand_rotation_range = 1.0,
					.rand_velocity_range = 0.5,
					.one_shot = true
				});

				break;
			}
		}

		// skulle kunna bara swappa plats med sista elementet och sen pop
		// behöver vända på loopen och ha if delete: i-- p_size--
		if (should_delete) {
			if (!played_sfx) {
				// gs_audio_play_source(hit_sound_hndl, 0.5f); 
				played_sfx = true;
			}
			if (i < p_size-1) {
				for (int j = i+1; j < gs_dyn_array_size(gd->projecitles); j++) {
					gd->projecitles[j-1] = gd->projecitles[j];
				}
			}
			
			gs_dyn_array_pop(gd->projecitles);
		}

	}
}

void update_worms(game_data_t* gd)
{
	float delta = gs_platform_delta_time();
	gs_vec2 ws = window_size();
	
	
	gd->spawn_worm_time -= delta;
	if (gd->spawn_worm_time <= 0) {
		gd->spawn_worm_time = gd->spawn_worm_timer * randf();
		gs_vec2 spawn_pos;
		spawn_pos.x = RESOLUTION_X * randf();
		spawn_pos.y = 0;
		if (randf() > 0.5) {
			spawn_pos.y = RESOLUTION_Y;
		}
		spawn_worm(gd, 5 + rand()%5, spawn_pos);
	}
	

	int worms_size = gs_dyn_array_size(gd->worms);
	

	for (int i = 0; i < worms_size; i++) {
		worm_t* head = gd->worms[i];

		// .5 secounds delay after dead for death animation
		if (head->entity.dead) {
			head->entity.dead_timer += delta;
			head->particle_emitter->emitting = false;
			if (head->entity.dead_timer > 0.5f) {
				gd->worms[i] = gd->worms[worms_size-1];
				gs_dyn_array_pop(gd->worms);
				worms_size--;
				i--;
				// free from memory
				delete_worm(head);
				continue;
			}
		}

		// add repell to other worms
		for (int j = 0; j < worms_size; j++) {
			worm_t* other_worm = gd->worms[j];
			if (head == other_worm) 
				continue;
			gs_vec2 dist = gs_vec2_sub(other_worm->entity.position, head->entity.position);
			float len = gs_vec2_len(dist);
			float total_radius = head->entity.radius+other_worm->entity.radius;
			if (len < total_radius) {
				//gs_println("Colliding!");
				float force = 1-(len / total_radius);
				gs_vec2 norm = gs_vec2_norm(dist);
				head->entity.velocity.x += -norm.x * force * WORM_REPEL_AMOUNT * delta;
				head->entity.velocity.y += -norm.y * force * WORM_REPEL_AMOUNT * delta;
				other_worm->entity.velocity.x += norm.x * force * WORM_REPEL_AMOUNT * delta;
				other_worm->entity.velocity.y += norm.y * force * WORM_REPEL_AMOUNT * delta;
			}

		}

		if (!head->entity.dead) {
			// speed boost in walls
			int x = head->entity.position.x / TILE_SIZE;
			int y = head->entity.position.y / TILE_SIZE;
			gs_vec2 target_pos = gd->player.entity.position;
			if (is_tile_solid(gd, x, y)) {
				head->entity.velocity = steer(head->entity.position, target_pos, head->entity.velocity, WORM_MAX_VELOCITY*2.f, WORM_MAX_FORCE*2.f, WORM_MAX_SPEED*2.f);
			} else {
				head->entity.velocity = steer(head->entity.position, target_pos, head->entity.velocity, WORM_MAX_VELOCITY, WORM_MAX_FORCE, WORM_MAX_SPEED);
			}
		} else {
			head->entity.velocity.x *= 0.5;
			head->entity.velocity.y *= 0.5;
		}
		
		head->entity.position.x += head->entity.velocity.x * delta;
		head->entity.position.y += head->entity.velocity.y * delta;

		head->particle_emitter->position = head->entity.position;

		// segment follows parent
		worm_t* parent = head;
		while (parent->segment) {
			worm_t* child = parent->segment;
			gs_vec2 dist = gs_vec2_sub(parent->entity.position, child->entity.position);
			if (gs_vec2_len(dist) > 2) {
				child->entity.position.x += dist.x * 10 * delta;
				child->entity.position.y += dist.y * 10 * delta;
			}
			parent = child;
		}

	}
	
}

void update_powerups(game_data_t* gd) 
{

	float delta = gs_platform_delta_time();
	gs_vec2 ws = window_size();

	gd->spawn_powerup_time -= delta;
	if (gd->spawn_powerup_time <= 0) {
		gd->spawn_powerup_time = gd->spawn_powerup_timer * randf();
		gs_vec2 spawn_pos;
		spawn_pos.x = RESOLUTION_X * randf();
		spawn_pos.y = RESOLUTION_Y * randf();
		spawn_powerup(gd, POWERUP_ATTACK_RATE, spawn_pos);
	}

	int p_size = gs_dyn_array_size(gd->powerups);
	for (int i = 0; i < p_size; i++) {
		powerup_t* p = gd->powerups[i];
		p->lifespan += delta;
		p->radius = 8 + sin(p->lifespan*4) * 4;
		if (is_colliding(p->position, gd->player.entity.position, p->radius, gd->player.entity.radius)) {
			
			// do stuff

			gd->powerups[i] = gd->powerups[p_size-1];
			gs_dyn_array_pop(gd->powerups);
			free(p);
			p_size--;
			i--;
			
		}
	}
}

void update_particle_emitters(game_data_t* gd, float delta)
{
	int e_size = gs_dyn_array_size(gd->particle_emitters);
	for (int i = 0; i < e_size; i++) {
		particle_emitter_t* emitter = gd->particle_emitters[i];
		if (emitter->should_delete && emitter->particles_alive == 0) {
			delete_particle_emitter(emitter);
			gd->particle_emitters[i] = gd->particle_emitters[e_size-1];
			gs_dyn_array_pop(gd->particle_emitters);
			e_size--;
			i--;
			continue;
		}
		
		emitter->spawn_timer += delta;
		if (emitter->should_delete)
			emitter->emitting = false;
		for (int j = 0; j < emitter->particle_amount; j++) {
			particle_t* p = &emitter->particles[j];
			p->life_time += delta;
			if (!p->dead && p->life_time >= emitter->particle_life_time) {
				p->dead = true;
				emitter->particles_alive--;
				emitter->last_dead_index = j;
			}
			if (p->dead) {
				if (emitter->emitting) {
					// check respawn
					if (emitter->spawn_timer >= emitter->spawn_delay || emitter->explode) {
						// init particle
						emitter->spawn_timer = 0.f;
						p->position = emitter->position;
						p->life_time = 0.f;
						p->size = emitter->particle_size;
						p->velocity = emitter->particle_velocity;
						float rand_rot = 2.0*(0.5 - randf()) * emitter->rand_rotation_range;
						
						float vel_rand_range = 1.0 - (randf() * emitter->rand_velocity_range);
						float vel_x = p->velocity.x * vel_rand_range;
						float vel_y = p->velocity.y * vel_rand_range;
						p->velocity.x = vel_x * cos(rand_rot) - vel_y * sin(rand_rot);
						p->velocity.y = vel_x * sin(rand_rot) + vel_y * cos(rand_rot);
						
						p->color = emitter->particle_color;
						p->dead = false;
						emitter->particles_alive++;
					}

				}
			} else {
				p->position.x += p->velocity.x * delta;
				p->position.y += p->velocity.y * delta;

			}

		}
		if (emitter->one_shot) {
			emitter->emitting = false;
			emitter->should_delete = true;
		}
		
		


		//for (int j = 0; j < )


	}
}




void explode_tiles(game_data_t* gd, int tile_x, int tile_y, int radius)
{
	for (int x = tile_x - radius; x < tile_x + radius+1; x++) {
		for (int y = tile_y - radius; y < tile_y + radius+1; y++) {
			if (x < 0 || y < 0 || x >= TILES_SIZE_X || y >= TILES_SIZE_Y)
				continue;
			gs_vec2 vector = {.x = x- tile_x, .y = y - tile_y};
			float length = gs_vec2_len(vector);
			
			float force = 1 - (length / radius);
			gd->tiles[x][y].destruction_value += 1 * force;
		}
	}
}

particle_emitter_t* spawn_particle_emitter(game_data_t* gd, particle_emitter_desc_t* desc)
{
	particle_emitter_t* p = malloc(sizeof(particle_emitter_t));
	p->position = desc->position;
	p->particle_amount = desc->particle_amount;
	p->particle_life_time = desc->particle_lifetime;
	p->spawn_timer = 0.f;
	p->spawn_delay = desc->particle_lifetime / desc->particle_amount;
	p->particles = NULL;
	p->emitting = true;
	p->should_delete = false;
	p->particles_alive = 0;
	p->last_dead_index = 0;

	
	// particle properties
	p->particle_color = desc->particle_color;
	p->particle_size = desc->particle_size;
	p->particle_velocity = desc->particle_velocity;
	p->particle_shrink_out = desc->particle_shink_out;
	p->explode = desc->explode;
	p->rand_rotation_range = desc->rand_rotation_range;
	p->rand_velocity_range = desc->rand_velocity_range;
	p->one_shot = desc->one_shot;
	

	gs_dyn_array_push(gd->particle_emitters, p);

	for (int i = 0; i < p->particle_amount; i++) {
		particle_t particle = {0};
		particle.dead = true;
		gs_dyn_array_push(p->particles, particle);
	}

	// fyll på particles arrayen med antalet partiklar som ska spawnas
	// ha particle lifetime sen spawn_timer = particle_life_time / particle_amount
	// uniform float life_span som är från 0 till 1 kan ju vara bra med för fade out. Eller bara sätt färgen...

	return p;
}

void spawn_worm(game_data_t* gd, int segments, gs_vec2 pos)
{
	worm_t* head = malloc(sizeof(worm_t));
	head->entity.hp = 3;
	head->entity.position = pos;
	head->entity.radius = 8;
	head->entity.dead = false;
	head->entity.dead_timer = 0.f;
	
	
	head->particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
		.particle_amount 	= 10,
		.particle_color 	= gs_v4(0.4, 0.05, 0.05, 0.5),
		.particle_lifetime 	= 1.0f,
		.particle_shink_out = true,
		.particle_size 		= gs_v2(8,8),
		.particle_velocity 	= gs_v2(20.f, 0.f),
		.position 			= pos,
		.explode = false,
		.rand_rotation_range = 3.14,
		.rand_velocity_range = 0.5
	});
	
	worm_t* parent = head;
	for (int i = 0; i < segments; i++) {
		worm_t* segment = malloc(sizeof(worm_t));
		segment->entity.hp = 1;
		segment->entity.position = pos;
		segment->entity.radius = 6;
		segment->particle_emitter = NULL;
		segment->segment = NULL;

		parent->segment = segment;
		parent = segment;
	}
	gs_dyn_array_push(gd->worms, head);
}

void spawn_powerup(game_data_t* gd, powerup type, gs_vec2 pos)
{
	powerup_t* p = malloc(sizeof(powerup_t));
	p->position = pos;
	p->type = type;
	p->radius = 8;
	p->lifespan = 0.f;
	gs_dyn_array_push(gd->powerups, p);
}

void delete_particle_emitter(particle_emitter_t* p)
{
	gs_dyn_array_free(p->particles);
	free(p);
}

void delete_worm(worm_t* worm)
{
	if (worm->particle_emitter)
		worm->particle_emitter->should_delete = true;
	worm_t* next = worm->segment;
	free(worm);
	if (next)
		delete_worm(next);
}


/*=================
// Helper Functions
=================*/

bool is_tile_solid(game_data_t* gd, int tile_x, int tile_y)
{
	if (tile_x < 0 || tile_y < 0 || tile_x >= TILES_SIZE_X || tile_y >= TILES_SIZE_Y)
		return true;
	return gd->tiles[tile_x][tile_y].value < TILE_AIR_THRESHOLD;
}

gs_vec2 get_world_mouse_pos()
{
	gs_vec2 platform_m_pos = gs_platform_mouse_positionv();
	gs_vec2 window_size = window_size();
	float percent_x = platform_m_pos.x / window_size.x;
	float percent_y = platform_m_pos.y / window_size.y;
	gs_vec2 m_pos;
	m_pos.x = RESOLUTION_X * percent_x;
	m_pos.y = RESOLUTION_Y * percent_y;
	//gs_println("percent_x %f", percent_x);
	return m_pos;
}

bool is_colliding(gs_vec2 p1, gs_vec2 p2, float r1, float r2)
{
	return gs_vec2_dist(p1, p2) <= r1+r2;
}

gs_vec2 truncate(gs_vec2 v, float max_length)
{
	if (gs_vec2_len(v) > max_length) {
		return gs_vec2_scale(gs_vec2_norm(v), max_length);
	}
	return v;
}

gs_vec2 steer(gs_vec2 from_pos, gs_vec2 target_pos, gs_vec2 velocity, float max_velocity, float max_force, float max_speed)
{
	gs_vec2 desired_vel = gs_vec2_norm(gs_vec2_sub(target_pos, from_pos));
	desired_vel = gs_vec2_scale(desired_vel, max_velocity);
	gs_vec2 steering = gs_vec2_sub(desired_vel, velocity);
	steering = truncate(steering, max_force);

	velocity = gs_vec2_add(velocity, steering);
	return truncate(velocity, max_speed);
}

float randf() {
	return (float)rand()/(double)(RAND_MAX);
}


// Globals
game_data_t gdata = {0};

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
	return (gs_app_desc_t) {
		.window_width = RESOLUTION_X,
		.window_height = RESOLUTION_Y,
		.init = init,
		.update = update,
		.window_title = "Ever-changing Caves",
		.frame_rate = 120.f,
		.user_data = &gdata,
		.window_flags = GS_WINDOW_FLAGS_FULLSCREEN
	};
}
