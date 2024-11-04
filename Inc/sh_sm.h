#ifndef __SH_SM_H__
#define __SH_SM_H__

#include "sh_event.h"
#include "sh_timer.h"
#include "sh_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_SM_NAME_MAX   16

enum sh_sm_timer_type {
    SH_SM_PRIVATE_TIMER = 0,
    SH_SM_GLOBAL_TIMER,
};

typedef struct sh_sm sh_sm_t;

typedef uint32_t (*sh_get_tick_fn)(void);

sh_sm_t* sh_sm_create(uint8_t *event_buf, size_t size, sh_get_tick_fn fn);
void sh_sm_destroy(sh_sm_t *sm);
int sh_sm_state_create(sh_sm_t *sm, uint8_t state_id);
int sh_sm_state_destroy(sh_sm_t *sm, uint8_t state_id);
int sh_sm_state_subscribe_event(sh_sm_t *sm, uint8_t state_id, uint8_t event_id, event_cb cb);
int sh_sm_state_subscribe_events(sh_sm_t *sm, uint8_t state_id, uint8_t *event_buf, uint8_t event_cnt, event_cb cb);
int sh_sm_state_unsubscribe_event(sh_sm_t *sm, uint8_t state_id, uint8_t event_id);
int sh_sm_trans_to(sh_sm_t *sm, uint8_t state_id);
int sh_sm_handler(sh_sm_t *sm);
int sh_sm_publish_event(sh_sm_t *sm, uint8_t event_id);
int sh_sm_publish_event_with_param(sh_sm_t *sm, uint8_t event_id, size_t param);
int sh_sm_start_global_timer(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id);
int sh_sm_start_timer(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id);
int sh_sm_start_timer_with_param(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id, size_t param);
sh_timer_t* sh_sm_start_normal_timer(sh_sm_t *sm, uint32_t interval_tick, overtick_cb_fn cb);
int sh_sm_remove_timer(sh_sm_t *sm, enum sh_sm_timer_type type, uint8_t timer_id);
int sh_sm_remove_state_all_timer(sh_sm_t *sm, uint8_t state_id);
void sh_sm_remove_all_global_timer(sh_sm_t *sm);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
