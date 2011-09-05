/* velox: velox/event_handler.c
 *
 * Copyright (c) 2010 Michael Forney <mforney@mforney.org>
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
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

#include "event_handler.h"
#include "velox.h"
#include "window.h"
#include "hook.h"
#include "modifier.h"
#include "debug.h"
#include "list.h"

#include "velox-private.h"
#include "hook-private.h"
#include "ewmh-private.h"
#include "binding-private.h"

static struct list_head event_handlers[UINT8_MAX];
static uint32_t event_handlers_size =
    sizeof(event_handlers) / sizeof(struct list_head);

static void __attribute__((constructor)) initialize_event_handlers()
{
    uint8_t index;

    for (index = 0; index < event_handlers_size; ++index)
    {
        INIT_LIST_HEAD(&event_handlers[index]);
    }
}

struct event_handler_entry
{
    velox_event_handler_t handler;
    struct list_head head;
};

void add_event_handler(uint8_t type, velox_event_handler_t handler)
{
    struct event_handler_entry * entry;

    entry = (struct event_handler_entry *)
        malloc(sizeof(struct event_handler_entry));
    entry->handler = handler;

    list_add_tail(&entry->head, &event_handlers[type]);
}

void handle_event(xcb_generic_event_t * event)
{
    struct event_handler_entry * entry;
    struct list_head * handlers;
    uint8_t type;

    type = XCB_EVENT_RESPONSE_TYPE(event);
    handlers = &event_handlers[type];

    if (list_empty(handlers))
    {
        DEBUG_PRINT("no handlers for event type %u\n", type);
    }

    list_for_each_entry(entry, handlers, head)
    {
        entry->handler(event);
    }
}

/* X event handlers */
static void key_press(xcb_key_press_event_t * event)
{
    xcb_keysym_t keysym = 0;
    struct velox_binding * binding;

    keysym = xcb_get_keyboard_mapping_keysyms(keyboard_mapping)[keyboard_mapping->keysyms_per_keycode * (event->detail - xcb_get_setup(c)->min_keycode)];

    DEBUG_PRINT("keysym: %x\n", keysym)
    DEBUG_PRINT("modifiers: %i\n", event->state)

    vector_for_each(&key_bindings, binding)
    {
        if (keysym == binding->bindable.pressable.key &&
            ((binding->bindable.modifiers == XCB_MOD_MASK_ANY) ||
            (CLEAN_MASK(event->state) == binding->bindable.modifiers)))
        {
            if (binding->function != NULL)
            {
                binding->function(binding->arg);
            }
        }
    }
}

static void button_press(xcb_button_press_event_t * event)
{
    struct velox_binding * binding;
    xcb_button_t button;

    DEBUG_ENTER

    button = event->detail;

    DEBUG_PRINT("button: %u\n", button);
    DEBUG_PRINT("window: 0x%x\n", event->event);

    /* Mouse bindings are grabbed with the root window, so if the event window
     * is root, call any binding functions on the sub window */
    if (event->event == screen->root)
    {
        vector_for_each(&button_bindings, binding)
        {
            DEBUG_PRINT("binding button: %u\n", binding->bindable.pressable.button);
            if (button == binding->bindable.pressable.button &&
                ((binding->bindable.modifiers == XCB_MOD_MASK_ANY) ||
                (CLEAN_MASK(event->state) == binding->bindable.modifiers)))
            {
                if (binding->function != NULL)
                {
                    binding->function(window_id_argument(event->child));
                }
            }
        }
    }
    /* Otherwise, we are just clicking on the window to focus it */
    else
    {
        struct velox_window * window = lookup_window(event->event);

        focus(event->event);

        if (window && window->floating)
            monitor->tag->focus_type = FLOAT;
        else
            monitor->tag->focus_type = TILE;
    }
}

static void enter_notify(xcb_enter_notify_event_t * event)
{
    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event)

    if (monitor->tag->focus_type == FLOAT) return;

    if (event->event == screen->root) focus(screen->root);
    else
    {
        struct velox_window_entry * entry;
        struct velox_window * window;

        window = NULL;

        /* Look through tiled windows */
        list_for_each_entry(entry, &monitor->tag->tiled.windows, head)
        {
            if (entry->window->window_id == event->event)
            {
                window = entry->window;
                window->tag->tiled.focus = &entry->head;
                break;
            }
        }

        if (window != NULL)
        {
            DEBUG_PRINT("mode: %i\n", event->mode)
            DEBUG_PRINT("detail: %i\n", event->detail)

            if (event->mode == XCB_NOTIFY_MODE_NORMAL && event->detail != XCB_NOTIFY_DETAIL_INFERIOR)
            {
                focus(window->window_id);
            }
        }
    }
}

static void leave_notify(xcb_leave_notify_event_t * event)
{
    DEBUG_ENTER
}

static void destroy_notify(xcb_destroy_notify_event_t * event)
{
    struct velox_window_entry * entry;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event)

    if (event->event == screen->root) return;

    unmanage(event->event);
}

