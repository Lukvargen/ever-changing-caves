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
void spawn_player(game_data_t* gd);
void update_player(game_data_t* gd);
void spawn_projectile(game_data_t* gd, projectile_t* projectile);
void update_projectiles(game_data_t* gd);
void update_tiles(game_data_t* gd);
void update_powerups(game_data_t* gd);
void update_worms(game_data_t* gd);
void update_particle_emitters(game_data_t* gd, float delta);

void spawn_turret(game_data_t* gd, gs_vec2 pos);
void update_turrets(game_data_t* gd, float delta);

void spawn_worm(game_data_t* gd, int segments);
void delete_worm(entity_t* worm);
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
void remove_entity(entity_t* arr[], int* arr_length, int* i);



void init() 
{
	game_data_t* gd = gs_engine_user_data(game_data_t);
	
	hit_sound_hndl = gs_audio_load_from_file("./assets/Hit_Hurt2.wav");
	gs_vec2 ws = window_size();

	// Initialize Game Data
	
	gd->projecitles = NULL;
	gd->worms = NULL;
	gd->powerups = NULL;
	gd->particle_emitters = NULL;
	gd->powerup_spawn_time = 0.f;
	gd->powerup_spawn_timer = 10.f;

	gd->worm_spawn_timer = 5.f;
	gd->worm_spawn_time = 0.f;

	gd->turret_spawn_timer = 10.f;

	gd->enemies = NULL;
	gs_dyn_array_push(gd->enemies, &gd->worms);
	gs_dyn_array_push(gd->enemies, &gd->turrets);

	/*
	spawn_worm(gd, 5, gs_v2(100, 100));
	spawn_worm(gd, 5, gs_v2(50, 100));

	printf("dyn array soze %i \n", gs_dyn_array_size(*gd->enemies[0]));

	entity_t** worms = *gd->enemies[0];
	entity_t* worm = *gd->enemies[0]+1;
	for (int i = 0; i < 2; i++) {
		printf("worm pos x %f \n", worms[i]->position.x);
	}
	printf("worm alone pos x %f \n", worm->position.x);
	//printf("dyn array soze %i \n", *(gd->enemies[0])[0]->hp);
	*/
		
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			gd->tiles[x][y].destruction_value = 0;
			gd->tiles[x][y].noise_value = 0;
			gd->tiles[x][y].value = 0;
		}
	}

	graphics_init(gd);


	spawn_player(gd);


}


/*=================
// Update Functions
=================*/
float t = 0.0;
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
	update_turrets(gd, delta);
	update_projectiles(gd);
	update_tiles(gd);
	update_particle_emitters(gd, delta);


	
	draw_game(gd);

	t += delta;
	if (t > 0.5) {
		t = 0.0;
		printf("FPS: %f\n", 1.0/delta);
	}
	
}

