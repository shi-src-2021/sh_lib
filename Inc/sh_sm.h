#ifndef __SH_SM_H__
#define __SH_SM_H__

#include "sh_event.h"
#include "sh_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_SM_NAME_MAX   16

struct sh_sm_state {
    sh_list_t list;
    uint8_t state_id;
    struct sh_sm_state *parent;
    sh_event_server_t *server;
    sh_list_t private_timer_head;
    uint32_t timer_bitmap;
};

typedef struct sh_sm_state sh_sm_state_t;

struct sh_sm {
    sh_list_t state_list;
    sh_list_t global_timer_head;
    sh_sm_state_t *current_state;
    sh_event_map_t *map;
    sh_timer_get_tick_fn timer_get_tick;
    uint32_t timer_bitmap;
};

typedef struct sh_sm sh_sm_t;

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