static void unmap_notify(xcb_unmap_notify_event_t * event)
{
    struct velox_window_entry * entry;
    struct velox_window * window;

    DEBUG_ENTER
    DEBUG_PRINT("window_id: 0x%x\n", event->event);

    if (event->event == screen->root) return;

    uint32_t property_values[2];

    DEBUG_PRINT("setting state to withdrawn\n")

    xcb_grab_server(c);

    property_values[0] = XCB_WM_STATE_WITHDRAWN;
    property_values[1] = 0;
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, event->window, WM_STATE, WM_STATE, 32, 2, property_values);

    unmanage(event->event);

    xcb_flush(c);

    xcb_ungrab_server(c);
}

static void map_request(xcb_map_request_event_t * event)
{
    struct velox_window * window;
    xcb_get_window_attributes_cookie_t window_attributes_cookie;
    xcb_get_window_attributes_reply_t * window_attributes;

    DEBUG_ENTER

    window_attributes_cookie = xcb_get_window_attributes(c, event->window);

    window = lookup_window(event->window);

    window_attributes = xcb_get_window_attributes_reply(c, window_attributes_cookie, NULL);

    if (window_attributes)
    {
        if (window == NULL && !window_attributes->override_redirect)
        {
            manage(event->window);
        }
        else
        {
            DEBUG_PRINT("not managing window with id: 0x%x\n", event->window);
        }

        free(window_attributes);
    }
}

static void configure_notify(xcb_configure_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->window == screen->root)
    {
        screen_area.width = event->width;
        screen_area.height = event->height;

        arrange_all();

        run_hooks(NULL, VELOX_HOOK_ROOT_RESIZED);
    }
}

static void configure_request(xcb_configure_request_event_t * event)
{
    struct velox_window * window;

    DEBUG_ENTER

    window = lookup_window(event->window);

    /* Case 1 of the ICCCM 4.1.5 */
    if (window != NULL && !window->floating)
    {
        DEBUG_PRINT("configure_request: case 1\n")
        synthetic_configure(window);
    }
    /* Case 2 of the ICCCM 4.1.5 */
    else
    {
        uint16_t mask = 0;
        uint32_t values[7];
        uint8_t field = 0;

        DEBUG_PRINT("configure_request: case 2\n")

        if (event->value_mask & XCB_CONFIG_WINDOW_X)
        {
            values[field++] = event->x;
            mask |= XCB_CONFIG_WINDOW_X;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_Y)
        {
            values[field++] = event->y;
            mask |= XCB_CONFIG_WINDOW_Y;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_WIDTH)
        {
            values[field++] = event->width;
            mask |= XCB_CONFIG_WINDOW_WIDTH;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
        {
            values[field++] = event->height;
            mask |= XCB_CONFIG_WINDOW_HEIGHT;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_SIBLING)
        {
            values[field++] = event->sibling;
            mask |= XCB_CONFIG_WINDOW_SIBLING;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
        {
            values[field++] = event->stack_mode;
            mask |= XCB_CONFIG_WINDOW_STACK_MODE;
        }

        xcb_configure_window(c, event->window, mask, values);
    }

    xcb_flush(c);
}

static void property_notify(xcb_property_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->atom == XCB_ATOM_WM_NAME && event->state == XCB_PROPERTY_NEW_VALUE)
    {
        struct velox_window * window = lookup_window(event->window);

        if (window)
        {
            update_name_class(window);
            run_hooks(window, VELOX_HOOK_WINDOW_NAME_CHANGED);
        }
    }
    else
    {
        xcb_get_atom_name_cookie_t atom_name_cookie = xcb_get_atom_name(c, event->atom);
        xcb_get_atom_name_reply_t * atom_name_reply = xcb_get_atom_name_reply(c,
            atom_name_cookie, NULL);

        if (atom_name_reply)
        {
            char * atom_name = strndup(
                xcb_get_atom_name_name(atom_name_reply),
                xcb_get_atom_name_name_length(atom_name_reply)
            );
            DEBUG_PRINT("atom: %s\n", atom_name)
            free(atom_name);
        }

        free(atom_name_reply);
    }
}

static void client_message(xcb_client_message_event_t * event)
{
}

static void mapping_notify(xcb_mapping_notify_event_t * event)
{
    DEBUG_ENTER

    if (event->request == XCB_MAPPING_KEYBOARD)
    {
        grab_keys(event->first_keycode, event->first_keycode + event->count - 1);
    }
}

void setup_event_handlers()
{
    add_event_handler(XCB_KEY_PRESS,            &key_press);
    add_event_handler(XCB_BUTTON_PRESS,         &button_press);
    add_event_handler(XCB_ENTER_NOTIFY,         &enter_notify);
    add_event_handler(XCB_LEAVE_NOTIFY,         &leave_notify);
    add_event_handler(XCB_DESTROY_NOTIFY,       &destroy_notify);
    add_event_handler(XCB_UNMAP_NOTIFY,         &unmap_notify);
    add_event_handler(XCB_MAP_REQUEST,          &map_request);
    add_event_handler(XCB_CONFIGURE_NOTIFY,     &configure_notify);
    add_event_handler(XCB_CONFIGURE_REQUEST,    &configure_request);
    add_event_handler(XCB_PROPERTY_NOTIFY,      &property_notify);
    add_event_handler(XCB_CLIENT_MESSAGE,       &client_message);
    add_event_handler(XCB_MAPPING_NOTIFY,       &mapping_notify);
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

