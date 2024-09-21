#include "sh_sm.h"
#include "sh_timer.h"
#include "sh_assert.h"
#include "sh_lib.h"

#ifndef SH_MALLOC
    #define SH_MALLOC   malloc
#endif

#ifndef SH_FREE
    #define SH_FREE     free
#endif

enum sh_sm_timer_type {
    SH_SM_PRIVATE_TIMER = 0,
    SH_SM_GLOBAL_TIMER,
};

struct sh_timer_node {
    sh_timer_t *timer;
    uint32_t current_cnt;
    uint32_t destroy_cnt;
    uint8_t event_id;
    sh_sm_t *sm;
    uint8_t timer_id;
};

typedef struct sh_timer_node sh_timer_node_t;

int sh_sm_init(sh_sm_t *sm, sh_event_map_t *map, sh_timer_get_tick_fn fn)
{
    SH_ASSERT(sm);
    SH_ASSERT(map);
    SH_ASSERT(fn);

    sh_list_init(&sm->global_timer_head);
    sh_list_init(&sm->state_list);
    sm->current_state = NULL;
    sm->map = map;
    sm->timer_get_tick = fn;

    sh_timer_sys_init(fn);
    
    return 0;
}

static int sh_sm_state_init(sh_sm_state_t *sm_state, sh_event_server_t *server, uint8_t state_id)
{
    SH_ASSERT(sm_state);

    sh_list_init(&sm_state->list);
    sh_list_init(&sm_state->private_timer_head);
    sm_state->parent = NULL;
    sm_state->server = server;
    sm_state->state_id = state_id;

    return 0;
}

sh_sm_state_t* sh_sm_state_create(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);
    SH_ASSERT(sm->map);

    sh_event_server_t *server = sh_event_server_create(sm->map, NULL);
    if (server == NULL) {
        return NULL;
    }

    sh_sm_state_t *state = SH_MALLOC(sizeof(sh_sm_state_t));
    if (state == NULL) {
        goto free_server;
    }
    
    sh_sm_state_init(state, server, state_id);

    return state;

free_server:
    SH_FREE(server);

    return NULL;
}

int sh_sm_add_state(sh_sm_t *sm, sh_sm_state_t *state)
{
    SH_ASSERT(sm);
    SH_ASSERT(state);

    sh_list_insert_before(&state->list, &sm->state_list);
    
    return 0;
}

static sh_sm_state_t* sh_sm_get_state(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);

    sh_list_for_each(node, &sm->state_list) {
        sh_sm_state_t *state = sh_container_of(node, sh_sm_state_t, list);
        if (state->state_id == state_id) {
            sm->current_state = state;
            return state;
        }
    }

    return NULL;
}

int sh_sm_trans_to(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);

    sh_sm_state_t *to_state = sh_sm_get_state(sm, state_id);
    if (to_state == NULL) {
        return -1;
    }

    if (sm->current_state) {
        sh_event_server_t *server = sm->current_state->server;
        sh_event_server_stop(server);
        sh_event_server_clear_msg(server);
    }

    sm->current_state = to_state;
    sh_event_server_start(sm->current_state->server);

    return 0;
}

int sh_sm_handler(sh_sm_t *sm)
{
    SH_ASSERT(sm);

    if (sm->current_state == NULL) {
        return -1;
    }

    sh_event_server_t *server = sm->current_state->server;
    if (server == NULL) {
        return -1;
    }

    sh_timer_handler(&sm->global_timer_head);
    sh_timer_handler(&sm->current_state->private_timer_head);

    return sh_event_handler(server);
}

int sh_sm_send_event(sh_sm_t *sm, uint8_t event_id)
{
    SH_ASSERT(sm);
    SH_ASSERT(sm->map);

    return sh_event_publish(sm->map, event_id);
}

static void sh_sm_timer_overtick_cb(void *param)
{
    SH_ASSERT(param);

    sh_timer_node_t *timer_node = (sh_timer_node_t*)param;

    sh_sm_send_event(timer_node->sm, timer_node->event_id);
}

