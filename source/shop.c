#include "shop.h"
#include "main.h"

void append_all_upgrades(game_data_t* gd, upgrade_type_t type, int cost, int amount)
{
    for (int i = 0; i < amount; i++) {
        upgrade_t upgrade = {
            .type = type,
            .cost = cost
        };
        gs_dyn_array_push(gd->shop.all_upgrades, upgrade);
    }
}

void shop_init_all_upgrades(game_data_t* gd)
{
    for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
        gd->shop.upgrades_available[i] = (upgrade_t){.type=UPGRADE_TYPE_NULL,.cost = 9999};
    }

    gd->shop.all_upgrades = NULL;
    append_all_upgrades(gd, UPGRADE_TYPE_DMG, 10, 10);
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

    // remove from list and then readd the unpurchased ones.
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
    get_available_upgrades(gd);
    
    gd->shop.visible = true;
}
