/* mwm: mwm/hook.c
 *
 * Copyright (c) 2009, 2010 Michael Forney <michael@obberon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mwm.h"
#include "hook.h"

typedef void (* startup_hook_t)();
typedef void (* manage_hook_t)(struct mwm_window *);

/* Manage hooks */
void handle_floating(struct mwm_window * window);
void handle_fullscreen(struct mwm_window * window);

startup_hook_t startup_hooks[] = {
};

manage_hook_t manage_hooks[] = {
    &handle_floating,
    &handle_fullscreen
};

void run_startup_hooks()
{
    uint16_t startup_hooks_count = sizeof(startup_hooks) / sizeof(startup_hook_t);
    uint16_t startup_hook_index;

    for (startup_hook_index = 0; startup_hook_index < startup_hooks_count; startup_hook_index++)
    {
        startup_hooks[startup_hook_index]();
    }
}

void run_manage_hooks(struct mwm_window * window)
{
    uint16_t manage_hooks_count = sizeof(manage_hooks) / sizeof(manage_hook_t);
    uint16_t manage_hook_index;

    for (manage_hook_index = 0; manage_hook_index < manage_hooks_count; manage_hook_index++)
    {
        manage_hooks[manage_hook_index](window);
    }
}

/* Manage hooks */
void handle_floating(struct mwm_window * window)
{
    if (strcmp(window->name, "MPlayer") == 0)
    {
        window->floating = true;
    }
}

void handle_fullscreen(struct mwm_window * window)
{
    if (window->width == screen_width && window->height == screen_height)
    {
        printf("fullscreen window\n");

        window->x = 0;
        window->y = 0;
        window->border_width = 0;
    }
}