static sh_timer_node_t* sh_sm_timer_node_create(enum sh_timer_mode mode)
{
    sh_timer_node_t *timer_node = SH_MALLOC(sizeof(sh_timer_node_t));
    if (timer_node == NULL) {
        return NULL;
    }

    sh_timer_t *timer = sh_timer_create(mode, sh_sm_timer_overtick_cb);
    if (timer == NULL) {
        goto free_timer_node;
    }
    
    timer_node->timer = timer;

free_timer_node:
    SH_FREE(timer_node);

    return NULL;
}

static void sh_sm_timer_node_destroy(sh_timer_node_t *timer_node)
{
    if (timer_node == NULL) {
        return;
    }

    if (timer_node->timer) {
        sh_timer_destroy(timer_node->timer);
    }

    SH_FREE(timer_node);
}

static int sh_sm_timer_get_uniqe_id(uint32_t bitmap)
{
    for (int i = 0; i < (sizeof(bitmap) * 8); i++) {
        if (bitmap & (1ul << i)) {
            return i;
        }
    }

    return -1;
}

static int _sh_sm_start_timer(sh_sm_t *sm, 
                              sh_list_t *head,
                              enum sh_timer_mode mode, 
                              uint32_t interval_tick, 
                              uint8_t event_id,
                              uint32_t destroy_cnt,
                              uint8_t timer_id)
{
    sh_timer_node_t *timer_node = sh_sm_timer_node_create(mode);
    if (timer_node == NULL) {
        return -1;
    }

    timer_node->current_cnt = 0;
    timer_node->sm = sm;
    timer_node->event_id = event_id;
    timer_node->destroy_cnt = destroy_cnt;
    timer_node->timer_id = timer_id;

    sh_timer_set_param(timer_node->timer, timer_node);

    if (sh_timer_start(timer_node->timer, head, 
                       sm->timer_get_tick(), interval_tick))
    {
        sh_sm_timer_node_destroy(timer_node);
        return -1;
    }

    return 0;
}

static int sh_sm_start_timer_and_get_id(sh_sm_t *sm, 
                                        uint32_t *bitmap,
                                        sh_list_t *head,
                                        enum sh_timer_mode mode,
                                        uint32_t interval_tick,
                                        uint8_t event_id)
{
    SH_ASSERT(sm);
    
    int timer_id = sh_sm_timer_get_uniqe_id(*bitmap);
    if (timer_id < 0) {
        return -1;
    }

    if (_sh_sm_start_timer(sm, head, mode, interval_tick, event_id, 
                           (mode == SH_TIMER_MODE_SINGLE) ? 1 : UINT32_MAX, timer_id))
    {
        return -1;
    }

    *bitmap |= (1ul << timer_id);

    return timer_id;
}

int sh_sm_start_global_timer(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->timer_bitmap, &sm->global_timer_head,
                                        SH_TIMER_MODE_SINGLE, interval_tick, event_id);
}

int sh_sm_start_global_timer_loop(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->timer_bitmap, &sm->global_timer_head,
                                        SH_TIMER_MODE_LOOP, interval_tick, event_id);
}

int sh_sm_start_timer(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->current_state->timer_bitmap,
                                        &sm->current_state->private_timer_head,
                                        SH_TIMER_MODE_SINGLE, interval_tick, event_id);
}

int sh_sm_start_timer_loop(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->current_state->timer_bitmap,
                                        &sm->current_state->private_timer_head,
                                        SH_TIMER_MODE_LOOP, interval_tick, event_id);
}

static int sh_sm_remove_timer(sh_sm_t *sm, enum sh_sm_timer_type type, uint8_t timer_id)
{
    SH_ASSERT(sm);

// TODO
    if (type == SH_SM_PRIVATE_TIMER) {
        uint32_t *bitmap_ptr = &sm->current_state->timer_bitmap;
        sh_timer_node_t *timer_node = sh_container_of()
    }
    uint32_t *bitmap_ptr = (type == SH_SM_PRIVATE_TIMER) ? 
                            &sm->current_state->timer_bitmap : &sm->timer_bitmap;
    
    sh_sm_timer_node_destroy(sm->)
}
