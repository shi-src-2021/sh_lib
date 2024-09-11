#include <stdbool.h>

#include "sh_lib.h"
#include "sh_timer.h"
#include "sh_assert.h"

static sh_timer_get_tick_fn sh_timer_get_tick = NULL;

static SH_LIST_INIT(head);

int sh_timer_sys_init(sh_timer_get_tick_fn fn)
{
    SH_ASSERT(fn);
    
    sh_timer_get_tick = fn;

    return 0;
}

void sh_timer_init(sh_timer_t *timer, enum sh_timer_mode mode, overtick_cb_fn cb)
{
    SH_ASSERT(timer);

    timer->cb = cb;
    timer->mode = mode;
    timer->param = NULL;
    timer->enable = false;
    timer->interval_tick = 0;
    timer->overtick = 0;

    sh_list_init(&timer->list);
}

void sh_timer_set_param(sh_timer_t *timer, void *param)
{
    SH_ASSERT(timer);
    
    timer->param = param;
}

void sh_timer_set_mode(sh_timer_t *timer, enum sh_timer_mode mode)
{
    SH_ASSERT(timer);
    
    timer->mode = mode;
}

int sh_timer_start(sh_timer_t *timer, uint32_t now, uint32_t interval_tick)
{
    SH_ASSERT(timer);
    SH_ASSERT(interval_tick);

    sh_list_remove(&timer->list);

    if (interval_tick > (UINT32_MAX / 2)) {
        return -1;
    }

    timer->enable = true;
    timer->interval_tick = interval_tick;
    timer->overtick = now + interval_tick;

    sh_list_for_each(node, &head) {
        sh_timer_t *timer = sh_container_of(node, sh_timer_t, list);
        if (timer->overtick < timer->overtick) {
            sh_list_insert_before(&timer->list, node);
            return 0;
        }
    }
    sh_list_insert_before(&timer->list, &head);

    return 0;
}

void sh_timer_restart(sh_timer_t *timer, uint32_t now)
{
    SH_ASSERT(timer);
    
    sh_timer_start(timer, now, timer->interval_tick);
}

void sh_timer_stop(sh_timer_t *timer)
{
    SH_ASSERT(timer);
    
    timer->enable = false;

    sh_list_remove(&timer->list);
}

bool sh_timer_is_time_out(uint32_t now, uint32_t set_tick)
{
    return ((int32_t)(now - set_tick) >= 0);
}

void sh_timer_loop(void)
{
    SH_ASSERT(sh_timer_get_tick);

    uint32_t current_tick = sh_timer_get_tick();

restart:
    sh_list_for_each(node, &head) {
        sh_timer_t *timer = sh_container_of(node, sh_timer_t, list);
        if (!sh_timer_is_time_out(current_tick, timer->overtick)) {
            return;
        }
        sh_timer_stop(timer);
        if (timer->mode == SH_TIMER_MODE_LOOP) {
            sh_timer_restart(timer, current_tick);
        }
        if (timer->cb) {
            timer->cb(timer->param);
            goto restart;
        }
    }
}


