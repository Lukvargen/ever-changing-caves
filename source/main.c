#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>


#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "main.h"
#include "graphics.h"



// Forward Declares
void spawn_player(game_data_t* gd);
void update_player(game_data_t* gd, float delta);
void spawn_projectile(game_data_t* gd, projectile_t* projectile);
void update_projectiles(game_data_t* gd, float delta);
void update_tiles(game_data_t* gd, float delta);
void update_powerups(game_data_t* gd, float delta);
void update_worms(game_data_t* gd, float delta);
void update_particle_emitters(game_data_t* gd, float delta);

void spawn_laser(game_data_t* gd, int dmg, gs_vec2 pos, float radius, gs_color_t color, int max_targets);
void update_lasers(game_data_t* gd, float delta);

void spawn_turret(game_data_t* gd, gs_vec2 pos, turret_type_t type);
void update_turrets(game_data_t* gd, float delta);

void spawn_orb(game_data_t* gd, orb_type_t orb_type, gs_vec2 pos);
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
gs_vec2 gs_vec2_truncate(gs_vec2 v, float max_length);
bool is_colliding(gs_vec2 p1, gs_vec2 p2, float r1, float r2);
gs_vec2 get_world_mouse_pos();
gs_vec2 steer(gs_vec2 from_pos, gs_vec2 target_pos, gs_vec2 velocity, float max_velocity, float max_force, float max_speed);
float randf();
void remove_entity(entity_t* arr[], int* arr_length, int* i);
void entity_take_dmg(game_data_t* gd, entity_t* entity, int dmg, gs_vec2 hit_pos, gs_vec2 knock_dir);


void update_wave_system(game_data_t* gd, float delta);
void next_wave(game_data_t* gd);



void init() 
{
	game_data_t* gd = gs_engine_user_data(game_data_t);

	gd->hit_sound_hndl = gs_audio_load_from_file("./assets/Hit_Hurt2.wav");
	gd->crystal_pickup_sound_hndl = gs_audio_load_from_file("./assets/pickup.wav");
	gd->hit_wall_sound_hndl = gs_audio_load_from_file("./assets/HitWall.wav");
	gd->buy_positive_sound_hndl = gs_audio_load_from_file("./assets/BuyComplete.wav");
	gd->buy_negative_sound_hndl = gs_audio_load_from_file("./assets/BuyNegative.wav");

	// Initialize Game Data
	gd->volume = 0.5;

	gd->projecitles = NULL;
	gd->worms = NULL;
	gd->powerups = NULL;
	gd->particle_emitters = NULL;

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

	
	gd->wave = 0;
	gd->enemies_to_spawn = NULL;

	gd->shop.all_upgrades = NULL;

	next_wave(gd);
	graphics_init(gd);
	spawn_player(gd);
	gd->crystals_currency = 0;
	shop_init_all_upgrades(gd);
	//shop_show(gd);
}


/*=================
// Update Functions
=================*/
float t = 0.0;
void update()
{
	gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
	game_data_t* gd = gs_engine_user_data(game_data_t);
	float delta = gs_platform_delta_time();

	if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
		gs_engine_quit();
	} else if (gs_platform_key_pressed(GS_KEYCODE_R)) {
		restart_game(gd);
		return;
	}
	if (gs_platform_key_pressed(GS_KEYCODE_ENTER)) {
		gs_audio_play_source(gd->hit_sound_hndl, 0.5f);
	}
	if (gs_platform_key_pressed(GS_KEYCODE_P)) {
		gd->paused = !gd->paused;
	}
	if (gs_platform_key_pressed(GS_KEYCODE_M)) {
		gd->mute = !gd->mute;
		if (gd->mute)
			gd->volume = 0.f;
		else
			gd->volume = 0.5f;
	}

	gd->shake_time -= delta;

	if (gd->restart) {
		restart_game(gd);
		return;
	}
	

	if (!gd->paused) {
		gd->time += delta;
		update_wave_system(gd, delta);
		update_tiles(gd, delta);
		update_player(gd, delta);
		update_powerups(gd, delta);
		update_worms(gd, delta);
		update_turrets(gd, delta);
		update_orbs(gd, delta);
		update_projectiles(gd, delta);
		update_lasers(gd, delta);
		update_crystals(gd, delta);
		update_particle_emitters(gd, delta);
	}

	
	draw_game(gd);

	t += delta;
	if (t > 0.5) {
		t = 0.0;
		//printf("FPS: %f\n", 1.0/delta);
	}
	
}

