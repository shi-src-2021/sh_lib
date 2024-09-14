#ifndef __SH_BUTTON_H__
#define __SH_BUTTON_H__

#include <stdint.h>

#include "sh_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_BUTTON_NAME_MAX_LEN  16
#define SH_BUTTON_REPEAT_MAX    8

enum sh_button_event {
    SH_BUTTON_EVENT_PRESS = 0,
    SH_BUTTON_EVENT_RELEASE,
    SH_BUTTON_EVENT_SINGLE_CLICK,
    SH_BUTTON_EVENT_DOUBLE_CLICK,
    SH_BUTTON_EVENT_MULTI_CLICK,
    SH_BUTTON_EVENT_LONG_PRESS_START,
    SH_BUTTON_EVENT_LONG_PRESS_REPEAT,
    SH_BUTTON_EVENT_MAX,
    SH_BUTTON_EVENT_NONE,
};

typedef uint8_t (*get_button_level_fn)(uint8_t id);

struct sh_button;
typedef void (*button_cb_fn)(struct sh_button*);

struct sh_button {
    char        name[SH_BUTTON_NAME_MAX_LEN];
    sh_list_t   list;
    uint16_t    ticks;
	uint8_t     debounce_cnt : 4;
	uint8_t     state : 3;
	uint8_t     active_level : 1;
	uint8_t     current_level : 1;
    uint8_t     repeat : 3;
    uint8_t     id;
    button_cb_fn cb[SH_BUTTON_EVENT_MAX];
    get_button_level_fn get_button_level;
};

struct sh_button_ctrl {
    sh_list_t   head;
    uint8_t     invoke_interval_ms;
    uint8_t     debounce_ticks;
    uint16_t    release_timeout_ticks;
    uint16_t    long_press_ticks;
    uint8_t     long_press_repeat_ticks;
};

typedef struct sh_button_ctrl sh_button_ctrl_t;

int sh_button_ctrl_init(sh_button_ctrl_t *button_ctrl,
                        uint8_t invoke_interval_ms,
                        uint16_t release_timeout_ms,
                        uint16_t long_press_ms,
                        uint8_t long_press_repeat_ticks,
                        uint8_t debounce_ticks);
int sh_button_ctrl_add(sh_list_t *head, struct sh_button *button);
int sh_button_init(struct sh_button *button,
                   char *name, 
                   uint8_t active_level, 
                   get_button_level_fn get_button_level, 
                   uint8_t id);
int sh_button_attach_cb(struct sh_button *button, 
                        enum sh_button_event event_id, 
                        button_cb_fn cb);
int sh_button_detach_cb(struct sh_button *button,
                        enum sh_button_event event_id);
int sh_button_handler(sh_button_ctrl_t *button_ctrl);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

