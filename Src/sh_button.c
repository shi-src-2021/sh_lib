#include <string.h>

#include "sh_button.h"
#include "sh_assert.h"
#include "sh_lib.h"

enum sh_button_system_state {
    SH_BUTTON_STATE_IDLE = 0,
    SH_BUTTON_STATE_PRESS,
    SH_BUTTON_STATE_RELEASE,
    SH_BUTTON_STATE_LONG_PRESS,
};

enum sh_button_press_state {
    SH_BUTTON_PRESS,
    SH_BUTTON_RELEASE,
};

int sh_button_init(struct sh_button *button,
                   char *name, 
                   enum sh_button_active_level active_level, 
                   get_button_level_fn get_button_level, 
                   uint8_t id)
{
    SH_ASSERT(button);
    SH_ASSERT(get_button_level);

    sh_list_init(&button->list);
    strncpy(button->name, name, SH_BUTTON_NAME_MAX_LEN - 1);
    button->ticks = 0;
    button->repeat = 0;
    button->state = SH_BUTTON_STATE_IDLE;
    button->debounce_cnt = 0;
    button->active_level = (!!active_level);
    button->current_level = !button->active_level;
    button->id = id;
    button->get_button_level = get_button_level;

    for (int i = 0; i < SH_BUTTON_EVENT_MAX; i++) {
        button->cb[i] = NULL;
    }

    return 0;
}

int sh_button_attach_cb(struct sh_button *button, 
                        enum sh_button_event event_id, 
                        button_cb_fn cb)
{
    SH_ASSERT(button);
    
    if (event_id >= SH_BUTTON_EVENT_MAX) {
        return -1;
    }

    button->cb[event_id] = cb;

    return 0;
}

int sh_button_detach_cb(struct sh_button *button,
                        enum sh_button_event event_id)
{
    SH_ASSERT(button);

    return sh_button_attach_cb(button, event_id, NULL);
}

int sh_button_ctrl_init(sh_button_ctrl_t *button_ctrl,
                        uint8_t invoke_interval_ms,
                        uint16_t release_timeout_ms,
                        uint16_t long_press_ms,
                        uint8_t long_press_repeat_ticks,
                        uint8_t debounce_ticks)
{
    SH_ASSERT(button_ctrl);
    SH_ASSERT(invoke_interval_ms);
    SH_ASSERT(release_timeout_ms);
    SH_ASSERT(long_press_ms);

    sh_list_init(&button_ctrl->head);
    button_ctrl->invoke_interval_ms = invoke_interval_ms;
    button_ctrl->debounce_ticks = debounce_ticks;
    button_ctrl->release_timeout_ticks = 
        (release_timeout_ms + invoke_interval_ms - 1) / invoke_interval_ms;
    button_ctrl->long_press_ticks = 
        (long_press_ms + invoke_interval_ms - 1) / invoke_interval_ms;
    button_ctrl->long_press_repeat_ticks = long_press_repeat_ticks;

    return 0;
}

int sh_button_ctrl_default_init(sh_button_ctrl_t *button_ctrl)
{
    SH_ASSERT(button_ctrl);

    sh_list_init(&button_ctrl->head);
    button_ctrl->invoke_interval_ms = 1;
    button_ctrl->debounce_ticks = 8;
    button_ctrl->release_timeout_ticks = 300;
    button_ctrl->long_press_ticks = 1000;
    button_ctrl->long_press_repeat_ticks = 200;

    return 0;
}

int sh_button_ctrl_add(sh_button_ctrl_t *button_ctrl, struct sh_button *button)
{
    SH_ASSERT(button_ctrl);
    SH_ASSERT(button);

    sh_list_insert_before(&button->list, &button_ctrl->head);

    return 0;
}

static enum sh_button_press_state 
sh_button_get_press_state(struct sh_button *button)
{
    SH_ASSERT(button);

    return button->current_level == button->active_level ? 
                SH_BUTTON_PRESS : SH_BUTTON_RELEASE;
}

