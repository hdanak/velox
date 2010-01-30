/* velox: velox/module-private.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#ifndef VELOX_MODULE_PRIVATE_H
#define VELOX_MODULE_PRIVATE_H

#include <libvelox/list.h>

struct velox_module
{
    void * handle;
    const char * name;
    void (* initialize)();
    void (* cleanup)();
};

extern struct velox_list * modules;

void load_module(const char * path);
void initialize_modules();
void cleanup_modules();

#endif

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8
