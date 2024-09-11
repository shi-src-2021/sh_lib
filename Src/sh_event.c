#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "sh_event.h"
#include "sh_lib.h"

#ifndef SH_MALLOC
    #define SH_MALLOC       malloc
#endif

#ifndef SH_FREE
    #define SH_FREE         free
#endif

struct sh_event_node {
    sh_list_t       list;
    void           *data;
};

struct sh_event_handler {
    sh_event_obj_t  obj;
    uint8_t         id;
    sh_list_t       server;
};

struct sh_event_control {
    struct sh_event msg;
    size_t          ref;
};

static int sh_event_obj_init(sh_event_obj_t *obj, const char *name);
static struct sh_event_node* sh_event_node_create(void *data);
static struct sh_event_control* sh_event_msg_create(uint8_t event_id, void *data, size_t size);
static struct sh_event_handler *sh_event_get_handler_by_id(sh_event_map_t *map, uint8_t id);
static int sh_event_get_index_by_id(sh_event_map_t *map, uint8_t id, uint8_t *index);
static int __sh_event_execute(sh_event_server_t *server, bool is_cb_called);

sh_event_map_t* sh_event_map_create(struct sh_event_type_table *table, size_t size)
{
    sh_event_map_t *map = (sh_event_map_t*)SH_MALLOC(sizeof(sh_event_map_t));
    if (map == NULL) {
        return NULL;
    }

    sh_event_obj_init((sh_event_obj_t *)map, "event_map");
    map->cnt = (uint8_t)size;

    for (int i = 0; i < (int)size; i++) {
        struct sh_event_handler *handler = 
            (struct sh_event_handler*)SH_MALLOC(sizeof(struct sh_event_handler));

        if (handler == NULL) {
            return NULL;
        }

        sh_event_obj_init((sh_event_obj_t *)handler, table[i].name);
        handler->id = table[i].event_id;
        sh_list_init(&handler->server);

        sh_list_insert_before(&handler->obj.list, &map->obj.list);
    }

    return map;
}

char* sh_event_get_event_id_name(struct sh_event_type_table *table, size_t size, uint8_t event_id)
{
    if (table == NULL) {
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        if (table[i].event_id == event_id) {
            return table[i].name;
        }
    }

    return NULL;
}

static int sh_event_server_init(sh_event_server_t *server, sh_event_map_t *map, const char *name)
{
    if (map == NULL) {
        return -1;
    }

    if (server == NULL) {
        return -1;
    }

    for (int i = 0; i < map->cnt; i++) {
        server->cb[i]       = NULL;
        server->sub_mode[i] = SH_EVENT_SUB_ASYNC;
    }

    sh_list_init(&server->event_queue);

    server->map = map;

    return sh_event_obj_init((sh_event_obj_t *)server, name);
}

sh_event_server_t* sh_event_server_create(sh_event_map_t *map, const char *name)
{
    sh_event_server_t *server = SH_MALLOC(sizeof(sh_event_server_t));
    if (server == NULL) {
        return NULL;
    }

    event_cb *__cb = (event_cb*)SH_MALLOC(map->cnt * sizeof(event_cb));
    if (__cb == NULL) {
        return NULL;
    }

    uint8_t *__sub_mode = (uint8_t *)SH_MALLOC(map->cnt * sizeof(uint8_t));
    if (__sub_mode == NULL) {
        return NULL;
    }

    server->cb = __cb;
    server->sub_mode = __sub_mode;

    if (sh_event_server_init(server, map, name)) {
        SH_FREE(server);
        server = NULL;
    }

    return server;
}

static int __sh_event_subscribe(sh_event_server_t *server, uint8_t event_id, event_cb cb, uint8_t sub_mode)
{
    if (server == NULL) {
        return -1;
    }

    struct sh_event_handler *handler = sh_event_get_handler_by_id(server->map, event_id);
    if (handler == NULL) {
        return -1;
    }

    sh_list_for_each(node, &handler->server) {
        struct sh_event_node *server_node = sh_container_of(node, struct sh_event_node, list);
        if (server_node == NULL) {
            return -1;
        }
        sh_event_server_t *_server = (sh_event_server_t*)server_node->data;

        if (_server == server) {
            return 0;
        }
    }

    struct sh_event_node *new_server_node = sh_event_node_create(server);
    if (new_server_node == NULL) {
        return -1;
    }

    sh_list_insert_before(&new_server_node->list, &handler->server);

    uint8_t index = 0;
    if (sh_event_get_index_by_id(server->map, event_id, &index)) {
        return -1;
    }

    server->cb[index] = cb;
    server->sub_mode[index] = sub_mode;

    return 0;
}

int sh_event_subscribe_sync(sh_event_server_t *server, uint8_t event_id, event_cb cb)
{
    return __sh_event_subscribe(server, event_id, cb, SH_EVENT_SUB_SYNC);
}

int sh_event_subscribe(sh_event_server_t *server, uint8_t event_id, event_cb cb)
{
    return __sh_event_subscribe(server, event_id, cb, SH_EVENT_SUB_ASYNC);
}

int sh_event_unsubscribe(sh_event_server_t *server, uint8_t event_id)
{
    struct sh_event_handler *handler = sh_event_get_handler_by_id(server->map, event_id);
    if (handler == NULL) {
        return -1;
    }

    sh_list_for_each(node, &handler->server) {
        struct sh_event_node *server_node = sh_container_of(node, struct sh_event_node, list);
        if (server_node == NULL) {
            return -1;
        }
        sh_event_server_t *_server = (sh_event_server_t*)server_node->data;

        if (_server == server) {
            sh_list_remove(node);
            SH_FREE(server_node);
            server_node = NULL;

            uint8_t index = 0;
            if (sh_event_get_index_by_id(server->map, event_id, &index)) {
                return -1;
            }
            server->cb[index] = NULL;
            server->sub_mode[index] = SH_EVENT_SUB_ASYNC;
            return 0;
        }
    }

    return 0;
}

