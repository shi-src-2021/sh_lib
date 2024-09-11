#ifndef __SH_EVENT_H__
#define __SH_EVENT_H__

#include <stdint.h>

#include "sh_list.h"
#include "sh_event_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

enum sh_event_sub_mode {
    SH_EVENT_SUB_ASYNC = 0,
    SH_EVENT_SUB_SYNC,
};

struct sh_event_obj {
    sh_list_t       list;
    char            name[SH_EVENT_NAME_MAX];
};

typedef struct sh_event_obj sh_event_obj_t;

struct sh_event_map {
    sh_event_obj_t  obj;
    uint8_t         cnt;
};

typedef struct sh_event_map sh_event_map_t;

struct sh_event_type_table {
    uint8_t         event_id;
    char            name[SH_EVENT_NAME_MAX];
};

struct sh_event {
    uint8_t         id;
    void           *data;
    size_t          data_size;
};

typedef void(*event_cb)(const struct sh_event *e);

struct sh_event_server {
    sh_event_obj_t  obj;
    sh_list_t       event_queue;
    event_cb       *cb;
    uint8_t        *sub_mode;
    sh_event_map_t *map;
};

typedef struct sh_event_server sh_event_server_t;

sh_event_map_t* sh_event_map_create(struct sh_event_type_table *table, size_t size);
char* sh_event_get_event_id_name(struct sh_event_type_table *table, size_t size, uint8_t event_id);
sh_event_server_t* sh_event_server_create(sh_event_map_t *map, const char *name);
int sh_event_subscribe_sync(sh_event_server_t *server, uint8_t event_id, event_cb cb);
int sh_event_subscribe(sh_event_server_t *server, uint8_t event_id, event_cb cb);
int sh_event_unsubscribe(sh_event_server_t *server, uint8_t event_id);
int sh_event_publish(sh_event_map_t *map, uint8_t event_id);
int sh_event_publish_with_param(sh_event_map_t *map, uint8_t event_id, void *data, size_t size);
int sh_event_execute(sh_event_server_t *server);
int sh_event_server_clear_msg(sh_event_server_t *server);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