void restart_game(game_data_t* gd)
{
	gs_dyn_array_clear(gd->enemies_to_spawn);
	gs_dyn_array_clear(gd->lasers);
	gs_dyn_array_clear(gd->projecitles);
	gs_dyn_array_clear(gd->crystals);

	for (int enemy_list_i = 0; enemy_list_i < gs_dyn_array_size(gd->enemies); enemy_list_i++) {
		entity_t** enemy_list = *gd->enemies[enemy_list_i];
		int list_size = gs_dyn_array_size(enemy_list);
		for (int j = 0; j < list_size; j++) {
			entity_t* enemy = enemy_list[j];
			free(enemy);
		}
		gs_dyn_array_clear(enemy_list);
	}
	for (int i = 0; i < gs_dyn_array_size(gd->particle_emitters); i++) {
		free(gd->particle_emitters[i]);
	}
	gs_dyn_array_clear(gd->particle_emitters);


	gd->time = 0.f;
	gd->game_over = false;
	gd->restart = false;
	gd->paused = false;
	gd->shop.visible = false;
	gd->spawn_timer = 0.f;
	
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			gd->tiles[x][y].destruction_value = 0;
			gd->tiles[x][y].noise_value = 0;
			gd->tiles[x][y].value = 0;
		}
	}
	gs_dyn_array_clear(gd->shop.all_upgrades);
	shop_init_all_upgrades(gd);
	spawn_player(gd);
	gd->crystals_currency = 0;
	gd->wave = 0;
	next_wave(gd);

}

