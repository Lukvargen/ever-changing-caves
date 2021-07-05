#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>


#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "main.h"
#include "graphics.h"


gs_handle(gs_audio_source_t) hit_sound_hndl = {0}; // move me!
gs_handle(gs_audio_instance_t) hit_sound_instance_hndl = {0};



// Forward Declares
void spawn_player(game_data_t* gd);
void update_player(game_data_t* gd);
void spawn_projectile(game_data_t* gd, projectile_t* projectile);
void update_projectiles(game_data_t* gd);
void update_tiles(game_data_t* gd);
void update_powerups(game_data_t* gd);
void update_worms(game_data_t* gd);
void update_particle_emitters(game_data_t* gd, float delta);

void spawn_turret(game_data_t* gd, gs_vec2 pos, turret_type_t type);
void update_turrets(game_data_t* gd, float delta);

void spawn_orb(game_data_t* gd, gs_vec2 pos);
void update_orbs(game_data_t* gd, float delta);

void spawn_worm(game_data_t* gd, worm_type_t type, int segments, gs_vec2 pos, float radius);
void delete_worm(entity_t* worm);

void spawn_crystals(game_data_t* gd, gs_vec2 pos, int amount);
void update_crystals(game_data_t* gd, float delta);

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
void hit_entity(game_data_t* gd, entity_t* entity, gs_vec2 hit_pos, gs_vec2 knock_dir);

void next_wave(game_data_t* gd);

