#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "sh_event.h"
#include "sh_lib.h"
#include "sh_assert.h"

#ifndef SH_MALLOC
    #define SH_MALLOC   malloc
#endif

#ifndef SH_FREE
    #define SH_FREE     free
#endif

struct sh_event_list_node {
    sh_list_t       list;
    void           *data;
};

typedef struct sh_event_list_node sh_event_list_node_t;

struct sh_event {
    sh_event_obj_t  obj;
    uint8_t         id;
    sh_list_t       server;
};

typedef struct sh_event sh_event_t;

struct sh_event_msg_ctrl {
    sh_event_msg_t  msg;
    size_t          ref;
};

typedef struct sh_event_msg_ctrl sh_event_msg_ctrl_t;

static int sh_event_obj_init(sh_event_obj_t *obj, const char *name);
static sh_event_list_node_t* sh_event_list_node_create(void *data);
static sh_event_t *sh_event_get_event_by_id(sh_event_map_t *map, uint8_t id);
static int sh_event_get_index_by_id(sh_event_map_t *map, uint8_t id, uint8_t *index);
static int _sh_event_execute(sh_event_server_t *server, bool is_cb_called);

sh_event_map_t* sh_event_map_create(struct sh_event_type_table *table, size_t size)
{
    SH_ASSERT(table);
    
    sh_event_map_t *map = (sh_event_map_t*)SH_MALLOC(sizeof(sh_event_map_t));
    if (map == NULL) {
        return NULL;
    }

    sh_event_obj_init((sh_event_obj_t *)map, "event_map");
    map->cnt = (uint8_t)size;

    for (int i = 0; i < (int)size; i++) {
        sh_event_t *event = (sh_event_t*)SH_MALLOC(sizeof(sh_event_t));
        if (event == NULL) {
            return NULL;
        }

        sh_event_obj_init((sh_event_obj_t *)event, table[i].name);
        event->id = table[i].event_id;
        sh_list_init(&event->server);

        sh_list_insert_before(&event->obj.list, &map->obj.list);
    }

    return map;
}

/**
 * must destroy all servers that created with this map before destroy the map.
 */
void sh_event_map_destroy(sh_event_map_t *map)
{
    if (map == NULL) {
        return;
    }

    sh_list_for_each_safe(node, &map->obj.list) {
        sh_event_t *event = (sh_event_t*)sh_container_of(node, sh_event_obj_t, list);
        SH_FREE(event);
    }

    SH_FREE(map);
}

static int sh_event_server_init(sh_event_server_t *server, 
                                sh_event_map_t *map, 
                                const char *name)
{
    SH_ASSERT(server);
    SH_ASSERT(map);

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
    SH_ASSERT(map);
    SH_ASSERT(name);

    sh_event_server_t *server = SH_MALLOC(sizeof(sh_event_server_t));
    if (server == NULL) {
        return NULL;
    }

    event_cb *_cb = (event_cb*)SH_MALLOC(map->cnt * sizeof(event_cb));
    if (_cb == NULL) {
        goto free_server;
    }

    uint8_t *_sub_mode = (uint8_t *)SH_MALLOC(map->cnt * sizeof(uint8_t));
    if (_sub_mode == NULL) {
        goto free_cb;
    }

    server->cb = _cb;
    server->sub_mode = _sub_mode;

    if (sh_event_server_init(server, map, name)) {
        goto free_sub_mode;
    }

    return server;
    
free_sub_mode:
    SH_FREE(_sub_mode);
free_cb:
    SH_FREE(_cb);
free_server:
    SH_FREE(server);

    return NULL;
}

sh_event_list_node_t *sh_event_server_list_find_node(sh_list_t *server_list,
                                                     sh_event_server_t *server)
{
    SH_ASSERT(server_list);
    SH_ASSERT(server);

    sh_list_for_each(node, server_list) {
        sh_event_list_node_t *server_node = 
            sh_container_of(node, sh_event_list_node_t, list);
            
        SH_ASSERT(server_node);
        
        sh_event_server_t *_server = (sh_event_server_t*)server_node->data;

        if (_server == server) {
            return server_node;
        }
    }

    return NULL;
}

void sh_event_list_node_destroy(sh_event_list_node_t *list_node)
{
    sh_list_remove(&list_node->list);
    SH_FREE(list_node);
}