void update_tiles(game_data_t* gd)
{
	float delta = gs_platform_delta_time();
	float time = gs_platform_elapsed_time() * 0.0001f;
	
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

void spawn_player(game_data_t* gd)
{
	entity_t* p = &gd->player;
	p->position.x = RESOLUTION_X / 2;
	p->position.y = RESOLUTION_Y / 2;
	p->radius = 4.f;

	p->player_particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
		.particle_amount = 8,
		.particle_color = gs_v4(0.1, 0.3, 0.1, 0.25),
		.particle_lifetime = 0.25,
		.particle_shink_out = true,
		.particle_size = gs_v2(10, 10),
		.particle_velocity = gs_v2(20, 0),
		.rand_velocity_range = 0.75,
		.rand_rotation_range = 2 * 3.14,
		.position = gd->player.position
		
	});
	printf("Particle emitter %i\n", p->player_particle_emitter->emitting);
}
void update_player(game_data_t* gd)
{
	entity_t* p = &gd->player;
	gs_vec2* pos = &gd->player.position;
	gs_vec2* vel = &gd->player.velocity;
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

	// handel collision with walls
	int tile_x = pos->x/TILE_SIZE;
	int tile_y = pos->y/TILE_SIZE;
	if (is_tile_solid(gd, tile_x, tile_y)) {
		
		// take dmg or something
		// maybe explode after some time
		explode_tiles(gd, tile_x, tile_y, 3);
	} else {
		pos->x += vel->x * delta;
		pos->y += vel->y * delta;

		int center_y = pos->y / TILE_SIZE;
		int left_tile_pos = (pos->x - p->radius)/TILE_SIZE;
		int right_tile_pos = (pos->x + p->radius)/TILE_SIZE;
		if (is_tile_solid(gd, right_tile_pos, center_y)) {
			pos->x = right_tile_pos*TILE_SIZE - p->radius;
			p->velocity.x = 0;
		} else if (is_tile_solid(gd, left_tile_pos, center_y)) {
			pos->x = left_tile_pos*TILE_SIZE + TILE_SIZE + p->radius;
			p->velocity.x = 0;
		}
		int center_x = pos->x / TILE_SIZE;
		int up_tile_pos = (pos->y - p->radius)/TILE_SIZE;
		int down_tile_pos = (pos->y + p->radius)/TILE_SIZE;
		if (is_tile_solid(gd, center_x, down_tile_pos)) {
			pos->y = down_tile_pos*TILE_SIZE - p->radius;
			p->velocity.y = 0;
		} else if (is_tile_solid(gd, center_x, up_tile_pos)) {
			pos->y = up_tile_pos*TILE_SIZE + TILE_SIZE + p->radius;
			p->velocity.y = 0;
		}
	}

	p->player_particle_emitter->position = p->position;

	//printf("paRticle emitter %i\n", p->player_particle_emitter->emitting);

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
	

	p->turret_shoot_time += delta;
	if (gs_platform_mouse_down(GS_MOUSE_LBUTTON) && p->player_shoot_time >= PLAYER_SHOOT_TIMER) {
		p->turret_shoot_time = 0.f;
		gs_vec2 dir = gs_vec2_sub(get_world_mouse_pos(), *pos);
		dir = gs_vec2_norm(dir);
		
		gs_vec2 vel = gs_vec2_scale(dir, 300.f);

		// (float)rand()/(float)(RAND_MAX))
		
		
		spawn_projectile(gd, &(projectile_t){
			.position = *pos,
			.velocity = vel,
			.radius = 5,
			.accell = 200,
			.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
				.particle_amount = 8,
				.particle_color = gs_v4(0.1, 0.3, 0.3, 1.0),
				.particle_lifetime = 0.25,
				.particle_shink_out = true,
				.particle_size = gs_v2(8, 8),
				.position = *pos
				
			})
		});
		
	}

}

