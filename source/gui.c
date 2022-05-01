
#include "gui.h"

void app_load_style_sheet(game_data_t* gd, bool destroy);
void draw_debug_window(game_data_t* gd, gs_gui_context_t* gui);
void draw_shop_upgrade_panel(game_data_t* gd, gs_gui_context_t* gui, int upgrade_id);
void draw_shop(game_data_t* gd, gs_gui_context_t* gui);
void draw_winscreen(game_data_t* gd, gs_gui_context_t* gui);

void gui_init(game_data_t* gd)
{
    gs_gui_init(&gd->gs_gui, gs_platform_main_window());
    gs_println("GUI INIT");

    gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &gd->font_large, 32);
	gs_asset_font_load_from_file("./assets/joystix monospace.ttf", &gd->font_medium, 16);
	gs_asset_font_load_from_file("./assets/fff-forward.regular.ttf", &gd->font_small, 12);
    gs_gui_init_font_stash(&gd->gs_gui, &(gs_gui_font_stash_desc_t){
        .fonts = (gs_gui_font_desc_t[]) {
            //{.key = "large", .font = &gd->font_large},
            {.key = "medium", .font = &gd->font_medium},
            //{.key = "small", .font = &gd->font_small}
        },
        .size = 1 * sizeof(gs_gui_font_desc_t)
    });

    app_load_style_sheet(gd, false);
}


void gui_update(game_data_t* gd)
{

    gs_gui_context_t* gui = &gd->gs_gui;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    
    gs_gui_begin(gui, gs_v2(RESOLUTION_X, RESOLUTION_Y));

    if (gs_gui_window_begin_ex(gui, "#root", gs_gui_rect(0, 0, 0, 0), NULL, NULL, 
        GS_GUI_OPT_NOFRAME | 
        GS_GUI_OPT_NOTITLE | 
        GS_GUI_OPT_NOMOVE | 
        GS_GUI_OPT_FULLSCREEN | 
        GS_GUI_OPT_NORESIZE | 
        GS_GUI_OPT_NODOCK | 
        GS_GUI_OPT_NOBRINGTOFRONT
    ))
    { 
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui);

        draw_winscreen(gd, gui);
        draw_shop(gd, gui);

        // Wave text
        {
            gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 100, 25, 0, 0, GS_GUI_LAYOUT_ANCHOR_TOPRIGHT), 0); 
            gs_snprintfc(WAVE_TEXT, 256, "Wave: %d", gd->wave);
            gs_gui_label_ex(gui, WAVE_TEXT, NULL, 0x00);
        }

        // top left
        {
            gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, RESOLUTION_Y -5*2, 5, 5, GS_GUI_LAYOUT_ANCHOR_TOPLEFT), 0); 
            gs_gui_panel_begin_ex(gui, "#left_panel", NULL, GS_GUI_OPT_NOSCROLL | GS_GUI_OPT_NOFRAME | GS_GUI_OPT_NOCLIP);

                // hpbar
                float hp_bar_height = 25;
                gs_gui_layout_row(gui, 1, (int[]){-1}, hp_bar_height);

                gs_gui_rect_t hp_bar_bg_rect = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, hp_bar_bg_rect, gs_color_ctor(40, 40, 40, 255));
                
                gs_gui_rect_t hp_bar_content_rect = hp_bar_bg_rect;
                float padding = 2;
                hp_bar_bg_rect.x += padding;
                hp_bar_bg_rect.y += padding;
                hp_bar_bg_rect.w -= padding*2;
                hp_bar_bg_rect.h -= padding*2;
                
                gs_gui_rect_t hp_bar_over_rect = hp_bar_bg_rect;
                hp_bar_over_rect.w *= gd->player.hp / gd->player.max_hp;
                gs_gui_draw_rect(gui, hp_bar_bg_rect, gs_color_ctor(204, 41, 54, 255));
                gs_gui_draw_rect(gui, hp_bar_over_rect, gs_color_ctor(71, 160, 37, 255));


                gs_gui_layout_row(gui, 1, (int[]){-1}, 20);
                gs_snprintfc(CRYSTALS_TEXT, 256, "Crystals: %d", gd->crystals_currency);
                
                gs_gui_label_ex(gui, CRYSTALS_TEXT, NULL, 0x00);

            gs_gui_panel_end(gui);
            
        }


        


        if (gd->game_over) {
            float game_over_width = RESOLUTION_X / 3.0;
            gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, game_over_width, RESOLUTION_Y*0.5, 0, 0, GS_GUI_LAYOUT_ANCHOR_CENTER), 0); 
            gs_gui_panel_begin_ex(gui, "#game_over_panel", NULL, GS_GUI_OPT_NOFRAME);

                gs_gui_layout_row_ex(gui, 1, (int[]){-1}, 50, GS_GUI_JUSTIFY_CENTER);

                gs_gui_label_ex(gui, "GAME OVER", &(gs_gui_selector_desc_t){.classes={"header"}}, 0x00);

                if (gs_gui_button_ex(gui, "RESTART", &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                    gd->restart = true;
                }

            gs_gui_panel_end(gui);

        }

        if (gd->debug) {
            draw_debug_window(gd, gui);
        }

        gs_gui_window_end(gui);
    }

    gs_gui_end(gui);
}


