/* velox: velox/monitor.h
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

#ifndef VELOX_MONITOR_H
#define VELOX_MONITOR_H

#include <xcb/xcb.h>

#include <velox/vector.h>
#include <velox/area.h>

struct velox_monitor
{
    struct velox_area area;
    struct velox_tag * tag;
};

DEFINE_VECTOR(velox_monitor_vector, struct velox_monitor);

extern struct velox_monitor_vector monitors;

void setup_monitors();
void cleanup_monitors();

void add_monitor(const struct velox_area * area);

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