void spawn_projectile(game_data_t* gd, projectile_t* projectile)
{
	printf("spawn projectile\n");
	gs_dyn_array_push(gd->projecitles, *projectile);
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
		vel->x += dir.x * p->accell * delta;
		vel->y += dir.y * p->accell * delta;

		pos->x += vel->x * delta;
		pos->y += vel->y * delta;
		if (p->particle_emitter) {
			p->particle_emitter->position = *pos;
		}


		int tile_x = pos->x / TILE_SIZE;
		int tile_y = pos->y / TILE_SIZE;
		if (is_tile_solid(gd ,tile_x, tile_y)) {
			should_delete = true;
			explode_tiles(gd, tile_x, tile_y, 5);
			if (!p->enemy_created)
				gd->shake_time = 0.05;
		}

		p->life_time += delta;
		if (p->life_time > PROJECITLE_MAX_LIFE_TIME) {
			should_delete = true;
		}

		if (p->enemy_created) {

		} else { // spawned by player
			// Collisions with worms. Add collision with worm segments??
			//int worms_size = gs_dyn_array_size(gd->worms);

			// combine all enemies??????

			for (int enemy_list_i = 0; enemy_list_i < gs_dyn_array_size(gd->enemies); enemy_list_i++) {
				entity_t** enemy_list = *gd->enemies[enemy_list_i]; // read the value of the pointer to the list
				int list_size = gs_dyn_array_size(enemy_list);
				
				for (int j = 0; j < list_size; j++) {
					
					entity_t* enemy = enemy_list[j];
					if (enemy->dead)
						continue;
					if (is_colliding(*pos, enemy->position, p->radius, enemy->radius)) {
						//gs_println("Collision with worm");
						
						should_delete = true;
						gd->shake_time = 0.05;
						explode_tiles(gd, tile_x, tile_y, 5);
						gs_vec2 projectile_dir = gs_vec2_norm(p->velocity);
						// TODO make hit(entity) function
						enemy->hp -= 1;
						enemy->flash = 1.0;
						enemy->velocity = gs_vec2_add(enemy->velocity, gs_vec2_scale(projectile_dir, 200));
						//worm->dead_timer = 0.f;
						gs_vec2 particle_vel = projectile_dir;
						particle_vel.x *= 75;
						particle_vel.y *= 75;
						//printf("particle vel x %f, y %f \n", particle_vel.x, particle_vel.y);
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
			}
/*
			for (int j = 0; j < worms_size; j++) {
				entity_t* worm = gd->worms[j];
				if (worm->dead)
					continue;
				if (is_colliding(*pos, worm->position, p->radius, worm->radius)) {
					//gs_println("Collision with worm");
					should_delete = true;
					gd->shake_time = 0.05;
					explode_tiles(gd, tile_x, tile_y, 5);
					gs_vec2 projectile_dir = gs_vec2_norm(p->velocity);
					worm->hp -= 1;
					worm->flash = 1.0;
					worm->velocity = gs_vec2_add(worm->velocity, gs_vec2_scale(projectile_dir, 200));
					//worm->dead_timer = 0.f;
					gs_vec2 particle_vel = projectile_dir;
					particle_vel.x *= 75;
					particle_vel.y *= 75;
					//printf("particle vel x %f, y %f \n", particle_vel.x, particle_vel.y);
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
			}*/
		} // end spawned by player

		// skulle kunna bara swappa plats med sista elementet och sen pop
		// behöver vända på loopen och ha if delete: i-- p_size--
		if (should_delete) {
			if (p->particle_emitter) {
				p->particle_emitter->should_delete = true;
			}

			if (!played_sfx) {
				//gs_audio_play_source(hit_sound_hndl, 0.5f); 
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
	
	
	gd->worm_spawn_time -= delta;
	if (gd->worm_spawn_time <= 0) {
		gd->worm_spawn_time = gd->worm_spawn_timer * randf();
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
		entity_t* head = gd->worms[i];

		head->flash -= 5*delta;
		if (head->flash < 0)
			head->flash = 0.f;

		if (head->hp <= 0)
			head->dead = true;

		// .5 secounds delay after dead for death animation
		if (head->dead) {
			head->dead_timer += delta;
			head->worm_particle_emitter->emitting = false;
			if (head->dead_timer > 0.5f) {
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
			entity_t* other_worm = gd->worms[j];
			if (head == other_worm) 
				continue;
			gs_vec2 dist = gs_vec2_sub(other_worm->position, head->position);
			float len = gs_vec2_len(dist);
			float total_radius = head->radius+other_worm->radius;
			if (len < total_radius) {
				//gs_println("Colliding!");
				float force = 1-(len / total_radius);
				gs_vec2 norm = gs_vec2_norm(dist);
				head->velocity.x += -norm.x * force * WORM_REPEL_AMOUNT * delta;
				head->velocity.y += -norm.y * force * WORM_REPEL_AMOUNT * delta;
				other_worm->velocity.x += norm.x * force * WORM_REPEL_AMOUNT * delta;
				other_worm->velocity.y += norm.y * force * WORM_REPEL_AMOUNT * delta;
			}

		}

		if (!head->dead) {
			// speed boost in walls
			int x = head->position.x / TILE_SIZE;
			int y = head->position.y / TILE_SIZE;
			gs_vec2 target_pos = gd->player.position;
			if (is_tile_solid(gd, x, y)) {
				head->velocity = steer(head->position, target_pos, head->velocity, WORM_MAX_VELOCITY*2.f, WORM_MAX_FORCE*2.f, WORM_MAX_SPEED*2.f);
			} else {
				head->velocity = steer(head->position, target_pos, head->velocity, WORM_MAX_VELOCITY, WORM_MAX_FORCE, WORM_MAX_SPEED);
			}
		} else {
			head->velocity.x *= 0.5;
			head->velocity.y *= 0.5;
		}
		
		head->position.x += head->velocity.x * delta;
		head->position.y += head->velocity.y * delta;

		head->worm_particle_emitter->position = head->position;

		// segment follows parent
		entity_t* parent = head;
		while (parent->worm_segment) {
			entity_t* child = parent->worm_segment;
			gs_vec2 dist = gs_vec2_sub(parent->position, child->position);
			if (gs_vec2_len(dist) > 2) {
				child->position.x += dist.x * 10 * delta;
				child->position.y += dist.y * 10 * delta;
			}
			parent = child;
		}

	}
	
}

void update_powerups(game_data_t* gd) 
{

	float delta = gs_platform_delta_time();
	gs_vec2 ws = window_size();

	gd->powerup_spawn_time -= delta;
	if (gd->powerup_spawn_time <= 0) {
		gd->powerup_spawn_time = gd->powerup_spawn_timer * randf();
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
		if (is_colliding(p->position, gd->player.position, p->radius, gd->player.radius)) {
			
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

void spawn_turret(game_data_t* gd, gs_vec2 pos)
{
	entity_t* turret = malloc(sizeof(entity_t));
	*turret = (entity_t) {
		.position = pos,
		.radius = 9,
		.hp = 3,
		.turret_shoot_delay = 1.f
	};
	printf("spawn turret");
	gs_dyn_array_push(gd->turrets, turret);
}
void update_turrets(game_data_t* gd, float delta)
{
	gd->turret_spawn_time -= delta;
	if (gd->turret_spawn_time <= 0) {
		gd->turret_spawn_time = gd->turret_spawn_timer * randf();
		gs_vec2 spawn_pos;
		spawn_pos.x = (RESOLUTION_X-20) * randf() + 20;
		spawn_pos.y = (RESOLUTION_Y-20) * randf() + 20;
		spawn_turret(gd, spawn_pos);
	}

	

	int turrets_size = gs_dyn_array_size(gd->turrets);
	for (int i = 0; i < turrets_size; i++) {
		entity_t* t = gd->turrets[i];
		t->flash -= 5*delta;
		if (t->flash < 0)
			t->flash = 0.f;
		t->turret_time_since_spawn += delta;
		

		if (t->dead) {
			// måste fixa något death animation system
			remove_entity(gd->turrets, &turrets_size, &i);
			continue;
		}

		gs_vec2 player_pos = gd->player.position;
		t->turret_angle = atan2f(player_pos.y - t->position.y, player_pos.x - t->position.x);
		gs_vec2 target_dir = gs_vec2_sub(player_pos, t->position);
		target_dir = gs_vec2_norm(target_dir);

		if (t->turret_time_since_spawn < TURRET_ANIMATION_SPAWN_TIME)
			continue;
		
		if (!t->turret_shooting) {
			t->turret_shoot_time += delta;
			if (t->turret_shoot_time >= t->turret_shoot_delay) {
				t->turret_shot_count = 0;
				t->turret_shooting = true;
			}
		}
		if (t->turret_shooting) {
			t->turret_shoot_time += delta;
			if (t->turret_shoot_time >= TURRET_BURST_SHOT_DELAY) {
				t->turret_shoot_time = 0.f;
				// spawn projectile
				gs_vec2 projectile_vel = gs_vec2_scale(target_dir, 100);
				spawn_projectile(gd, &(projectile_t){
					.enemy_created = true,
					.position = t->position,
					.velocity = projectile_vel,
					.radius = 4,
					.accell = 10,
					.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
						.particle_amount = 6,
						.particle_color = gs_v4(0.3, 0.1, 0.1, 1.0),
						.particle_lifetime = 0.15,
						.particle_shink_out = true,
						.particle_size = gs_v2(8, 8),
						.position = t->position
						
					})
				});
				/*
				spawn_projectile(gd, &(projectile_t){
					.position = *pos,
					.velocity = vel,
					.radius = 5,
					.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
						.particle_amount = 8,
						.particle_color = gs_v4(0.1, 0.3, 0.3, 1.0),
						.particle_lifetime = 0.25,
						.particle_shink_out = true,
						.particle_size = gs_v2(8, 8),
						.position = *pos
						
					})
				});
				
				*/


				t->turret_shot_count++;
				if (t->turret_shot_count >= TURRET_BURST_COUNT) {
					t->turret_shot_count = 0;
					t->turret_shooting = false;
				}
			}
		}
		

	}
}

void spawn_worm(game_data_t* gd, int segments, gs_vec2 pos)
{
	entity_t* head = malloc(sizeof(entity_t));
	*head = (entity_t){
		.hp = 3,
		.position = pos,
		.radius = 6,
	};

	
	
	head->worm_particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
		.particle_amount 	= 10,
		.particle_color 	= gs_v4(0.4, 0.05, 0.05, 0.5),
		.particle_lifetime 	= 1.0f,
		.particle_shink_out = true,
		.particle_size 		= gs_v2(6,6),
		.particle_velocity 	= gs_v2(20.f, 0.f),
		.position 			= pos,
		.explode = false,
		.rand_rotation_range = 3.14,
		.rand_velocity_range = 0.5
	});
	
	entity_t* parent = head;
	float radius_dec = 1.0 / (segments+1.0);
	for (int i = 0; i < segments; i++) {
		entity_t* segment = malloc(sizeof(entity_t));
		segment->hp = 1;
		segment->position = pos;
		segment->radius = 5 - radius_dec *i * 4;
		segment->worm_particle_emitter = NULL;
		segment->worm_segment = NULL;

		parent->worm_segment = segment;
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

void delete_worm(entity_t* worm)
{
	if (worm->worm_particle_emitter)
		worm->worm_particle_emitter->should_delete = true;
	entity_t* next = worm->worm_segment;
	free(worm);
	if (next)
		delete_worm(next);
}


/*=================
// Helper Functions
=================*/

void remove_entity(entity_t* arr[], int* arr_length, int* i)
{
	arr[*i] = arr[*arr_length-1];
	gs_dyn_array_pop(arr);
	*arr_length--;
	*i--;
}

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