void update_tiles(game_data_t* gd, float delta)
{
	for (int x = 0; x < TILES_SIZE_X; x++) {
		for (int y = 0; y < TILES_SIZE_Y; y++) {
			tile_t* tile = &gd->tiles[x][y];
			tile->noise_value = (stb_perlin_noise3(x*0.1, y*0.1, gd->time * 0.15, 0, 0, 0) + 1) / 2.f;
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
	*p = (entity_t){0};
	p->position.x = RESOLUTION_X / 2;
	p->position.y = RESOLUTION_Y / 2;
	p->radius = 4.f;
	p->max_hp = 10;
	p->hp = p->max_hp;
	p->dmg = 1;
	p->player_projectile_lifetime = PLAYER_BASE_PROJECTILE_LIFETIME;
	p->player_projectile_speed = PLAYER_BASE_PROJECITLE_SPEED;
	p->player_projectile_accel = PLAYER_BASE_PROJECTILE_ACCELL;
	p->player_explosion_radius = PLAYER_BASE_EXPLOSION_RADIUS;
	p->player_shoot_delay = PLAYER_BASE_SHOOT_DELAY;
	p->player_projectile_reflect_chance = 0.f;
	p->player_projectile_reflect_amount = 1;
	gd->player.player_laser_lvl = 0;


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
}

void update_player(game_data_t* gd, float delta)
{
	entity_t* p = &gd->player;
	gs_vec2* pos = &gd->player.position;
	gs_vec2* vel = &gd->player.velocity;

	if (p->dead) {
		gd->game_over = true;
		gd->paused = true;
		
	}

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

	// handle collision with walls
	int tile_x = pos->x/TILE_SIZE;
	int tile_y = pos->y/TILE_SIZE;
	if (is_tile_solid(gd, tile_x, tile_y)) {
		entity_take_dmg(gd, p, p->max_hp * 0.1, p->position, gs_v2(1,0)); //gs_v2(randf(), randf()));
		
		explode_tiles(gd, tile_x, tile_y, 3);
	} else {
		pos->x += vel->x * delta;
		pos->y += vel->y * delta;

		int center_y = pos->y / TILE_SIZE;
		
		int left_tile_pos = (int)floor((pos->x - p->radius)/TILE_SIZE);
		int right_tile_pos = (int)floor((pos->x + p->radius)/TILE_SIZE);

		
		if (is_tile_solid(gd, right_tile_pos, center_y)) {
			pos->x = right_tile_pos*TILE_SIZE - p->radius;
			p->velocity.x = 0;
		} else if (is_tile_solid(gd, left_tile_pos, center_y)) {
			pos->x = left_tile_pos*TILE_SIZE + TILE_SIZE + p->radius;
			p->velocity.x = 0;
		}
		int center_x = pos->x / TILE_SIZE;
		int up_tile_pos = (int)floor((pos->y - p->radius)/TILE_SIZE);
		int down_tile_pos = (int)floor((pos->y + p->radius)/TILE_SIZE);
		if (is_tile_solid(gd, center_x, down_tile_pos)) {
			pos->y = down_tile_pos*TILE_SIZE - p->radius;
			p->velocity.y = 0;
		} else if (is_tile_solid(gd, center_x, up_tile_pos)) {
			pos->y = up_tile_pos*TILE_SIZE + TILE_SIZE + p->radius;
			p->velocity.y = 0;
		}
	}

	p->player_particle_emitter->position = p->position;
	

	p->player_shoot_time += delta;
	if (gs_platform_mouse_down(GS_MOUSE_LBUTTON) && p->player_shoot_time >= p->player_shoot_delay) {
		p->player_shoot_time = 0.f;
		gs_vec2 dir = gs_vec2_sub(get_world_mouse_pos(), *pos);
		dir = gs_vec2_norm(dir);
		
		gs_vec2 vel = gs_vec2_scale(dir, p->player_projectile_speed);

		
		
		spawn_projectile(gd, &(projectile_t){
			.position = *pos,
			.velocity = vel,
			.radius = 5,
			.accell = p->player_projectile_accel,
			.max_life_time = p->player_projectile_lifetime,
			.dmg = p->dmg,
			.explode_radius = p->player_explosion_radius,
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

void spawn_laser(game_data_t* gd, int dmg, gs_vec2 pos, float radius, gs_color_t color, int max_targets)
{
	laser_t laser = {
		.color = color,
		.max_targets = max_targets,
		.max_time_alive = 1.f,
		.points = NULL
	};
	gs_dyn_array(entity_t*) targets;
	targets = NULL;
	while(true) {
		
		bool added_enemy = false;
		bool should_break = false;
		for (int enemy_list_i = 0; enemy_list_i < gs_dyn_array_size(gd->enemies); enemy_list_i++) {
			entity_t** enemy_list = *gd->enemies[enemy_list_i];
			int list_size = gs_dyn_array_size(enemy_list);
			for (int j = 0; j < list_size; j++) {
				entity_t* enemy = enemy_list[j];
				// check if already collided with enemy
				bool already_added = false;
				for (int i = 0; i < gs_dyn_array_size(targets); i++) {
					if (targets[i] == enemy) {
						already_added = true;
						break;
					}
				}
				if (is_colliding(pos, enemy->position, radius, enemy->radius) && !already_added) {
					gs_dyn_array_push(targets, enemy);
					
					added_enemy = true;
					pos = enemy->position;
					should_break = true;
					break;
				}

			}
			if (should_break)
				break;
		}
		if (!added_enemy || gs_dyn_array_size(targets) >= max_targets)
			break;
	}
	int targets_size = gs_dyn_array_size(targets); 
	if (targets_size > 1) {
		entity_take_dmg(gd, targets[targets_size-1], dmg, targets[targets_size-1]->position, gs_v2(randf()*2-1, randf()*2-1));
		for (int i = 0; i < targets_size-1; i++) {
			entity_t* enemy1 = targets[i];
			entity_t* enemy2 = targets[i+1];
			entity_take_dmg(gd, enemy1, dmg, enemy1->position, gs_v2(randf()*2-1, randf()*2-1));
			gs_dyn_array_push(laser.points, enemy1->position);

			gs_vec2 pos = enemy1->position;

			for (int s = 0; s < 4; s++) {
				gs_vec2 diff = gs_vec2_sub(enemy2->position, pos);
				float dist = gs_vec2_len(diff);

				float angle = atan2f(diff.y, diff.x);
				diff.x = cos(angle + (randf()-0.5)*3.14) * dist * (s/4.f);
				diff.y = sin(angle + (randf()-0.5)*3.14) * dist * (s/4.f);
				pos.x = pos.x + diff.x;
				pos.y = pos.y + diff.y;
				gs_dyn_array_push(laser.points, pos);

			}
			
			gs_dyn_array_push(laser.points, enemy2->position);

		}
		gs_dyn_array_push(gd->lasers, laser);
	}
	gs_dyn_array_free(targets);
}
void update_lasers(game_data_t* gd, float delta)
{
	int l_size = gs_dyn_array_size(gd->lasers);

	for (int i = 0; i < l_size; i++) {
		laser_t* l = &gd->lasers[i];
		l->time_alive += delta;
		if (l->time_alive >= l->max_time_alive) {
			gs_dyn_array_free(l->points);
			gd->lasers[i] = gd->lasers[l_size-1];
			gs_dyn_array_pop(gd->lasers);
			i--;
			l_size--;
			continue;
		}
	}
}

void spawn_projectile(game_data_t* gd, projectile_t* projectile)
{
	gs_dyn_array_push(gd->projecitles, *projectile);
}

void update_projectiles(game_data_t* gd, float delta)
{
	int p_size = gs_dyn_array_size(gd->projecitles);

	for (int i = 0; i < p_size; i++) {
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
				explode_tiles(gd, tile_x, tile_y, p->explode_radius);
				if (!p->enemy_created) {
					gd->shake_time = 0.05;
					gs_audio_play_source(gd->hit_wall_sound_hndl, gd->volume * 0.6);
				}
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
				entity_take_dmg(gd, player, p->dmg, *pos, projectile_dir);
			}
		} else {
			for (int enemy_list_i = 0; enemy_list_i < gs_dyn_array_size(gd->enemies); enemy_list_i++) {
				entity_t** enemy_list = *gd->enemies[enemy_list_i];
				if (should_delete)
					break;
				int list_size = gs_dyn_array_size(enemy_list);
				for (int j = 0; j < list_size; j++) {
					entity_t* enemy = enemy_list[j];
					if (enemy->dead || p->entity_ignore == enemy)
						continue;
					if (is_colliding(*pos, enemy->position, p->radius, enemy->radius)) {
						if (p->entity_ignore != 0) {
						}
						should_delete = true;
						gd->shake_time = 0.05;
						explode_tiles(gd, tile_x, tile_y, 5);
						gs_vec2 projectile_dir = gs_vec2_norm(p->velocity);
						entity_take_dmg(gd, enemy,p->dmg, *pos, projectile_dir);
						
						spawn_laser(gd, 1, enemy->position, 128, gs_color(125, 91, 166, 255), gd->player.player_laser_lvl+1);

						if (randf() <= gd->player.player_projectile_reflect_chance) {
							p->entity_ignore = enemy;
							for (int i = 0; i < gd->player.player_projectile_reflect_amount; i++) {

								projectile_t new_projectile = *p;
								new_projectile.entity_ignore = enemy;
								new_projectile.life_time = 0.f;
								
								new_projectile.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
									.particle_amount = p->particle_emitter->particle_amount,
									.particle_color = p->particle_emitter->particle_color,
									.particle_lifetime = p->particle_emitter->particle_life_time,
									.particle_shink_out = p->particle_emitter->particle_shrink_out,
									.particle_size = p->particle_emitter->particle_size,
									.position = p->position
								});
								int vel_length = gs_vec2_len(p->velocity);
								
								float angle = atan2f(p->velocity.y, p->velocity.x);
								new_projectile.velocity.x = cos(angle + (randf()-0.5)*3.14) * vel_length;
								new_projectile.velocity.y = sin(angle + (randf()-0.5)*3.14) * vel_length;
								p_size++;
								spawn_projectile(gd, &new_projectile);
							}
							
						}
						break;
					}
				}
			}
		} // end spawned by player

		if (should_delete) {

			if (p->particle_emitter) {
				p->particle_emitter->should_delete = true;
			}
			
			gd->projecitles[i] = gd->projecitles[p_size-1];
			i--;
			p_size--;
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
	int hp = 3 * pow(1.2, gd->wave);
	int dmg = 1 * pow(1.15, gd->wave);
	*head = (entity_t){
		.hp = hp,
		.position = pos,
		.radius = radius,
		.color = color,
		.worm_type = type,
		.dmg = dmg,
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

void update_worms(game_data_t* gd, float delta)
{
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
				spawn_crystals(gd, head->position, 5 * pow(1.05, gd->wave-1));

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
				
				float angle = atan2f(target_pos.y - head->position.y, target_pos.x - head->position.x);
				
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
				entity_take_dmg(gd, player, head->dmg, head->position, gs_vec2_norm(head->velocity));
				
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

void update_powerups(game_data_t* gd, float delta) 
{
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
	int hp = 5 * pow(1.2, gd->wave);
	int dmg = 1.5 * pow(1.15, gd->wave);
	gs_vec4 color = gs_v4(1.0, 1.0, 1.0, 1.0);
	float turret_shot_delay = 2.f;
	float turret_burst_shoot_delay = TURRET_BURST_SHOT_DELAY;

	
	if (type == TURRET_TYPE_SPIN) {
		color.z = 1.25;
		turret_shot_delay = 0.f;
		turret_burst_shoot_delay = 0.1;
		hp = 8 * pow(1.2, gd->wave);
	}
	*turret = (entity_t) {
		.position = pos,
		.radius = 9,
		.hp = hp,
		.turret_shoot_delay = turret_shot_delay,
		.turret_burst_shoot_delay = turret_burst_shoot_delay,
		.color = color,
		.type = ENTITY_TYPE_TURRET,
		.turret_type = type,
		.dmg = dmg
	};
	
	gs_dyn_array_push(gd->turrets, turret);
}
void update_turrets(game_data_t* gd, float delta)
{

 
	int turrets_size = gs_dyn_array_size(gd->turrets);
	for (int i = 0; i < turrets_size; i++) {
		entity_t* t = gd->turrets[i];
		t->flash -= 5*delta;
		if (t->flash < 0)
			t->flash = 0.f;
		t->turret_time_since_spawn += delta;
		
		if (t->dead) {
			spawn_crystals(gd, t->position, 7 * pow(1.05, gd->wave-1));
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
			free(t);
			remove_entity(gd->turrets, &turrets_size, &i);
			continue;
		}

		gs_vec2 player_pos = gd->player.position;
		gs_vec2 target_dir;
		t->turret_angle = atan2f(player_pos.y - t->position.y, player_pos.x - t->position.x);
		if (t->turret_type == TURRET_TYPE_NORMAL) {
			target_dir = gs_vec2_sub(player_pos, t->position);
			target_dir = gs_vec2_norm(target_dir);
		} else if (t->turret_type == TURRET_TYPE_SPIN) {
			t->turret_angle = t->turret_time_since_spawn*3 + sin(t->turret_time_since_spawn)*0.5;
			target_dir = gs_v2(cos(t->turret_angle), sin(t->turret_angle));
		}

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

			if (t->turret_shoot_time >= t->turret_burst_shoot_delay) {
				t->turret_shoot_time = 0.f;
				// spawn projectile
				switch (t->turret_type) {
					gs_vec2 projectile_vel;
					case (TURRET_TYPE_NORMAL):
						projectile_vel = gs_vec2_scale(target_dir, 100);
						
						spawn_projectile(gd, &(projectile_t){
							.enemy_created = true,
							.position = t->position,
							.velocity = projectile_vel,
							.radius = 4,
							.accell = 100,
							.max_life_time = 1.5,
							.dmg = t->dmg,
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
						break;
					case (TURRET_TYPE_SPIN):
						projectile_vel = gs_vec2_scale(target_dir, 25);
						
						spawn_projectile(gd, &(projectile_t){
							.enemy_created = true,
							.position = t->position,
							.velocity = projectile_vel,
							.radius = 4,
							.accell = 100,
							.max_life_time = 1.5,
							.dmg = t->dmg,
							.go_through_walls = false,
							.explode_radius = 1,
							.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
								.particle_amount = 6,
								.particle_color = gs_v4(0.3, 0.4, 0.7, 1.0),
								.particle_lifetime = 0.15,
								.particle_shink_out = true,
								.particle_size = gs_v2(8, 8),
								.position = t->position
								
							})
						});
						break;

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
	int hp = 5 * pow(1.2, gd->wave);
	int dmg = 1 * pow(1.15, gd->wave);
	*orb = (entity_t){
		.hp = hp,
		.radius = 12,
		.dmg = dmg,
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

	int orbs_size = gs_dyn_array_size(gd->orbs);
	for (int i = 0; i < orbs_size; i++) {
		entity_t* o = gd->orbs[i];
		o->flash -= 5*delta;
		if (o->flash < 0)
			o->flash = 0.f;
		
		if (o->dead) {
			spawn_crystals(gd, o->position, 5 * pow(1.05, gd->wave-1));
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
			continue;
		}

		o->charge_timer -= delta;
		if (o->charge_timer <= 0) {
			o->charge_timer = randf() * 2 + 1;
			gs_vec2 dir = gs_vec2_sub(gd->player.position, o->position);
			dir = gs_vec2_norm(dir);
			o->velocity.x = dir.x * 300;
			o->velocity.y = dir.y * 300;

			if (o->orb_type == ORB_TYPE_BLUE) {
				int p_amount = 8;
				for (int i = 0; i < p_amount; i++) {
					gs_vec2 p_vel;
					float speed = 100;
					float rot = 2 * 3.14 / p_amount;
					p_vel.x = cos(rot/2.0 + rot*i) * speed;
					p_vel.y = sin(rot/2.0 + rot*i) * speed;
					spawn_projectile(gd, &(projectile_t){
						.position = o->position,
						.velocity = p_vel,
						.radius = 4,
						.accell = 300,
						.max_life_time = 0.4f,
						.dmg = o->dmg,
						.enemy_created = true,
						.go_through_walls = true,
						
						.particle_emitter = spawn_particle_emitter(gd, &(particle_emitter_desc_t){
							.particle_amount = 10,
							.particle_color = gs_v4(0.5, 0.5, 0.8, 1.0),
							.particle_lifetime = 0.4,
							.particle_shink_out = true,
							.particle_size = gs_v2(6, 6),
							.position = o->position
							
						})
					});
				}
			}

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
		
		entity_t* player = &gd->player;
		if (is_colliding(o->position, player->position, o->radius, player->radius)) {
			gd->shake_time = 0.05;
			entity_take_dmg(gd, player, o->dmg, o->position, gs_vec2_norm(o->velocity));
		}
		
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


void entity_take_dmg(game_data_t* gd, entity_t* entity, int dmg, gs_vec2 hit_pos, gs_vec2 knock_dir)
{

	if (entity->flash > 0 && entity->type == ENTITY_TYPE_PLAYER)
		return;
	gs_audio_play_source(gd->hit_sound_hndl, gd->volume);

	entity->hp -= dmg;
	
	if (entity->hp <= 0) {
		entity->dead = true;
	}
	entity->flash = 1.0;
	
	entity->velocity = gs_vec2_add(entity->velocity, gs_vec2_scale(knock_dir, 200));
	
	
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
			gs_audio_play_source(gd->crystal_pickup_sound_hndl, gd->volume * 0.6);
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


void update_wave_system(game_data_t* gd, float delta)
 {
	gd->spawn_timer -= delta;
	if (gd->spawn_timer <= 0) {
		int enemies_left_to_spawn = gs_dyn_array_size(gd->enemies_to_spawn);
		if (enemies_left_to_spawn > 0) {
			gd->spawn_timer = 5;
			int spawn_amount_random = 0;
			if (gd->wave >= 10)
				spawn_amount_random = 4;
			else if (gd->wave >= 5)
				spawn_amount_random = 3;
			else if (gd->wave >= 2)
				spawn_amount_random = 2;
			int spawn_amount = 1 + (int)(randf() * spawn_amount_random);
			if (spawn_amount > enemies_left_to_spawn)
				spawn_amount = enemies_left_to_spawn;
			
			for (int i = 0; i < spawn_amount; i++) {
				int index = (int)(randf()*enemies_left_to_spawn);
				entity_type_t type = gd->enemies_to_spawn[index];
				gd->enemies_to_spawn[index] = gd->enemies_to_spawn[enemies_left_to_spawn-1];
				enemies_left_to_spawn--;
				gs_dyn_array_pop(gd->enemies_to_spawn);
				gs_vec2 spawn_pos;
				switch (type) {
					case (ENTITY_TYPE_WORM):
						spawn_pos.x = RESOLUTION_X * randf();
						spawn_pos.y = 0;
						if (randf() > 0.5) {
							spawn_pos.y = RESOLUTION_Y;
						}
						worm_type_t worm_type = WORM_TYPE_NORMAL;
						if (randf() > 0.5)
							worm_type = WORM_TYPE_SINUS;
						spawn_worm(gd, worm_type, 4, spawn_pos, 6);
						break;
					case (ENTITY_TYPE_TURRET):
						spawn_pos.x = (RESOLUTION_X-20) * randf() + 20;
						spawn_pos.y = (RESOLUTION_Y-20) * randf() + 20;
						turret_type_t type = TURRET_TYPE_NORMAL;
						if (randf() >= 0.6) {
							type = TURRET_TYPE_SPIN;
							
						}
						spawn_turret(gd, spawn_pos, type);
						break;
					case (ENTITY_TYPE_ORB):
						if (randf() > 0.5) {
							if (randf() > 0.5) {
								spawn_pos.x = 20;
								spawn_pos.y = (RESOLUTION_Y-40) * randf() + 20;
							} else {
								spawn_pos.x = RESOLUTION_X - 20;
								spawn_pos.y = (RESOLUTION_Y-40) * randf() + 20;
							}
						} else {
							if (randf() > 0.5) {
								spawn_pos.y = 20;
								spawn_pos.x = (RESOLUTION_X-40) * randf() + 20;
							} else {
								spawn_pos.y = RESOLUTION_Y - 20;
								spawn_pos.x = (RESOLUTION_X-40) * randf() + 20;
							}
						}
						int orb_type = stb_rand() % ORB_TYPE_SIZE;
						spawn_orb(gd, orb_type, spawn_pos);
						break;	
				}
			}
		} else {
			int enemies_alive = 0;
			for (int i = 0; i < gs_dyn_array_size(gd->enemies); i++) {
				enemies_alive += gs_dyn_array_size(*gd->enemies[i]);
			}
			if (enemies_alive == 0) {
				gd->open_shop_timer += delta;
				if (gd->open_shop_timer > 3.f) {
					shop_show(gd);
				} 
			} else {
				gd->open_shop_timer = 0.f;
			}
		}

	}
 }

void append_enemies_to_spawn(game_data_t* gd, entity_type_t type, int amount) 
{
	for (int i = 0; i < amount; i++) {
		gs_dyn_array_push(gd->enemies_to_spawn, type);
	}
}

void next_wave(game_data_t* gd)
{
	gd->wave++;
	gd->open_shop_timer = 0.f;
	int worm_amount = 2 * pow(1.1, gd->wave);
	append_enemies_to_spawn(gd, ENTITY_TYPE_WORM, worm_amount);
	int turret_amount = 1.5 * pow(1.1, gd->wave);
	append_enemies_to_spawn(gd, ENTITY_TYPE_TURRET, turret_amount);
	int orb_amount = 1 * pow(1.1, gd->wave);
	append_enemies_to_spawn(gd, ENTITY_TYPE_ORB, orb_amount);

	unlock_upgrades(gd);

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
	return m_pos;
}

bool is_colliding(gs_vec2 p1, gs_vec2 p2, float r1, float r2)
{
	return gs_vec2_dist(p1, p2) <= r1+r2;
}

gs_vec2 gs_vec2_truncate(gs_vec2 v, float max_length)
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
	steering = gs_vec2_truncate(steering, max_force);

	velocity = gs_vec2_add(velocity, steering);
	return gs_vec2_truncate(velocity, max_speed);
}

float randf()  
{
	return (float)stb_frand();
}



// Globals
game_data_t gdata = {0};

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
	double width = RESOLUTION_X;
	double height = RESOLUTION_Y;
#ifdef GS_PLATFORM_IMPL_EMSCRIPTEN
	emscripten_get_element_css_size("#canvas", &width, &height);
#endif
	return (gs_app_desc_t) {
		.window_width = width,
		.window_height = height,
		.init = init,
		.update = update,
		.window_title = "Ever-changing Caves",
		.frame_rate = 120.f,
		.user_data = &gdata,
		.enable_vsync = true
	};
}