static int sh_button_debounce(struct sh_button *button, uint8_t debounce_ticks)
{
    SH_ASSERT(button);

    if (button->current_level != button->get_button_level(button->id)) {
        button->debounce_cnt++;
        if (button->debounce_cnt >= debounce_ticks) {
            button->current_level ^= 1;
        }
    } else {
        button->debounce_cnt = 0;
    }

    return 0;
}

static int sh_button_invoke_cb(struct sh_button *button,
                               enum sh_button_event event_id)
{
    SH_ASSERT(button);
    
    if (event_id >= SH_BUTTON_EVENT_MAX) {
        return -1;
    }

    button->event = event_id;

    if (button->cb[event_id]) {
        button->cb[event_id](button);
    }

    return 0;
}

static void sh_button_trans_to_state(struct sh_button *button, uint8_t state)
{
    button->state = state;
    button->ticks = 0;
}

#define invoke_cd_and_update_state(event, state)                        \
            do {                                                        \
                sh_button_invoke_cb(button, SH_BUTTON_##event);         \
                sh_button_trans_to_state(button, SH_BUTTON_##state);    \
            } while (0);
            

static void sh_button_state_machine_process(struct sh_button *button,
                                            sh_button_ctrl_t *button_ctrl)
{
    SH_ASSERT(button);
    SH_ASSERT(button_ctrl);
    
    switch (button->state)
    {
    case SH_BUTTON_STATE_IDLE:
        if (sh_button_get_press_state(button) == SH_BUTTON_PRESS) {
            invoke_cd_and_update_state(EVENT_PRESS, STATE_PRESS);
            button->repeat = 1;
        }
        break;

    case SH_BUTTON_STATE_PRESS:
        button->ticks++;
        if (sh_button_get_press_state(button) == SH_BUTTON_RELEASE) {
            invoke_cd_and_update_state(EVENT_RELEASE, STATE_RELEASE);
        }
        if (button->ticks >= button_ctrl->long_press_ticks) {
            invoke_cd_and_update_state(EVENT_LONG_PRESS_START, STATE_LONG_PRESS);
        }
        break;

    case SH_BUTTON_STATE_RELEASE:
        button->ticks++;
        if (sh_button_get_press_state(button) == SH_BUTTON_PRESS) {
            if (button->repeat < SH_BUTTON_REPEAT_MAX) {
                button->repeat++;
            }
            invoke_cd_and_update_state(EVENT_PRESS, STATE_PRESS);
        }
        if (button->ticks >=  button_ctrl->release_timeout_ticks) {
            if (button->repeat == 1) {
                invoke_cd_and_update_state(EVENT_SINGLE_CLICK, STATE_IDLE);
            } else if (button->repeat == 2) {
                invoke_cd_and_update_state(EVENT_DOUBLE_CLICK, STATE_IDLE);
            } else {
                invoke_cd_and_update_state(EVENT_MULTI_CLICK, STATE_IDLE);
            }
        }
        
        break;
    
    case SH_BUTTON_STATE_LONG_PRESS:
        button->ticks++;
        if (sh_button_get_press_state(button) == SH_BUTTON_RELEASE) {
            invoke_cd_and_update_state(EVENT_RELEASE, STATE_IDLE);
        }
        if (button->ticks >=  button_ctrl->long_press_repeat_ticks) {
            invoke_cd_and_update_state(EVENT_LONG_PRESS_REPEAT, STATE_LONG_PRESS);
        }
        
        break;
    
    default:
        break;
    }
}

int sh_button_handler(sh_button_ctrl_t *button_ctrl)
{
    SH_ASSERT(button_ctrl);

    if (sh_list_isempty(&button_ctrl->head)) {
        return 0;
    }
    
    sh_list_for_each(node, &button_ctrl->head) {
        struct sh_button *button = 
            sh_container_of(node, struct sh_button, list);
        
        sh_button_debounce(button, button_ctrl->debounce_ticks);
        sh_button_state_machine_process(button, button_ctrl);
    }

    return 0;
}

uint8_t sh_button_get_level(struct sh_button *button)
{
    SH_ASSERT(button);
    SH_ASSERT(button->get_button_level);

    return button->get_button_level(button->id);
}



