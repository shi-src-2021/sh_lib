#ifndef __SH_EVENT_H__
#define __SH_EVENT_H__

#include <stdint.h>
#include <stdbool.h>

#include "sh_list.h"
#include "sh_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_EVENT_NAME_MAX   24

enum sh_event_sub_mode {
    SH_EVENT_SUB_ASYNC = 0,
    SH_EVENT_SUB_SYNC,
};

typedef struct sh_event_obj {
    sh_list_t       list;
    char            name[SH_EVENT_NAME_MAX];
} sh_event_obj_t;

typedef struct sh_event_map {
    sh_event_obj_t  obj;
    uint8_t         cnt;
} sh_event_map_t;

typedef struct sh_event_msg {
    uint8_t         id;
    void           *data;
    size_t          data_size;
} sh_event_msg_t;

typedef void(*event_cb)(const sh_event_msg_t *e);

typedef struct sh_event_server {
    sh_event_obj_t  obj;
    sh_list_t       event_queue;
    bool            enable;
    event_cb       *cb;
    uint8_t        *sub_mode;
    sh_event_map_t *map;
} sh_event_server_t;

sh_event_map_t* sh_event_map_create(uint8_t *table, size_t size);
void sh_event_map_destroy(sh_event_map_t *map);
sh_event_server_t* sh_event_server_create(sh_event_map_t *map, const char *name);
void sh_event_server_destroy(sh_event_server_t *server);
int sh_event_server_start(sh_event_server_t *server);
int sh_event_server_stop(sh_event_server_t *server);
int sh_event_subscribe_sync(sh_event_server_t *server, uint8_t event_id, event_cb cb);
int sh_event_subscribe(sh_event_server_t *server, uint8_t event_id, event_cb cb);
int sh_event_unsubscribe(sh_event_server_t *server, uint8_t event_id);
int sh_event_unsubscribe_all(sh_event_server_t *server);
int sh_event_publish(sh_event_map_t *map, uint8_t event_id);
int sh_event_publish_with_param(sh_event_map_t *map, uint8_t event_id, void *data, size_t size);
int sh_event_handler(sh_event_server_t *server);
int sh_event_server_clear_msg(sh_event_server_t *server);
int sh_event_server_get_msg_count(sh_event_server_t *server);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

