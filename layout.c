/* mwm: layout.c
 *
 * Copyright (c) 2009 Michael Forney <michael@obberon.com>
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

#include "mwm.h"
#include "layout.h"

void tile_arrange(struct mwm_window_stack * windows)
{
    struct mwm_window * window = NULL;
    struct mwm_window_stack * current_element = NULL;
    uint16_t mask;
    uint32_t values[4];
    uint16_t window_count = 0;
    uint16_t window_index = 0;

    if (windows == NULL)
    {
        return;
    }

    /* Arange the master */
    window = windows->window;

    if (windows->next == NULL)
    {
        window->x = 0;
        window->y = 0;
        window->width = screen_width;
        window->height = screen_height;
    }
    else
    {
        window->x = 0;
        window->y = 0;
        window->width = screen_width / 2;
        window->height = screen_height;
    }

    mask = XCB_CONFIG_WINDOW_X |
           XCB_CONFIG_WINDOW_Y |
           XCB_CONFIG_WINDOW_WIDTH |
           XCB_CONFIG_WINDOW_HEIGHT;

    values[0] = window->x;
    values[1] = window->y;
    values[2] = window->width;
    values[3] = window->height;

    xcb_configure_window(c, window->window_id, mask, values);
    synthetic_configure(window);

    /* Calculate number of windows */
    for (current_element = windows; current_element != NULL; current_element = current_element->next, window_count++);

    /* Arange the rest of the windows */
    for (current_element = windows->next; current_element != NULL; current_element = current_element->next, window_index++)
    {
        window = current_element->window;

        window->x = screen_width = 2;
        window->y = screen_height * window_index / window_count;
        window->width = screen_width / 2;
        window->height = streen_height / window_count;

        mask = XCB_CONFIG_WINDOW_X |
               XCB_CONFIG_WINDOW_Y |
               XCB_CONFIG_WINDOW_WIDTH |
               XCB_CONFIG_WINDOW_HEIGHT;

        values[0] = window->x;
        values[1] = window->y;
        values[2] = window->width;
        values[3] = window->height;

        xcb_configure_window(c, window->window_id, mask, values);
        synthetic_configure(window);
    }
}

void setup_layouts()
{
    layouts[TILE] = (struct mwm_layout *) malloc(sizeof(struct mwm_layout));
    layouts[TILE]->identifier = "Tile";
    layouts[TILE]->arrange = &tile_arrange;
}