int sh_event_publish_with_param(sh_event_map_t *map, uint8_t event_id, void *data, size_t size)
{
    struct sh_event_handler *handler = sh_event_get_handler_by_id(map, event_id);
    if (handler == NULL) {
        return -1;
    }

    if (sh_list_isempty(&handler->server)) {
        return 0;
    }

    struct sh_event_control *msg_ctrl = sh_event_msg_create(event_id, data, size);
    if (msg_ctrl == NULL) {
        return -1;
    }

    sh_list_for_each(node, &handler->server) {
        struct sh_event_node *server_node = sh_container_of(node, struct sh_event_node, list);
        if (server_node == NULL) {
            return -1;
        }
        sh_event_server_t *server = (sh_event_server_t*)server_node->data;
        uint8_t index = 0;
        if (sh_event_get_index_by_id(server->map, event_id, &index)) {
            return -1;
        }
        if (server->sub_mode[index] == SH_EVENT_SUB_SYNC) {
            if (server->cb[index] != NULL) {
                server->cb[index](&msg_ctrl->msg);
            }
            continue;
        }

        struct sh_event_node *event_node = sh_event_node_create(msg_ctrl);
        if (event_node == NULL) {
            return -1;
        }

        msg_ctrl->ref++;

        sh_list_insert_before(&event_node->list, &server->event_queue);
    }

    if (msg_ctrl->ref == 0) {
        if (msg_ctrl->msg.data) {
            SH_FREE(msg_ctrl->msg.data);
            msg_ctrl->msg.data = NULL;
        }
        SH_FREE(msg_ctrl);
        msg_ctrl = NULL;
    }

    return 0;
}

int sh_event_publish(sh_event_map_t *map, uint8_t event_id)
{
    return sh_event_publish_with_param(map, event_id, NULL, 0);
}

int sh_event_execute(sh_event_server_t *server)
{
    return __sh_event_execute(server, true);
}

int sh_event_server_clear_msg(sh_event_server_t *server)
{
    return __sh_event_execute(server, false);
}

/* static function */

static int __sh_event_execute(sh_event_server_t *server, bool is_cb_called)
{
    if (server == NULL) {
        return -1;
    }

    if (sh_list_isempty(&server->event_queue)) {
        return 0;
    }

    sh_list_for_each_safe(node, &server->event_queue) {
        struct sh_event_node *event_node = sh_container_of(node, struct sh_event_node, list);
        if (event_node == NULL) {
            return -1;
        }
        struct sh_event *msg = (struct sh_event*)event_node->data;
        if (msg == NULL) {
            return -1;
        }

        uint8_t index = 0;
        if (sh_event_get_index_by_id(server->map, msg->id, &index)) {
            return -1;
        }
        if (is_cb_called) {
            if (server->cb[index] != NULL) {
                server->cb[index](msg);
            }
        }

        sh_list_remove(node);
        SH_FREE(event_node);
        event_node = NULL;
        ((struct sh_event_control *)msg)->ref--;
        if (((struct sh_event_control *)msg)->ref == 0) {
            if (msg->data) {
                SH_FREE(msg->data);
                msg->data = NULL;
            }
            SH_FREE(msg);
            msg = NULL;
        }
    }

    return 0;
}

static int sh_event_obj_init(sh_event_obj_t *obj, const char *name)
{
    if (strlen(name) >= SH_EVENT_NAME_MAX) {
        return -1;
    }

    strcpy(obj->name, name);
    sh_list_init(&obj->list);

    return 0;
}

static struct sh_event_node* sh_event_node_create(void *data)
{
    struct sh_event_node *node = (struct sh_event_node*)SH_MALLOC(sizeof(struct sh_event_node));
    if (node == NULL) {
        return NULL;
    }

    sh_list_init(&node->list);
    node->data = data;

    return node;
}

static struct sh_event_control* sh_event_msg_create(uint8_t event_id, void *data, size_t size)
{
    uint8_t *_data = NULL;

    struct sh_event_control *event_ctrl = 
        (struct sh_event_control*)SH_MALLOC(sizeof(struct sh_event_control));
    
    if (event_ctrl == NULL) {
        return NULL;
    }

    if (data) {
        _data = SH_MALLOC(size);
        if (_data == NULL) {
            return NULL;
        }
        memcpy(_data, data, size);
    }

    event_ctrl->msg.id          = event_id;
    event_ctrl->msg.data        = _data;
    event_ctrl->msg.data_size   = size;
    event_ctrl->ref             = 0;

    return event_ctrl;
}

static struct sh_event_handler *sh_event_get_handler_by_id(sh_event_map_t *map, uint8_t id)
{
    if (map == NULL) {
        return NULL;
    }

    sh_list_for_each(node, &map->obj.list) {
        struct sh_event_handler *handler = 
            (struct sh_event_handler*)sh_container_of(node, sh_event_obj_t, list);
            
        if (handler->id == id) {
            return handler;
        }
    }

    return NULL;
}

static int sh_event_get_index_by_id(sh_event_map_t *map, uint8_t id, uint8_t *index)
{
    uint8_t __index = 0;

    if (map == NULL) {
        return -1;
    }

    sh_list_for_each(node, &map->obj.list) {
        struct sh_event_handler *handler = 
            (struct sh_event_handler *)sh_container_of(node, sh_event_obj_t, list);

        if (handler->id == id) {
            *index = __index;
            return 0;
        }

        __index++;
    }

    return -1;
}
