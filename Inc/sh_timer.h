#ifndef __SH_TIMER_H__
#define __SH_TIMER_H__

#include <stdbool.h>
#include <stdint.h>

#include "sh_list.h"
#include "sh_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*sh_timer_get_tick_fn)(void);
typedef void (*overtick_cb_fn)(void*);

enum sh_timer_mode {
    SH_TIMER_MODE_SINGLE = 1,
    SH_TIMER_MODE_LOOP,
};

typedef struct sh_timer sh_timer_t;

int sh_timer_sys_init(sh_timer_get_tick_fn fn);
void sh_timer_init(sh_timer_t *timer, enum sh_timer_mode mode, overtick_cb_fn cb);
void sh_timer_set_param(sh_timer_t *timer, void *param);
void sh_timer_set_mode(sh_timer_t *timer, enum sh_timer_mode mode);
int sh_timer_start(sh_timer_t *timer, sh_list_t *head, uint32_t now, uint32_t interval_tick);
void sh_timer_restart(sh_timer_t *timer, sh_list_t *head, uint32_t now);
void sh_timer_stop(sh_timer_t *timer);
void sh_timer_handler(sh_list_t *head);
bool sh_timer_is_time_out(uint32_t now, uint32_t set_tick);
sh_timer_t* sh_timer_create(enum sh_timer_mode mode, overtick_cb_fn cb);
void sh_timer_destroy(sh_timer_t *timer);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