int sound_counter = 0;
void init() 
{
	game_data_t* gd = gs_engine_user_data(game_data_t);
	
	hit_sound_hndl = gs_audio_load_from_file("./assets/Hit_Hurt2.wav");

	gs_audio_t* audio = gs_engine_subsystem(audio);
    gs_audio_instance_decl_t decl = gs_default_val();
    decl.src = hit_sound_hndl;
    decl.volume = gs_clamp(0.5, audio->min_audio_volume, audio->max_audio_volume);
    decl.persistent = false;
    gs_handle(gs_audio_instance_t) hit_sound_instance_hndl = gs_audio_instance_create(&decl);
   // gs_audio_play(hit_sound_instance_hndl);

	gs_vec2 ws = window_size();

	// Initialize Game Data
	gd->shop_ui.upgrade_buttons = NULL;
	gd->shop_ui.visible = true;
	
	button_t button = {
		.position = gs_v2(200, 200),
		.size = gs_v2(100, 75),
		.type = BUTTON_TYPE_UPGRADE_DMG
	};
	gs_dyn_array_push(gd->shop_ui.upgrade_buttons, button);
	gs_dyn_array_push(gd->shop_ui.upgrade_buttons, button);
	gs_dyn_array_push(gd->shop_ui.upgrade_buttons, button);
	
	gd->projecitles = NULL;
	gd->worms = NULL;
	gd->powerups = NULL;
	gd->particle_emitters = NULL;
	gd->powerup_spawn_time = 0.f;
	gd->powerup_spawn_timer = 10.f;

	gd->worm_spawn_timer = 10.f;
	gd->worm_spawn_time = 0.f;

	gd->turret_spawn_timer = 10.f;
	
	gd->orb_spawn_timer = 15.f;

	gd->enemies = NULL;
	gs_dyn_array_push(gd->enemies, &gd->worms);
	gs_dyn_array_push(gd->enemies, &gd->turrets);
	gs_dyn_array_push(gd->enemies, &gd->orbs);
		
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			gd->tiles[x][y].destruction_value = 0;
			gd->tiles[x][y].noise_value = 0;
			gd->tiles[x][y].value = 0;
		}
	}

	
	next_wave(gd);
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


	if (gs_platform_key_pressed(GS_KEYCODE_ENTER)) {
		sound_counter += 1;
		printf("sound_counter: %i\n", sound_counter);
		//gs_audio_play_source(hit_sound_hndl, 0.5f);
		gs_audio_play(hit_sound_instance_hndl);
	}
	

	gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
	game_data_t* gd = gs_engine_user_data(game_data_t);
	float delta = gs_platform_delta_time();

	gd->shake_time -= delta;

	int enemy_count = 0;
	enemy_count += gd->worms_left_to_spawn + gd->turrets_left_to_spawn + gd->orbs_left_to_spawn;
	if (enemy_count == 0) {
		next_wave(gd);
	}
	
	update_player(gd);
	update_powerups(gd);
	update_worms(gd);
	update_turrets(gd, delta);
	update_orbs(gd, delta);
	update_projectiles(gd);
	update_crystals(gd, delta);
	
	update_tiles(gd);
	update_particle_emitters(gd, delta);

	/*
	Would be better to have all entities in a update loop so that i dont have to copy code
	such as flash timer, check if dead
	men då måst man ha switch på varje entity men det kanske inte spelar något roll heller,

	for e in entities:
		e->flash--
		if dead:
			if player:
				game over
			else
				delete
		switch entity->type
			player:
				do stuff
			worm:
				do worm stuff

		mer jobb än vad man får ut av det just nu så skit i de
	*/
	
	draw_game(gd);

	t += delta;
	if (t > 0.5) {
		t = 0.0;
		//printf("FPS: %f\n", 1.0/delta);
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
	p->hp = PLAYER_MAX_HP;

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

	p->flash -= 5*delta;
	if (p->flash < 0)
		p->flash = 0.f;

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


	p->turret_shoot_time += delta;
	if (gs_platform_mouse_down(GS_MOUSE_LBUTTON) && p->player_shoot_time >= PLAYER_SHOOT_TIMER) {
		p->turret_shoot_time = 0.f;
		gs_vec2 dir = gs_vec2_sub(get_world_mouse_pos(), *pos);
		dir = gs_vec2_norm(dir);
		
		gs_vec2 vel = gs_vec2_scale(dir, 300.f);

		
		
		spawn_projectile(gd, &(projectile_t){
			.position = *pos,
			.velocity = vel,
			.radius = 5,
			.accell = 200,
			.max_life_time = 1,
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
	//printf("spawn projectile\n");
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
		if (!p->go_through_walls) {
			if (is_tile_solid(gd ,tile_x, tile_y)) {
				should_delete = true;
				explode_tiles(gd, tile_x, tile_y, 5);
				if (!p->enemy_created)
					gd->shake_time = 0.05;
			}
		}

		p->life_time += delta;
		if (p->life_time >= p->max_life_time) {
			should_delete = true;
		}

		if (p->enemy_created) {
			entity_t* player = &gd->player;
			if (is_colliding(*pos, player->position, p->radius, player->radius)) {
				should_delete = true;
				gd->shake_time = 0.05;
				explode_tiles(gd, tile_x, tile_y, 5);
				gs_vec2 projectile_dir = gs_vec2_norm(p->velocity);
				hit_entity(gd, player, *pos, projectile_dir);
			}
		} else {
			for (int enemy_list_i = 0; enemy_list_i < gs_dyn_array_size(gd->enemies); enemy_list_i++) {
				entity_t** enemy_list = *gd->enemies[enemy_list_i];
				
				int list_size = gs_dyn_array_size(enemy_list);
				for (int j = 0; j < list_size; j++) {
					entity_t* enemy = enemy_list[j];
					if (enemy->dead)
						continue;
					if (is_colliding(*pos, enemy->position, p->radius, enemy->radius)) {
						should_delete = true;
						gd->shake_time = 0.05;
						explode_tiles(gd, tile_x, tile_y, 5);
						gs_vec2 projectile_dir = gs_vec2_norm(p->velocity);
						hit_entity(gd, enemy, *pos, projectile_dir);

						break;
					}
				}
			}
		} // end spawned by player

		// skulle kunna bara swappa plats med sista elementet och sen pop
		// behöver vända på loopen och ha if delete: i-- p_size--
		if (should_delete) {
			/*
			spawn_particle_emitter(gd, &(particle_emitter_desc_t){
				.particle_amount 	= 8,
				.particle_color 	= p->color,//gs_v4(0.4, 0.1, 0.1, 1.0),
				.particle_lifetime 	= 0.75f,
				.particle_shink_out = true,
				.particle_size 		= gs_v2(12,12),
				.particle_velocity 	= particle_vel,
				.position 			= hit_pos,
				.explode = true,
				.rand_rotation_range = 1.0,
				.rand_velocity_range = 0.5,
				.one_shot = true
			});*/


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

void spawn_worm(game_data_t* gd, worm_type_t type, int segments, gs_vec2 pos, float radius)
{
	gs_vec4 color = gs_v4(0.7, 0.2, 0.2, 1.0);
	if (type == WORM_TYPE_SINUS) {
		color = gs_v4(0.1, 0.4, 0.2, 1.0);

	}

	entity_t* head = malloc(sizeof(entity_t));
	*head = (entity_t){
		.hp = 3,
		.position = pos,
		.radius = radius,
		.color = color,
		.worm_type = type
	};
	gs_vec4 particle_color = color;
	particle_color.w = 0.5f;

	head->worm_particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
		.particle_amount 	= 10,
		.particle_color 	= particle_color,
		.particle_lifetime 	= 1.0f,
		.particle_shink_out = true,
		.particle_size 		= gs_v2(radius,radius),
		.particle_velocity 	= gs_v2(20.f, 0.f),
		.position 			= pos,
		.explode = false,
		.rand_rotation_range = 3.14*2,
		.rand_velocity_range = 0.5
	});
	
	entity_t* parent = head;
	for (int i = 0; i < segments; i++) {
		entity_t* segment = malloc(sizeof(entity_t));
		segment->hp = 1;
		segment->position = pos;
		float percent = (i+1.0) / (segments+1.0);
		segment->radius = head->radius * (1-percent);
		segment->worm_particle_emitter = NULL;
		segment->worm_segment = NULL;

		parent->worm_segment = segment;
		parent = segment;
	}
	gs_dyn_array_push(gd->worms, head);
}

void update_worms(game_data_t* gd)
{
	float delta = gs_platform_delta_time();
	gs_vec2 ws = window_size();
	
	if (gd->worms_left_to_spawn > 0) {
		gd->worm_spawn_time -= delta;
	}
	if (gd->worm_spawn_time <= 0) {
		gd->worm_spawn_time = gd->worm_spawn_timer * (randf() + 0.5);
		gs_vec2 spawn_pos;
		spawn_pos.x = RESOLUTION_X * randf();
		spawn_pos.y = 0;
		if (randf() > 0.5) {
			spawn_pos.y = RESOLUTION_Y;
		}
		worm_type_t worm_type = WORM_TYPE_NORMAL;
		printf("%f\n",randf());
		if (randf() > 0.5)
			worm_type = WORM_TYPE_SINUS;
		spawn_worm(gd, worm_type, 4, spawn_pos, 6);
		gd->worms_left_to_spawn--;
	}
	

	int worms_size = gs_dyn_array_size(gd->worms);
	for (int i = 0; i < worms_size; i++) {
		entity_t* head = gd->worms[i];

		head->time_alive += delta;

		head->flash -= 5*delta;
		if (head->flash < 0)
			head->flash = 0.f;

		// .5 secounds delay after dead for death animation
		if (head->dead) {
			head->dead_timer += delta;
			head->worm_particle_emitter->emitting = false;
			if (head->dead_timer > 0.5f) {
				spawn_crystals(gd, head->position, 5);

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
			if (head->worm_type == WORM_TYPE_SINUS) {
				gs_vec2 diff = gs_vec2_sub(target_pos, head->position);
				float diff_length = gs_vec2_len(diff);
				
				float angle = atan2f(target_pos.y - head->position.y, target_pos.x - head->position.x); //  gs_vec2_angle(gs_vec2_norm(diff), gs_v2(1, 0));
				
				float rotation = diff_length / 50;
				if (rotation > 1.5)
					rotation = 1.5;
				target_pos.x = head->position.x + cos(angle+sin(head->time_alive*2) * rotation) * diff_length;
				target_pos.y = head->position.y + sin(angle+sin(head->time_alive*2) * rotation) * diff_length;
			}

			if (is_tile_solid(gd, x, y)) {
				head->velocity = steer(head->position, target_pos, head->velocity, WORM_MAX_VELOCITY*2.f, WORM_MAX_FORCE*2.f, WORM_MAX_SPEED*2.f);
			} else {
				head->velocity = steer(head->position, target_pos, head->velocity, WORM_MAX_VELOCITY, WORM_MAX_FORCE, WORM_MAX_SPEED);
			}

			entity_t* player = &gd->player;
			if (is_colliding(head->position, player->position, head->radius, player->radius)) {
				gd->shake_time = 0.05;
				hit_entity(gd, player, head->position, gs_vec2_norm(head->velocity));
				
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
		//spawn_powerup(gd, POWERUP_ATTACK_RATE, spawn_pos);
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

	return p;
}

void spawn_turret(game_data_t* gd, gs_vec2 pos, turret_type_t type)
{
	entity_t* turret = malloc(sizeof(entity_t));
	*turret = (entity_t) {
		.position = pos,
		.radius = 9,
		.hp = 3,
		.turret_shoot_delay = 2.f,
		.color = gs_v4(0.4, 0.3, 0.3, 1.0),
		.type = ENTITY_TYPE_TURRET,
		.turret_type = type,
	};
	
	gs_dyn_array_push(gd->turrets, turret);
}
void update_turrets(game_data_t* gd, float delta)
{
	if (gd->turrets_left_to_spawn > 0) {
		gd->turret_spawn_time -= delta;
	}
	if (gd->turret_spawn_time <= 0) {
		gd->turret_spawn_time = gd->turret_spawn_timer * (randf() + 0.5);
		gs_vec2 spawn_pos;
		spawn_pos.x = (RESOLUTION_X-20) * randf() + 20;
		spawn_pos.y = (RESOLUTION_Y-20) * randf() + 20;
		turret_type_t type = TURRET_TYPE_NORMAL;
		if (randf() >= 1.0) {
			type = TURRET_TYPE_WORM_SPAWN; // gör om gör rätt eller nått
			
		}
		spawn_turret(gd, spawn_pos, type);
		gd->turret_spawn_time--;
	}

	int turrets_size = gs_dyn_array_size(gd->turrets);
	for (int i = 0; i < turrets_size; i++) {
		entity_t* t = gd->turrets[i];
		t->flash -= 5*delta;
		if (t->flash < 0)
			t->flash = 0.f;
		t->turret_time_since_spawn += delta;
		
		if (t->dead) {
			spawn_crystals(gd, t->position, 7);
			spawn_particle_emitter(gd, &(particle_emitter_desc_t){
				.explode = true,
				.one_shot = true,
				.particle_amount = 12,
				.particle_color = gs_v4(0.4, 0.3, 0.3, 1.0),
				.particle_lifetime = 0.5,
				.particle_shink_out = true,
				.particle_size = gs_v2(10, 10),
				.particle_velocity = gs_v2(50, 0),
				.position = t->position,
				.rand_rotation_range = 2 * 3.14,
				.rand_velocity_range = 0.5
			});
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
				if (t->turret_type == TURRET_TYPE_NORMAL) {
					spawn_projectile(gd, &(projectile_t){
						.enemy_created = true,
						.position = t->position,
						.velocity = projectile_vel,
						.radius = 4,
						.accell = 100,
						.max_life_time = 1.5,
						.go_through_walls = true,
						.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
							.particle_amount = 6,
							.particle_color = gs_v4(0.3, 0.1, 0.1, 1.0),
							.particle_lifetime = 0.15,
							.particle_shink_out = true,
							.particle_size = gs_v2(8, 8),
							.position = t->position
							
						})
					});
				} else if (t->turret_type == TURRET_TYPE_WORM_SPAWN) {
					spawn_worm(gd, WORM_TYPE_NORMAL, 2, t->position, 4);
				}

				t->turret_shot_count++;
				if (t->turret_shot_count >= TURRET_BURST_COUNT) {
					t->turret_shot_count = 0;
					t->turret_shooting = false;
				}
			}
		}
		

	}
}

void spawn_orb(game_data_t* gd, orb_type_t orb_type, gs_vec2 pos)
{
	gs_vec2 rand_vel;
	float rand_angle = 2 * 3.14 * randf();
	rand_vel.x = cos(rand_angle) * 75;
	rand_vel.y = sin(rand_angle) * 75;


	gs_vec4 color;
	switch (orb_type) {
	case ORB_TYPE_PINK:
		color = gs_v4(0.8, 0.2, 0.8, 1.0);
		break;
	case ORB_TYPE_BLUE:
		color = gs_v4(0.2, 0.2, 0.8, 1.0);
	default:
		break;
	}

	entity_t* orb = malloc(sizeof(entity_t));
	*orb = (entity_t){
		.hp = 5,
		.radius = 12,
		.velocity = rand_vel,
		.position = pos,
		.color = color,
		.type = ENTITY_TYPE_ORB,
		.orb_type = orb_type
	};
	orb->target_speed = 100;
	color.w = 0.5;
	orb->orb_particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
		.particle_amount 	= 10,
		.particle_color 	= color,
		.particle_lifetime 	= 0.25f,
		.particle_shink_out = true,
		.particle_size 		= gs_v2(12,12),
		.particle_velocity 	= gs_v2(20.f, 0.f),
		.position 			= pos,
		.rand_rotation_range = 3.14*2,
		.rand_velocity_range = 0.5
	});
	gs_dyn_array_push(gd->orbs, orb);
}



void update_orbs(game_data_t* gd, float delta)
{
	if (gd->orbs_left_to_spawn > 0) {
		gd->orb_spawn_time -= delta;
	}
	if (gd->orb_spawn_time <= 0) {
		gd->orb_spawn_time = gd->orb_spawn_timer * (randf() + 0.5);
		gs_vec2 spawn_pos;
		spawn_pos.x = (RESOLUTION_X-20) * randf() + 20;
		spawn_pos.y = (RESOLUTION_Y-20) * randf() + 20;
		int orb_type = stb_rand() % ORB_TYPE_SIZE;
		spawn_orb(gd, orb_type, spawn_pos);
		gd->orbs_left_to_spawn--;
	}

	int orbs_size = gs_dyn_array_size(gd->orbs);
	for (int i = 0; i < orbs_size; i++) {
		entity_t* o = gd->orbs[i];
		o->flash -= 5*delta;
		if (o->flash < 0)
			o->flash = 0.f;
		
		if (o->dead) {
			spawn_crystals(gd, o->position, 5);
			spawn_particle_emitter(gd, &(particle_emitter_desc_t){
				.explode = true,
				.one_shot = true,
				.particle_amount = 12,
				.particle_color = o->color,
				.particle_lifetime = 0.5,
				.particle_shink_out = true,
				.particle_size = gs_v2(10, 10),
				.particle_velocity = gs_v2(50, 0),
				.position = o->position,
				.rand_rotation_range = 2 * 3.14,
				.rand_velocity_range = 0.5
			});
			o->orb_particle_emitter->should_delete = true;
			free(o);
			remove_entity(gd->orbs, &orbs_size, &i);
			//printf("dead");
			continue;
		}

		if (o->velocity.x < 0 && o->position.x < 0) {
			o->velocity.x = -o->velocity.x;
		}
		if (o->velocity.x > 0 && o->position.x > RESOLUTION_X) {
			o->velocity.x = -o->velocity.x;
		}
		if (o->velocity.y < 0 && o->position.y < 0) {
			o->velocity.y = -o->velocity.y;
		}
		if (o->velocity.y > 0 && o->position.y > RESOLUTION_Y) {
			o->velocity.y = -o->velocity.y;
		}


		float current_speed = gs_vec2_len(o->velocity);
		float speed_diff = o->target_speed - current_speed;
		
		gs_vec2 dir = gs_vec2_norm(o->velocity);


		o->velocity.x += dir.x * speed_diff * delta;
		o->velocity.y += dir.y * speed_diff * delta;



		o->position.x += o->velocity.x * delta;
		o->position.y += o->velocity.y * delta;
		o->orb_particle_emitter->position = o->position;
		
	}
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


void hit_entity(game_data_t* gd, entity_t* entity, gs_vec2 hit_pos, gs_vec2 knock_dir)
{
	if (entity->flash > 0)
		return;

	entity->hp -= 1;
	
	if (entity->hp <= 0) {
		entity->dead = true;
	}
	entity->flash = 1.0;
	if (entity->type = ENTITY_TYPE_ORB && entity->orb_type == ORB_TYPE_BLUE) {
		float speed = 200;
		// 	var target = player.global_position + player.velocity * (global_position.distance_to(player.global_position)/projectile_speed)
		gs_vec2 predict_pos = gs_vec2_add(gd->player.position, gs_vec2_scale(gd->player.velocity, gs_vec2_dist(entity->position, gd->player.position)/speed));
		gs_vec2 dist = gs_vec2_sub(predict_pos, entity->position);

		gs_vec2 charge_dir = gs_vec2_norm(dist);
		entity->velocity = gs_vec2_scale(charge_dir, speed);
	} else {
		entity->velocity = gs_vec2_add(entity->velocity, gs_vec2_scale(knock_dir, 200));
	}
	
	gs_vec2 particle_vel = knock_dir;
	particle_vel.x *= 75;
	particle_vel.y *= 75;
	spawn_particle_emitter(gd, &(particle_emitter_desc_t){ 
		
		.particle_amount 	= 8,
		.particle_color 	= entity->color,
		.particle_lifetime 	= 0.75f,
		.particle_shink_out = true,
		.particle_size 		= gs_v2(12,12),
		.particle_velocity 	= particle_vel,
		.position 			= hit_pos,
		.explode = true,
		.rand_rotation_range = 1.0,
		.rand_velocity_range = 0.5,
		.one_shot = true
	});
}

void spawn_crystals(game_data_t* gd, gs_vec2 pos, int amount)
{
	for (int i = 0; i < amount; i++) {
		gs_vec2 rand_vel = gs_v2(randf()-0.5, randf()-0.5);
		rand_vel = gs_vec2_norm(rand_vel);
		rand_vel.x *= 400;
		rand_vel.y *= 400;
		crystal_t crystal = {
			.position = pos,
			.radius = 2,
			.velocity = rand_vel,
			.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
				.particle_amount = 6,
				.particle_color = gs_v4(0.2, 0.5, 0.9, 1.0),
				.particle_lifetime = 0.1f,
				.particle_shink_out = true,
				.particle_size = gs_v2(4, 4),
				.particle_velocity = gs_v2(20, 0),
				.position = pos,
				.rand_rotation_range = 2 * 3.14,
				.rand_velocity_range = 0.5
			}),
		};
		gs_dyn_array_push(gd->crystals, crystal);
	}
}

void update_crystals(game_data_t* gd, float delta)
{
	int crystals_size = gs_dyn_array_size(gd->crystals);
	for (int i = 0; i < crystals_size; i++) {
		crystal_t* c = &gd->crystals[i];
		gs_vec2 target_pos = gd->player.position;
		if (gs_vec2_dist(c->position, target_pos) < c->radius) {
			c->particle_emitter->should_delete = true;
			gd->crystals[i] = gd->crystals[crystals_size-1];
			i--;
			crystals_size--;
			gs_dyn_array_pop(gd->crystals);
			gd->crystals_currency++;
			continue;
		}

		c->time_alive += delta;

		c->velocity = steer(c->position, target_pos, c->velocity, 200, 5 * (c->time_alive*3+1), 200);
		c->position.x += c->velocity.x * delta;
		c->position.y += c->velocity.y * delta;

		c->particle_emitter->position = c->position;
	}
}


void next_wave(game_data_t* gd)
{
	gd->wave++;

	gd->orbs_left_to_spawn = gd->wave;
	gd->worms_left_to_spawn = 2 * gd->wave;
	gd->turrets_left_to_spawn = 0.5 * gd->wave;

	printf("next wave! wave: %i\n", gd->wave);
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
	return stb_frand();
	//return (float)rand()/(double)(RAND_MAX);
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
		.window_flags = GS_WINDOW_FLAGS_FULLSCREEN,
		.enable_vsync = true
	};
}