void draw_debug_window(game_data_t* gd, gs_gui_context_t* gui)
{
    const gs_vec2 ws = gs_v2(500.f, 300.f);
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    if (gs_gui_window_begin(gui, "Debug Window", gs_gui_rect((fbs.x - ws.x) * 0.5f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y)));
    {
        gs_gui_layout_row(gui, 2, (int[]){65, -1}, 0);

        gs_gui_label(gui, "Position");
        gs_gui_label(gui, "%.0f, %.0f", gd->player.position.x, gd->player.position.y);
        

        gs_gui_label(gui, "Wave");
        gs_gui_real wave = gd->wave;
        gs_gui_slider_ex(gui, &wave, 0, 100, 1, "%.0f", NULL, 0x00);
        gd->wave = (int)wave;

        gs_gui_layout_row(gui, 2, (int[]){-1, -1}, -1);
        gs_gui_label_ex(gui, "Some text", &(gs_gui_selector_desc_t){
            .classes={"c0"}
        }, 0x00);
        
        
    }
    gs_gui_window_end(gui);
}

void draw_shop(game_data_t* gd, gs_gui_context_t* gui)
{
    if (gd->shop.visible && !gd->winscreen_visible) {
        // Shop
        float shop_width = RESOLUTION_X;
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui);
        gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, shop_width, RESOLUTION_Y*0.6, 0, 0, GS_GUI_LAYOUT_ANCHOR_CENTER), 0); 
        gs_gui_panel_begin_ex(gui, "#shop_panel", NULL, GS_GUI_OPT_NOFRAME);

            gs_gui_layout_row_ex(gui, 2, (int[]){150, 150}, 50, GS_GUI_JUSTIFY_CENTER);
            
            gs_gui_panel_begin_ex(gui, "#heal_panel", &(gs_gui_selector_desc_t){.classes={"upgrade_panel"}}, 0x00);
                gs_gui_layout_row_ex(gui, 1, (int[]){-1}, 15, GS_GUI_JUSTIFY_START);

                gs_gui_label_ex(gui, "HEAL", NULL, 0x00);

                gs_gui_layout_row(gui, 1, (int[]){-1}, -1);
                
                int heal_cost = 10;
                gs_color_t btn_color = gs_color(60, 20, 20, 255);
                if (gd->crystals_currency >= heal_cost) {
                    btn_color = gs_color(20, 60, 20, 255);
                };

                gs_snprintfc(HEAL_UPGRADE_ID, 256, "Cost: %d##%s",heal_cost, "heal");
                
                if (gs_gui_button_ex(gui, HEAL_UPGRADE_ID, &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                    if (gd->crystals_currency >= heal_cost) {
                        gd->crystals_currency -= heal_cost;
                        gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
                        gd->player.hp = gd->player.max_hp;
                    } else {
                        gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
                        
                    }
                }

            gs_gui_panel_end(gui);

            gs_gui_panel_begin_ex(gui, "#reroll_panel", &(gs_gui_selector_desc_t){.classes={"upgrade_panel"}}, 0x00);
                gs_gui_layout_row_ex(gui, 1, (int[]){-1}, 15, GS_GUI_JUSTIFY_START);

                gs_gui_label_ex(gui, "REROLL", NULL, 0x00);

                gs_gui_layout_row(gui, 1, (int[]){-1}, -1);
                int reroll_cost = 5 * pow(1.1, gd->wave-1);
                if (gd->shop.free_reroll_left > 0)
                    reroll_cost = 0;
                gs_snprintfc(REROLL_UPGRADE_ID, 256, "Cost: %d##%s", reroll_cost, "reroll");
                if (gs_gui_button_ex(gui, REROLL_UPGRADE_ID, &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                    if (gd->crystals_currency >= reroll_cost) {
                        gd->crystals_currency -= reroll_cost;
                        if (gd->shop.free_reroll_left > 0)
                            gd->shop.free_reroll_left--;
                        gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
                        get_available_upgrades(gd);
                    } else {
                        gs_audio_play_source(gd->buy_negative_sound_hndl, gd->volume);
                        
                    }
                }

            gs_gui_panel_end(gui);

            
            gs_gui_layout_row_ex(gui, 3, (int[]){200, 200, 200}, 120, GS_GUI_JUSTIFY_CENTER);
            
            
            for (int i = 0; i < SHOP_UPGRADES_SIZE; i++) {
                draw_shop_upgrade_panel(gd, gui, i);

            }

            gs_gui_layout_row_ex(gui, 1, (int[]){150}, 30, GS_GUI_JUSTIFY_CENTER);
            if (gs_gui_button_ex(gui, "Next Wave", &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
                shop_hide(gd);
            }

        gs_gui_panel_end(gui);
    }
}

void draw_winscreen(game_data_t* gd, gs_gui_context_t* gui) 
{
    if (gd->winscreen_visible) {
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui);
        gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, RESOLUTION_Y*0.5, 0, 0, GS_GUI_LAYOUT_ANCHOR_CENTER), 0); 

         gs_gui_panel_begin_ex(gui, "#win_panel", NULL, GS_GUI_OPT_NOFRAME | GS_GUI_JUSTIFY_CENTER);
            gs_gui_layout_row_ex(gui, 1, (int[]){-1}, 20, 0x00);
            

            gs_gui_label_ex(gui, "You Won!",&(gs_gui_selector_desc_t){.classes={"header"}}, 0x00);

            gs_gui_layout_row(gui, 1, (int[]){-1}, 50);
            
            if (gs_gui_button_ex(gui, "Continue", &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                gd->winscreen_visible = false;
                gs_audio_play_source(gd->buy_positive_sound_hndl, gd->volume);
            }

        gs_gui_panel_end(gui);
    }
}


