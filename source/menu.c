/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "menu.h"

#include "types.h"
#include "utils.h"
#include "gfx.h"
#include "console.h"
#include <stdio.h>

#include "smc.h"

menu* __menu;
menu *menu_chain[100];
int opened_menus = 0;

bool menu_active = true;
int _menu_state = -1;

void menu_set_state(int state)
{
    _menu_state = state;
}

int menu_get_state()
{
    return _menu_state;
}

void menu_init(menu* new_menu)
{
    menu_set_state(0); // Set state to in-menu.

    int i = 0;
    char item_buffer[100] = {0};
    __menu = new_menu;
    console_init();
    __menu->selected = 0;
    __menu->showed = 0;
    console_add_text(__menu->title);
    console_add_text("");

    for (i = 0; i < __menu->subtitles; i++)
        console_add_text(__menu->subtitle[i]);

    console_add_text("");

    for(i = 0; i < __menu->entries; i++)
    {
        sprintf(item_buffer, " %s", __menu->option[i].text);
        console_add_text(item_buffer);
    }

    while(menu_active)
    {
        menu_show();

        u8 input = smc_get_events();

        //TODO: Double press to go back? Or just add "Back" options

        if(input & SMC_EJECT_BUTTON) menu_select();
        if(input & SMC_POWER_BUTTON) menu_next_selection();
    }
}

void menu_show()
{
    int i = 0, x = 0, y = 0;
    console_get_xy(&x, &y);
    if(!__menu->showed)
    {
        console_show();
        __menu->showed = 1;
    }

    // Update cursor.
    for(i = 0; i < __menu->entries; i++)
        gfx_draw_string(GFX_DRC, i == __menu->selected ? ">" : " ", x + CHAR_WIDTH, (i+3+__menu->subtitles) * CHAR_WIDTH + y + CHAR_WIDTH * 2, GREEN);
}

void menu_next_selection()
{
    if(__menu->selected + 1 < __menu->entries)
        __menu->selected++;
    else
        __menu->selected = 0;
}

void menu_prev_selection()
{
    if(__menu->selected > 0)
        __menu->selected--;
    else
        __menu->selected = __menu->entries - 1;
}

void menu_next_jump()
{
    if(__menu->selected + 5 < __menu->entries)
        __menu->selected += 5;
    else
        __menu->selected = __menu->entries - 1;
}

void menu_prev_jump()
{
    if(__menu->selected > 5)
        __menu->selected -= 5;
    else
        __menu->selected = 0;
}


void menu_select()
{
    if(__menu->option[__menu->selected].callback != NULL)
    {
        menu_chain[opened_menus++] = __menu;
        menu_set_state(1); // Set menu state to in callback.
        __menu->option[__menu->selected].callback();
        menu_set_state(0); // Return to in-menu state.
        menu_init(menu_chain[--opened_menus]);
    }
}

void menu_close()
{
    if(opened_menus > 0)
        menu_init(menu_chain[--opened_menus]);
}

void menu_reset()
{
    opened_menus = 0;
}
