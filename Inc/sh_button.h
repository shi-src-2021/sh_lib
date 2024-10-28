#ifndef __SH_BUTTON_H__
#define __SH_BUTTON_H__

#include <stdint.h>

#include "sh_list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_BUTTON_NAME_MAX_LEN  16
#define SH_BUTTON_REPEAT_MAX    7

enum sh_button_active_level {
    SH_BUTTON_ACTIVE_LOW = 0,
    SH_BUTTON_ACTIVE_HIGH,
};

enum sh_button_press_state {
    SH_BUTTON_PRESS,
    SH_BUTTON_RELEASE,
};

enum sh_button_event {
    SH_BUTTON_EVENT_PRESS = 0,
    SH_BUTTON_EVENT_RELEASE,
    SH_BUTTON_EVENT_SINGLE_CLICK,
    SH_BUTTON_EVENT_DOUBLE_CLICK,
    SH_BUTTON_EVENT_MULTI_CLICK,
    SH_BUTTON_EVENT_LONG_PRESS_START,
    SH_BUTTON_EVENT_LONG_PRESS_REPEAT,
    SH_BUTTON_EVENT_MAX,
};

typedef uint8_t (*get_button_level_fn)(uint8_t id);

typedef struct sh_button sh_button_t;
typedef void (*button_cb_fn)(sh_button_t *button, enum sh_button_event event_id);

typedef struct sh_button_config {
    char        name[SH_BUTTON_NAME_MAX_LEN];
    uint8_t     id;
    enum sh_button_active_level active_level;
    get_button_level_fn         get_button_level;
} sh_button_config_t;

struct sh_button {
    char        name[SH_BUTTON_NAME_MAX_LEN];
    sh_list_t   list;
    uint16_t    ticks;
	uint8_t     debounce_cnt : 4;
    uint8_t     event : 4;
	uint8_t     state : 3;
    uint8_t     repeat : 3;
	uint8_t     active_level : 1;
	uint8_t     current_level : 1;
    uint8_t     id;
    button_cb_fn cb[SH_BUTTON_EVENT_MAX];
    get_button_level_fn get_button_level;
};

typedef struct sh_button_ctrl {
    sh_list_t   head;
    uint8_t     debounce_ticks;
    uint16_t    release_timeout_ticks;
    uint16_t    long_press_ticks;
    uint8_t     long_press_repeat_ticks;
} sh_button_ctrl_t;

int sh_button_ctrl_init(sh_button_ctrl_t *button_ctrl);
int sh_button_ctrl_add(sh_button_ctrl_t *button_ctrl, sh_button_t *button);
int sh_button_init(sh_button_t *button, sh_button_config_t *config);
int sh_button_attach_cb(sh_button_t *button, enum sh_button_event event_id, button_cb_fn cb);
int sh_button_detach_cb(sh_button_t *button, enum sh_button_event event_id);
int sh_button_handler(sh_button_ctrl_t *button_ctrl);
uint8_t sh_button_get_level(sh_button_t *button);
enum sh_button_press_state sh_button_get_current_press_state(sh_button_t *button);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