void draw_shop_upgrade_panel(game_data_t* gd, gs_gui_context_t* gui, int upgrade_id)
{
    upgrade_t* upgrade = &gd->shop.upgrades_available[upgrade_id];
    #define TEXT_SIZE 128
    char text[TEXT_SIZE];
    get_upgrade_string(gd, upgrade, text, TEXT_SIZE);
    gs_snprintfc(cost_text, TEXT_SIZE, "Cost %i", upgrade->cost);
    
    if (upgrade->type != UPGRADE_TYPE_NULL) {

        gs_gui_panel_begin_ex(gui, "#upgrade_panel", &(gs_gui_selector_desc_t){.classes={"upgrade_panel"}}, 0x00);
            gs_gui_layout_row_ex(gui, 1, (int[]){-1}, 80, GS_GUI_JUSTIFY_START);
            gs_gui_text_ex(gui, text,true, NULL, 0x00);

            gs_gui_layout_row(gui, 1, (int[]){-1}, -1);
            gs_snprintfc(UPGRADE_ID, 256, "%s##%d",cost_text, upgrade_id);
            if (gs_gui_button_ex(gui, UPGRADE_ID, &(gs_gui_selector_desc_t){.classes={"btn"}}, 0x00)) {
                upgrade_purchase(gd, upgrade);
            }

        gs_gui_panel_end(gui);
    }
}


void app_load_style_sheet(game_data_t* gd, bool destroy)
{
    if (destroy) {
        gs_gui_style_sheet_destroy(&gd->gs_gui, &gd->style_sheet);
    }

    gd->style_sheet = gs_gui_style_sheet_load_from_file(&gd->gs_gui, "./assets/gui.ss");
    gs_gui_set_style_sheet(&gd->gs_gui, &gd->style_sheet);
}
