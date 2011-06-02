/* velox: velox/monitor.c
 *
 * Copyright (c) 2011 Michael Forney <mforney@mforney.org>
 *
 * This file is a part of velox.
 *
 * velox is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2, as published by the Free
 * Software Foundation.
 *
 * velox is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with velox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "monitor.h"
#include "velox.h"

struct velox_monitor_vector monitors;

static void __attribute__((constructor)) initialize_monitors()
{
    vector_initialize(&monitors, 32);
}

static void __attribute__((destructor)) free_tags()
{
    vector_free(&monitors);
}

void setup_monitors()
{
    uint32_t index;

    if (monitors.size == 0)
        add_monitor(&screen_area);

    if (tags.size < monitors.size)
        die("You must have at least as many tags as monitors");

    for (index = 0; index < monitors.size; ++index)
        monitors.data[index].tag = &tags.data[index];

    monitor = &monitors.data[0];
}

void cleanup_monitors()
{
    vector_clear(&monitors);
}

void add_monitor(const struct velox_area * area)
{
    struct velox_monitor * monitor;

    monitor = vector_append_address(&monitors);
    monitor->area = *area;
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

