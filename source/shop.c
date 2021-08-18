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
	gd->shop.free_reroll_left = gd->shop.free_reroll_count;
    get_available_upgrades(gd);
    gd->shop.visible = true;
}
void shop_hide(game_data_t* gd)
{
    gd->paused = false;
    gd->shop.visible = false;
    next_wave(gd);
}


float calculate_shoot_delay(int player_shoot_delay_upgrades, int dualshot)
{
	return PLAYER_BASE_SHOOT_DELAY * pow(PLAYER_SHOOT_DELAY_UPGRADE_EFFECT, player_shoot_delay_upgrades) * pow((1+PLAYER_DUAL_SHOT_SHOOT_DELAY_EFFECT), dualshot);
}

char* get_upgrade_string(game_data_t* gd, upgrade_t* upgrade, char* text, int TEXT_SIZE)
{
	char buffer[128];
    switch(upgrade->type) {
        case (UPGRADE_TYPE_DMG):
		{
			int new_base_dmg = gd->player.player_base_dmg + (int)upgrade->ivalue;
            gs_snprintf(text, TEXT_SIZE, "BASE DMG+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_base_dmg, new_base_dmg);
			gs_snprintf(buffer, TEXT_SIZE, "\nDMG (%i->%i)", gd->player.dmg, (int)(new_base_dmg * gd->player.player_dmg_multiplier));
			strcat(text, buffer);
            break;
		}
		case (UPGRADE_TYPE_DMG_MULTIPLIER):
		{
			float new_dmg_multiplier = gd->player.player_dmg_multiplier + upgrade->fvalue;
            gs_snprintf(text, TEXT_SIZE, "DMG MULTIPLIER+%.2f\n(%.2f->%.2f)", upgrade->fvalue, gd->player.player_dmg_multiplier, new_dmg_multiplier);
			
			gs_snprintf(buffer, TEXT_SIZE, "\nDMG (%i->%i)", gd->player.dmg, (int)(gd->player.player_base_dmg * new_dmg_multiplier));
			strcat(text, buffer);
            break;
		}
        case (UPGRADE_TYPE_HP):
		{
			int new_base_hp = gd->player.player_base_hp + upgrade->ivalue;
			gs_snprintf(text, TEXT_SIZE, "BASE HP+%i\n(%i->%i)", upgrade->ivalue, gd->player.player_base_hp, new_base_hp);
			
			gs_snprintf(buffer, TEXT_SIZE, "\nHP (%i->%i)", gd->player.max_hp, (int)(new_base_hp * gd->player.player_hp_multiplier));
			strcat(text, buffer);
			break;
		}
		case (UPGRADE_TYPE_HP_MULTIPLIER):
		{
			float new_hp_multiplier = gd->player.player_hp_multiplier + upgrade->fvalue;
            gs_snprintf(text, TEXT_SIZE, "HP MULTIPLIER+%.2f\n(%.2f->%.2f)", upgrade->fvalue, gd->player.player_hp_multiplier, new_hp_multiplier);
			
			gs_snprintf(buffer, TEXT_SIZE, "\nHP (%i->%i)", gd->player.max_hp, (int)(gd->player.player_base_hp * new_hp_multiplier));
			strcat(text, buffer);
            break;
		}
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
            gs_snprintf(text, TEXT_SIZE, "SHOOT DELAY-%.0f%%\n(%.2f->%.2f)", upgrade->ivalue*(1-PLAYER_SHOOT_DELAY_UPGRADE_EFFECT)*100, gd->player.player_shoot_delay, calculate_shoot_delay(gd->player.player_shoot_delay_upgrades+upgrade->ivalue, (int)gd->player.player_dual_shot));
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
		case (UPGRADE_TYPE_FREE_REROLL):
            gs_snprintf(text, TEXT_SIZE, "FREE REROLL+%i \n(%i->%i)", upgrade->ivalue, gd->shop.free_reroll_count, gd->shop.free_reroll_count + upgrade->ivalue);
            break;
		case (UPGRADE_TYPE_FIRE_DEBUFF):
            gs_snprintf(text, TEXT_SIZE, "FIRE DEBUFF+%i \n(%i->%is)", upgrade->ivalue, gd->player.player_fire_lvl, gd->player.player_fire_lvl + upgrade->ivalue);
            break;
		case (UPGRADE_TYPE_DUALSHOT):
            gs_snprintf(text, TEXT_SIZE, "DUALSHOT\nSHOOT DELAY+%.0f%% (%.2f->%.2fs)", (PLAYER_DUAL_SHOT_SHOOT_DELAY_EFFECT*100), gd->player.player_shoot_delay, calculate_shoot_delay(gd->player.player_shoot_delay_upgrades, 1));
            break;
		case (UPGRADE_TYPE_MISSILE):
            gs_snprintf(text, TEXT_SIZE, "MISSILE CHANCE+%.0f\n(%.0f%%->%.0f%%)", upgrade->fvalue * 100, gd->player.player_spawn_missile_chance * 100, (gd->player.player_spawn_missile_chance+upgrade->fvalue)*100);
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
            gd->player.player_base_dmg += upgrade->ivalue;
			gd->player.dmg = gd->player.player_base_dmg * gd->player.player_dmg_multiplier;
            break;
		case (UPGRADE_TYPE_DMG_MULTIPLIER):
            gd->player.player_dmg_multiplier += upgrade->fvalue;
			gd->player.dmg = gd->player.player_base_dmg * gd->player.player_dmg_multiplier;
            break;
        case (UPGRADE_TYPE_HP):
            gd->player.player_base_hp += upgrade->ivalue;
			gd->player.hp += upgrade->ivalue;
            gd->player.max_hp = gd->player.player_base_hp * gd->player.player_hp_multiplier;
            break;
		case (UPGRADE_TYPE_HP_MULTIPLIER):
            gd->player.player_hp_multiplier += upgrade->fvalue;
			int new_max_hp = gd->player.player_base_hp * gd->player.player_hp_multiplier;
			gd->player.hp += new_max_hp - gd->player.max_hp;
            gd->player.max_hp = new_max_hp;
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
			gd->player.player_shoot_delay_upgrades++;
			gd->player.player_shoot_delay = calculate_shoot_delay(gd->player.player_shoot_delay_upgrades, (int)gd->player.player_dual_shot);
            //gd->player.player_shoot_delay -= upgrade->fvalue;
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
		case (UPGRADE_TYPE_FREE_REROLL):
            gd->shop.free_reroll_count += upgrade->ivalue;
			gd->shop.free_reroll_left += upgrade->ivalue;
            break;
		case (UPGRADE_TYPE_FIRE_DEBUFF):
            gd->player.player_fire_lvl += upgrade->ivalue;
            break;
		case (UPGRADE_TYPE_DUALSHOT):
            gd->player.player_dual_shot = true;
			gd->player.player_shoot_delay = calculate_shoot_delay(gd->player.player_shoot_delay_upgrades, (int)gd->player.player_dual_shot);
            break;
		case (UPGRADE_TYPE_MISSILE):
            gd->player.player_spawn_missile_chance += upgrade->fvalue;
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
			.cost = 10,
			.ivalue = 1
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 10,
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
				.ivalue = 1
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT,
				.cost = 20,
				.fvalue = 0.2
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
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_FREE_REROLL,
				.cost = 10,
				.ivalue = 1
			}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_FIRE_DEBUFF,
				.cost = 25,
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
			.type = UPGRADE_TYPE_DMG_MULTIPLIER,
			.cost = 20,
			.fvalue = 0.25
			}, 4);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_HP,
				.cost = 20,
				.ivalue = 3
				}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP_MULTIPLIER,
			.cost = 20,
			.fvalue = 0.25
			}, 4);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT,
				.cost = 20,
				.fvalue = 0.1
				}, 1);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT,
				.cost = 20,
				.ivalue = 1
			}, 2);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_MISSILE,
				.cost = 25,
				.fvalue = 0.2
			}, 2);
			break;
		case(5):
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG,
			.cost = 25,
			.ivalue = 1
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG_MULTIPLIER,
			.cost = 30,
			.fvalue = 0.25
			}, 4);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 25,
			.ivalue = 5
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP_MULTIPLIER,
			.cost = 30,
			.fvalue = 0.25
			}, 4);
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
				.ivalue = 1
			}, 5);
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
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_FIRE_DEBUFF,
				.cost = 50,
				.ivalue = 1
			}, 2);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_DUALSHOT,
				.cost = 50,
				.bvalue = true
			}, 1);

			break;
		case (10):
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_DMG,
			.cost = 75,
			.ivalue = 2
			}, 5);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_SHOOT_DELAY,
				.cost = 75,
				.ivalue = 1
			}, 2);
			
			append_all_upgrades(gd, (upgrade_t){
			.type = UPGRADE_TYPE_HP,
			.cost = 75,
			.ivalue = 5
			}, 10);
			append_all_upgrades(gd, (upgrade_t){
				.type = UPGRADE_TYPE_MISSILE,
				.cost = 75,
				.fvalue = 0.1
			}, 2);
			break;
	}
}