/* velox: modules/randr.c
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <yaml.h>
#include <xcb/randr.h>

#include <velox/velox.h>
#include <velox/monitor.h>
#include <velox/debug.h>

const char name[] = "randr";

void configure(yaml_document_t * document)
{
    yaml_node_t * map;
    yaml_node_pair_t * pair;

    yaml_node_t * key, * value;

    printf("RandR: Loading configuration...");

    map = yaml_document_get_root_node(document);
    assert(map->type == YAML_MAPPING_NODE);

    for (pair = map->data.mapping.pairs.start;
        pair < map->data.mapping.pairs.top;
        ++pair)
    {
        key = yaml_document_get_node(document, pair->key);
        value = yaml_document_get_node(document, pair->value);

        assert(key->type == YAML_SCALAR_NODE);
    }

    printf("done\n");
}

void setup()
{
    xcb_randr_query_version_cookie_t version_cookie;
    xcb_randr_query_version_reply_t * version_reply;
    xcb_randr_get_screen_info_cookie_t info_cookie;
    xcb_randr_get_screen_info_reply_t * info_reply;
    xcb_randr_get_screen_resources_cookie_t resources_cookie;
    xcb_randr_get_screen_resources_reply_t * resources_reply;
    xcb_randr_crtc_t * crtc;
    uint32_t index, crtcs_length;
    struct velox_area area;

    printf("RandR: Initializing...");

    version_cookie = xcb_randr_query_version(c, XCB_RANDR_MAJOR_VERSION,
            XCB_RANDR_MINOR_VERSION);
    version_reply = xcb_randr_query_version_reply(c, version_cookie, NULL);

    if (!version_reply)
        die("Could not initialize RandR extension");

    free(version_reply);

    xcb_randr_select_input(c, screen->root,
        XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);

    info_cookie = xcb_randr_get_screen_info(c, screen->root);
    info_reply = xcb_randr_get_screen_info_reply(c, info_cookie, NULL);

    resources_cookie = xcb_randr_get_screen_resources(c, screen->root);
    resources_reply = xcb_randr_get_screen_resources_reply(c,
        resources_cookie, NULL);

    crtc = xcb_randr_get_screen_resources_crtcs(resources_reply);
    crtcs_length = xcb_randr_get_screen_resources_crtcs_length(resources_reply);

    for (index = 0; index < crtcs_length; ++index, ++crtc)
    {
        xcb_randr_get_crtc_info_cookie_t crtc_cookie;
        xcb_randr_get_crtc_info_reply_t * crtc_reply;

        crtc_cookie = xcb_randr_get_crtc_info(c, *crtc, XCB_TIME_CURRENT_TIME);
        crtc_reply = xcb_randr_get_crtc_info_reply(c, crtc_cookie, NULL);

        area.x = crtc_reply->x;
        area.y = crtc_reply->y;
        area.width = crtc_reply->width;
        area.height = crtc_reply->height;

        add_monitor(&area);
    }

    printf("done\n");
}

void cleanup()
{
    printf("RandR: Cleaning up...");
    printf("done\n");
}

// vim: fdm=syntax fo=croql et sw=4 sts=4 ts=8

