#include "shop.h"
#include "main.h"

void append_all_upgrades(game_data_t* gd, upgrade_t upgrade, int amount)
{
    for (int i = 0; i < amount; i++) {
        gs_dyn_array_push(gd->shop.all_upgrades, upgrade);
    }
}

void shop_init_all_upgrades(game_data_t* gd)
{
    for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
        gd->shop.upgrades_available[i] = (upgrade_t){.type=UPGRADE_TYPE_NULL,.cost = 9999};
    }
}

uint32_t pos_mod(int value, uint32_t m)
{
    int mod = value % m;
    if (mod < 0) {
        mod += m;
    }
    return mod;
}

void get_available_upgrades(game_data_t* gd) 
{
    for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
        if (gd->shop.upgrades_available[i].type != UPGRADE_TYPE_NULL) {
            gs_dyn_array_push(gd->shop.all_upgrades, gd->shop.upgrades_available[i]);
            
        }
    }

    // remove from list and then read the unpurchased ones
    for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
        int upgrades_size = gs_dyn_array_size(gd->shop.all_upgrades);
        if (upgrades_size > 0) {
            int index = pos_mod(stb_rand(), upgrades_size);
            
            gd->shop.upgrades_available[i] = gd->shop.all_upgrades[index];
            gd->shop.all_upgrades[index] = gd->shop.all_upgrades[upgrades_size-1];
            gs_dyn_array_pop(gd->shop.all_upgrades);
        } else {
            gd->shop.upgrades_available[i] = (upgrade_t){.type=UPGRADE_TYPE_NULL};
            printf("WARNING: NO MORE UPGRADES\n");
        }
    }
}

void shop_show(game_data_t* gd)
{
    gd->paused = true;
    get_available_upgrades(gd);
    gd->shop.visible = true;
}
void shop_hide(game_data_t* gd)
{
    gd->paused = false;
    gd->shop.visible = false;
    next_wave(gd);
}

char* get_upgrade_string(game_data_t* gd, upgrade_t* upgrade, char* text, int TEXT_SIZE)
{
    switch(upgrade->type) {
        case (UPGRADE_TYPE_DMG):
            gs_snprintf(text, TEXT_SIZE, "DMG+%i\n(%i->%i)", upgrade->ivalue, gd->player.dmg, gd->player.dmg + upgrade->ivalue);
            break;
        case (UPGRADE_TYPE_HP):
            gs_snprintf(text, TEXT_SIZE, "HP+%i\n(%i->%i)", upgrade->ivalue, gd->player.max_hp, gd->player.max_hp + upgrade->ivalue);
            break;
        case (UPGRADE_TYPE_LIFETIME):
            gs_snprintf(text, TEXT_SIZE, "PROJECTILE LIFETIME+%.1f\n(%.1f->%.1f)", upgrade->fvalue, gd->player.player_projectile_lifetime, gd->player.player_projectile_lifetime + upgrade->fvalue);
            break;
        case (UPGRADE_TYPE_SPEED):
            gs_snprintf(text, TEXT_SIZE, "PROJECTILE SPEED+%.0f\n(%.0f->%.0f)", upgrade->fvalue, gd->player.player_projectile_speed, gd->player.player_projectile_speed + upgrade->fvalue);
            break;
        case (UPGRADE_TYPE_ACCELL):
            gs_snprintf(text, TEXT_SIZE, "PROJECTILE ACCEL+%.0f\n(%.0f->%.0f)", upgrade->fvalue, gd->player.player_projectile_accel, gd->player.player_projectile_accel + upgrade->fvalue);
            break;
        case (UPGRADE_TYPE_EXPLODE):
            gs_snprintf(text, TEXT_SIZE, "PROJECTILE EXPLODE+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_explosion_radius, gd->player.player_explosion_radius + upgrade->ivalue);
            break;
        case (UPGRADE_TYPE_SHOOT_DELAY):
            gs_snprintf(text, TEXT_SIZE, "SHOOT DELAY-%.2f\n(%.2f->%.2f)", upgrade->fvalue, gd->player.player_shoot_delay, gd->player.player_shoot_delay - upgrade->fvalue);
            break;
        case (UPGRADE_TYPE_SHOOT_REFLECT):
            gs_snprintf(text, TEXT_SIZE, "SHOT SPLIT CHANCE+%.0f %%\n(%.0f%%->%.0f%%)", upgrade->fvalue*100, gd->player.player_projectile_reflect_chance*100, 100*(gd->player.player_projectile_reflect_chance + upgrade->fvalue));
            break;
        case (UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT):
            gs_snprintf(text, TEXT_SIZE, "SHOT ONHIT SPLIT AMOUNT+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_projectile_reflect_amount, gd->player.player_projectile_reflect_amount + upgrade->ivalue);
            break;
        case (UPGRADE_TYPE_LASER):
            gs_snprintf(text, TEXT_SIZE, "LASER TARGETS+%i \n(%i->%i)", upgrade->ivalue, gd->player.player_laser_lvl, gd->player.player_laser_lvl + upgrade->ivalue);
            break;
        default:
            gs_snprintf(text, TEXT_SIZE, "ERROR");
            gs_println("NO UPGRADE TYPE");
            break;
    }
}

void upgrade_purchase(game_data_t* gd, upgrade_t* upgrade)
{
    if (gd->crystals_currency >= upgrade->cost) {
        gd->crystals_currency -= upgrade->cost;
        gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
    } else {
        gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
        return;
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

void unlock_upgrades(game_data_t* gd)
{
    switch (gd->wave) {
		case (1):
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG,
			.cost = 20,
			.ivalue = 1
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 20,
			.ivalue = 2
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_LIFETIME,
				.cost = 10,
				.fvalue = 0.1f
			}, 3);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SPEED,
				.cost = 10,
				.fvalue = 50.f
			}, 3);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_ACCELL,
				.cost = 10,
				.fvalue = 50.f
			}, 3);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_EXPLODE,
				.cost = 20,
				.ivalue = 1
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_DELAY,
				.cost = 20,
				.fvalue = 0.05
			}, 3);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT,
				.cost = 20,
				.fvalue = 0.1
				}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT,
				.cost = 40,
				.fvalue = 0.1
				}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_LASER,
				.cost = 50,
				.ivalue = 1
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_LASER,
				.cost = 30,
				.ivalue = 1
			}, 1);
			break;
		case(3):
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_DMG,
				.cost = 20,
				.ivalue = 1
				}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_HP,
				.cost = 20,
				.ivalue = 3
				}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT,
				.cost = 30,
				.ivalue = 1
			}, 2);
			break;
		case(5):
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG,
			.cost = 25,
			.ivalue = 2
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 25,
			.ivalue = 5
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_LIFETIME,
				.cost = 15,
				.fvalue = 0.1f
			}, 2);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SPEED,
				.cost = 15,
				.fvalue = 50.f
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_ACCELL,
				.cost = 15,
				.fvalue = 50.f
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_EXPLODE,
				.cost = 40,
				.ivalue = 1
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_DELAY,
				.cost = 30,
				.fvalue = 0.05
			}, 2);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_SHOOT_REFLECT,
			.cost = 50,
			.fvalue = 0.1
			}, 2);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_LASER,
				.cost = 50,
				.ivalue = 1
			}, 2);

			break;
		case (10):
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG,
			.cost = 30,
			.ivalue = 2
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 40,
			.ivalue = 5
			}, 10);
			break;
	}
}