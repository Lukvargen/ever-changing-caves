#ifndef SHOP_H
#define SHOP_H

#include <gs/gs.h>

#define SHOP_UPGRADES_SIZE 3

typedef enum upgrade_type_t
{
	UPGRADE_TYPE_DMG,
	UPGRADE_TYPE_DMG_MULTIPLIER,
	UPGRADE_TYPE_HP,
	UPGRADE_TYPE_HP_MULTIPLIER,
	UPGRADE_TYPE_LIFETIME,
	UPGRADE_TYPE_SPEED,
	UPGRADE_TYPE_ACCELL,
	UPGRADE_TYPE_EXPLODE,
	UPGRADE_TYPE_SHOOT_DELAY,
	UPGRADE_TYPE_SHOOT_REFLECT,
	UPGRADE_TYPE_SHOOT_REFLECT_AMOUNT,
	UPGRADE_TYPE_LASER,
	UPGRADE_TYPE_FREE_REROLL,
	UPGRADE_TYPE_FIRE_DEBUFF,
	UPGRADE_TYPE_SIZE,
    UPGRADE_TYPE_NULL
} upgrade_type_t;


typedef struct upgrade_t
{
	upgrade_type_t type;
	int cost;
	union
	{
		int ivalue;
		float fvalue;
		bool bvalue;
	};
} upgrade_t;

typedef struct shop_t
{
	upgrade_t upgrades_available[SHOP_UPGRADES_SIZE];
	gs_dyn_array(upgrade_t) all_upgrades;
	int free_reroll_count;
	int free_reroll_left;
	bool visible;

} shop_t;

typedef struct game_data_t game_data_t;
void shop_init_all_upgrades(game_data_t* gd);
void append_all_upgrades(game_data_t* gd, upgrade_t upgrade, int amount);
void get_available_upgrades(game_data_t* gd);
void shop_show(game_data_t* gd);
void shop_hide(game_data_t* gd);
char* get_upgrade_string(game_data_t* gd, upgrade_t* upgrade, char* text, int TEXT_SIZE);
void upgrade_purchase(game_data_t* gd, upgrade_t* upgrade);
void unlock_upgrades(game_data_t* gd);
#endif