void sh_event_server_destroy(sh_event_server_t *server)
{
    if (server == NULL) {
        return;
    }
    
    SH_ASSERT(server->map);

    sh_list_for_each(node, &server->map->obj.list) {
        sh_event_t *event = (sh_event_t*)sh_container_of(node, sh_event_obj_t, list);
        sh_event_list_node_t *server_node = 
            sh_event_server_list_find_node(&event->server, server);
        if (server_node == NULL) {
            continue;
        }

        sh_event_list_node_destroy(server_node);
        server_node = NULL;
    }

    SH_FREE(server->sub_mode);
    SH_FREE(server->cb);
    SH_FREE(server);
}

static int _sh_event_subscribe(sh_event_server_t *server, 
                               uint8_t event_id, 
                               event_cb cb, 
                               uint8_t sub_mode)
{
    SH_ASSERT(server);

    sh_event_t *event = sh_event_get_event_by_id(server->map, event_id);
    if (event == NULL) {
        return -1;
    }

    if (sh_event_server_list_find_node(&event->server, server)) {
        return 0;
    }

    sh_event_list_node_t *server_node = sh_event_list_node_create(server);
    if (server_node == NULL) {
        return -1;
    }

    sh_list_insert_before(&server_node->list, &event->server);

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
    SH_ASSERT(server);
    
    return _sh_event_subscribe(server, event_id, cb, SH_EVENT_SUB_SYNC);
}

int sh_event_subscribe(sh_event_server_t *server, uint8_t event_id, event_cb cb)
{
    SH_ASSERT(server);
    
    return _sh_event_subscribe(server, event_id, cb, SH_EVENT_SUB_ASYNC);
}

int sh_event_unsubscribe(sh_event_server_t *server, uint8_t event_id)
{
    SH_ASSERT(server);
    
    sh_event_t *event = sh_event_get_event_by_id(server->map, event_id);
    if (event == NULL) {
        return -1;
    }

    sh_event_list_node_t *server_node = 
        sh_event_server_list_find_node(&event->server, server);
    if (server_node == NULL) {
        return 0;
    }

    sh_event_list_node_destroy(server_node);
    server_node = NULL;

    uint8_t index = 0;
    if (sh_event_get_index_by_id(server->map, event_id, &index)) {
        return -1;
    }
    server->cb[index] = NULL;
    server->sub_mode[index] = SH_EVENT_SUB_ASYNC;

    return 0;
}

static bool sh_event_execute_sync_cb(sh_event_server_t *server,
                                     uint8_t index, 
                                     sh_event_msg_t *msg)
{
    SH_ASSERT(server);
    SH_ASSERT(msg);
    
    if (server->sub_mode[index] == SH_EVENT_SUB_SYNC) {
        if (server->cb[index] != NULL) {
            server->cb[index](msg);
        }
        return true;
    }

    return false;
}

static int sh_event_server_save_msg(sh_event_server_t *server,
                                    sh_event_msg_ctrl_t *msg_ctrl)
{
    SH_ASSERT(server);
    SH_ASSERT(msg_ctrl);
    
    sh_event_list_node_t *event_node = sh_event_list_node_create(msg_ctrl);
    if (event_node == NULL) {
        return -1;
    }

    msg_ctrl->ref++;

    sh_list_insert_before(&event_node->list, &server->event_queue);

    return 0;
}

static void sh_event_check_if_msg_needs_to_free(sh_event_msg_ctrl_t *msg_ctrl)
{
    SH_ASSERT(msg_ctrl);
    
    if (msg_ctrl->ref == 0) {
        if (msg_ctrl->msg.data) {
            SH_FREE(msg_ctrl->msg.data);
            msg_ctrl->msg.data = NULL;
        }
        SH_FREE(msg_ctrl);
        msg_ctrl = NULL;
    }
}

