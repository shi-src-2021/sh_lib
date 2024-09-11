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

void sh_timer_init(sh_timer_t *self, enum sh_timer_mode mode, overtick_cb_fn cb)
{
    SH_ASSERT(self);

    self->cb = cb;
    self->mode = mode;
    self->param = NULL;
    self->enable = false;
    self->interval_tick = 0;
    self->overtick = 0;

    sh_list_init(&self->list);
}

void sh_timer_set_param(sh_timer_t *self, void *param)
{
    SH_ASSERT(self);
    
    self->param = param;
}

void sh_timer_set_mode(sh_timer_t *self, enum sh_timer_mode mode)
{
    SH_ASSERT(self);
    
    self->mode = mode;
}

int sh_timer_start(sh_timer_t *self, uint32_t now, uint32_t interval_tick)
{
    SH_ASSERT(self);
    SH_ASSERT(interval_tick);

    sh_list_remove(&self->list);

    if (interval_tick > (UINT32_MAX / 2)) {
        return -1;
    }

    self->enable = true;
    self->interval_tick = interval_tick;
    self->overtick = now + interval_tick;

    sh_list_for_each(node, &head) {
        sh_timer_t *timer = sh_container_of(node, sh_timer_t, list);
        if (self->overtick < timer->overtick) {
            sh_list_insert_before(&self->list, node);
            return 0;
        }
    }
    sh_list_insert_before(&self->list, &head);

    return 0;
}

void sh_timer_restart(sh_timer_t *self, uint32_t now)
{
    SH_ASSERT(self);
    
    sh_timer_start(self, now, self->interval_tick);
}

void sh_timer_stop(sh_timer_t *self)
{
    SH_ASSERT(self);
    
    self->enable = false;

    sh_list_remove(&self->list);
}

void sh_timer_print(uint32_t current_tick)
{
    sh_list_for_each(node, &head) {
        sh_timer_t *timer = sh_container_of(node, sh_timer_t, list);
        printf("%s: %5d -> ", (char*)timer->param, timer->overtick - current_tick);
    }
    printf("\r\n");
}

void sh_timer_loop(void)
{
    SH_ASSERT(sh_timer_get_tick);

    uint32_t current_tick = sh_timer_get_tick();

restart:
    sh_list_for_each(node, &head) {
        sh_timer_t *timer = sh_container_of(node, sh_timer_t, list);
        if ((int32_t)(current_tick - timer->overtick) < 0) {
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


