#ifndef __SH_SM_H__
#define __SH_SM_H__

#include "sh_timer.h"
#include "sh_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_SM_NAME_MAX   16

struct sh_timer_node {
    sh_list_t list;
    sh_timer_t *timer;
};

struct sh_sm_state {
    sh_event_server_t *server;
    sh_list_t timer_queue;
};

struct sh_sm_ctrl {
    struct sh_sm_state *current_state;
    sh_list_t timer_head;
};

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
