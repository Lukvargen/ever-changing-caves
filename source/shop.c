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

    gd->shop.all_upgrades = NULL;
    
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_DMG,
        .cost = 25,
        .ivalue = 1
    }, 10);
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_DMG,
        .cost = 50,
        .ivalue = 3
    }, 5);
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_DMG,
        .cost = 150,
        .ivalue = 10
    }, 3);

    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_LIFETIME,
        .cost = 10,
        .fvalue = 0.1f
    }, 10);
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_SPEED,
        .cost = 10,
        .fvalue = 50.f
    }, 10);
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_ACCELL,
        .cost = 10,
        .fvalue = 50.f
    }, 10);

    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_EXPLODE,
        .cost = 100,
        .ivalue = 1
    }, 3);
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_EXPLODE,
        .cost = 50,
        .ivalue = 1
    }, 2);
    for (int i = 0; i < 10; i++) {
        append_all_upgrades(gd, (upgrade_t){
            .type = UPGRADE_TYPE_SHOOT_DELAY,
            .cost = 30+(3+i * 20),
            .fvalue = 0.05
        }, 1);
    }
    append_all_upgrades(gd, (upgrade_t){
        .type = UPGRADE_TYPE_SHOOT_REFLECT,
        .cost = 50,
        .fvalue = 0.05
    }, 10);
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
