#ifndef SHOP_H
#define SHOP_H

//#include "main.h"
#include <gs/gs.h>
#include <gs/util/gs_idraw.h>

#define SHOP_UPGRADES_SIZE 3

typedef enum upgrade_type_t
{
	UPGRADE_TYPE_DMG,
	UPGRADE_TYPE_LIFETIME,
	UPGRADE_TYPE_SPEED,
	UPGRADE_TYPE_ACCELL,
	UPGRADE_TYPE_EXPLODE,
	UPGRADE_TYPE_SHOOT_DELAY,
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
	int count;
	int max_count;
	// get_desc
	//  switch (type)
	// 		case UPGRADE_TYPE_DMG
	//			return 10
	// apply_effects
	// 	switch (type)
	//		case UPGRADE_TYPE_DMG
	//				player.dmg += 10
	//			kan ju ha value med men räcker de??
	// känns ju inte så bra då va.. olika typ på value osv. union? men det blir fortfarande knas när man ska få beskrivningen eller? igentligen inte då (bool)value
	// men det är ju inte så värst flexibelt
	// description = "Dmg += %f (%f -> %f)" eller bara ha i funktionen
} upgrade_t;

typedef struct shop_t
{
	upgrade_t upgrades_available[SHOP_UPGRADES_SIZE];
	gs_dyn_array(upgrade_t) all_upgrades;
	bool visible;
	

} shop_t;

typedef struct game_data_t game_data_t;
void shop_init_all_upgrades(game_data_t* gd);
void get_available_upgrades(game_data_t* gd);
void shop_show(game_data_t* gd);
#endif