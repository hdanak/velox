// vim: fdm=syntax fo=croql sw=4 sts=4 ts=8 et

/* mwm: mwm/config_file.h
 *
 * Copyright (c) 2010 Michael Forney <michael@obberon.com>
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

#include <yaml.h>
#include <assert.h>

#include "config_file.h"
#include "plugin.h"

FILE * open_config_file(const char * name)
{
    FILE * file;
    char path[1024];

    snprintf(path, sizeof(path), "%s/.mwm/%s", getenv("HOME"), name);
    if ((file = fopen(path, "r")) != NULL) return file;

    snprintf(path, sizeof(path), "/etc/mwm/%s", name);
    if ((file = fopen(path, "r")) != NULL) return file;

    return NULL;
}

void parse_config()
{
    FILE * file;
    char path[1024];

    yaml_parser_t parser;
    yaml_document_t document;

    file = open_config_file("mwm.yaml");

    if (file == NULL) return;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    if (yaml_parser_load(&parser, &document))
    {
        yaml_node_t * map;
        yaml_node_pair_t * pair;

        yaml_node_t * key, * value;

        map = document.nodes.start;
        assert(map->type == YAML_MAPPING_NODE);

        for (pair = map->data.mapping.pairs.start; pair < map->data.mapping.pairs.top; ++pair)
        {
            key = yaml_document_get_node(&document, pair->key);
            value = yaml_document_get_node(&document, pair->value);

            assert(key->type == YAML_SCALAR_NODE);

            if (strcmp(key->data.scalar.value, "plugins") == 0)
            {
                yaml_node_item_t * plugin_item;
                yaml_node_t * node;

                assert(value->type == YAML_SEQUENCE_NODE);

                for (plugin_item = value->data.sequence.items.start; plugin_item < value->data.sequence.items.top; ++plugin_item)
                {
                    node = yaml_document_get_node(&document, *plugin_item);

                    snprintf(path, sizeof(path), "%s/mwm_%s.so", getenv("MWM_PLUGIN_PATH"), node->data.scalar.value);

                    load_plugin(path);
                }
            }
        }

        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
    }
    else
    {
        printf("Error parsing document\n");
    }

    fclose(file);
}