static sh_event_msg_ctrl_t* sh_event_msg_create(uint8_t event_id, 
                                                void *data, 
                                                size_t size)
{
    uint8_t *_data = NULL;

    sh_event_msg_ctrl_t *event_ctrl = 
        (sh_event_msg_ctrl_t*)SH_MALLOC(sizeof(sh_event_msg_ctrl_t));
    
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

int sh_event_publish_with_param(sh_event_map_t *map,
                                uint8_t event_id, 
                                void *data, 
                                size_t size)
{
    SH_ASSERT(map);

    sh_event_t *event = sh_event_get_event_by_id(map, event_id);
    if (event == NULL) {
        return -1;
    }

    if (sh_list_isempty(&event->server)) {
        return 0;
    }

    sh_event_msg_ctrl_t *msg_ctrl = sh_event_msg_create(event_id, data, size);
    if (msg_ctrl == NULL) {
        return -1;
    }

    sh_list_for_each(node, &event->server) {
        sh_event_list_node_t *server_node = 
            sh_container_of(node, sh_event_list_node_t, list);

        sh_event_server_t *server = (sh_event_server_t*)server_node->data;

        uint8_t index = 0;
        if (sh_event_get_index_by_id(server->map, event_id, &index)) {
            return -1;
        }

        if (sh_event_execute_sync_cb(server, index, &msg_ctrl->msg)) {
            continue;
        }

        if (sh_event_server_save_msg(server, msg_ctrl)) {
            return -1;
        }
    }

    sh_event_check_if_msg_needs_to_free(msg_ctrl);

    return 0;
}

int sh_event_publish(sh_event_map_t *map, uint8_t event_id)
{
    SH_ASSERT(map);
    
    return sh_event_publish_with_param(map, event_id, NULL, 0);
}

int sh_event_execute(sh_event_server_t *server)
{
    SH_ASSERT(server);
    
    return _sh_event_execute(server, true);
}

int sh_event_server_clear_msg(sh_event_server_t *server)
{
    SH_ASSERT(server);
    
    return _sh_event_execute(server, false);
}

/* static function */

static int sh_event_execute_async_cb(sh_event_server_t   *server,
                                     sh_event_msg_ctrl_t *msg_ctrl,
                                     bool                 is_cb_called)
{
    SH_ASSERT(server);
    SH_ASSERT(msg_ctrl);
    
    uint8_t index = 0;

    if (sh_event_get_index_by_id(server->map, msg_ctrl->msg.id, &index)) {
        return -1;
    }
    if (is_cb_called) {
        if (server->cb[index] != NULL) {
            server->cb[index](&msg_ctrl->msg);
        }
    }

    return 0;
}

static int _sh_event_execute(sh_event_server_t *server, bool is_cb_called)
{
    SH_ASSERT(server);
    
    if (sh_list_isempty(&server->event_queue)) {
        return 0;
    }

    sh_list_for_each_safe(node, &server->event_queue) {
        sh_event_list_node_t *event_node = 
            sh_container_of(node, sh_event_list_node_t, list);

        sh_event_msg_ctrl_t *msg_ctrl = (sh_event_msg_ctrl_t*)event_node->data;
        if (msg_ctrl == NULL) {
            return -1;
        }

        if (sh_event_execute_async_cb(server, msg_ctrl, is_cb_called)) {
            return -1;
        }

        sh_list_remove(node);

        SH_FREE(event_node);
        event_node = NULL;

        msg_ctrl->ref--;
        sh_event_check_if_msg_needs_to_free(msg_ctrl);
    }

    return 0;
}

static int sh_event_obj_init(sh_event_obj_t *obj, const char *name)
{
    SH_ASSERT(obj);
    
    if (strlen(name) >= SH_EVENT_NAME_MAX) {
        return -1;
    }

    strcpy(obj->name, name);
    sh_list_init(&obj->list);

    return 0;
}

static sh_event_list_node_t* sh_event_list_node_create(void *data)
{
    sh_event_list_node_t *node = 
        (sh_event_list_node_t*)SH_MALLOC(sizeof(sh_event_list_node_t));

    if (node == NULL) {
        return NULL;
    }

    sh_list_init(&node->list);
    node->data = data;

    return node;
}

static sh_event_t *sh_event_get_event_by_id(sh_event_map_t *map, uint8_t id)
{
    SH_ASSERT(map);
    
    sh_list_for_each(node, &map->obj.list) {
        sh_event_t *event = (sh_event_t*)sh_container_of(node, sh_event_obj_t, list);
            
        if (event->id == id) {
            return event;
        }
    }

    return NULL;
}

static int sh_event_get_index_by_id(sh_event_map_t *map, uint8_t id, uint8_t *index)
{
    SH_ASSERT(map);
    
    uint8_t _index = 0;

    sh_list_for_each(node, &map->obj.list) {
        sh_event_t *event = (sh_event_t*)sh_container_of(node, sh_event_obj_t, list);

        if (event->id == id) {
            *index = _index;
            return 0;
        }

        _index++;
    }

    return -1;
}

int sh_event_server_get_msg_count(sh_event_server_t *server)
{
    if (server == NULL) {
        return -1;
    }

    int cnt = 0;

    sh_list_for_each(node, &server->event_queue) {
        cnt++;
    }

    return cnt;
}

char* sh_event_get_event_id_name(struct sh_event_type_table *table, 
                                 size_t size, 
                                 uint8_t event_id)
{
    SH_ASSERT(table);
    
    for (int i = 0; i < size; i++) {
        if (table[i].event_id == event_id) {
            return table[i].name;
        }
    }

    return NULL;
}


