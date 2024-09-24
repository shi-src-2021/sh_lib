#include "sh_sm.h"
#include "sh_assert.h"
#include "sh_lib.h"

#ifndef SH_MALLOC
    #define SH_MALLOC   malloc
#endif

#ifndef SH_FREE
    #define SH_FREE     free
#endif

struct sh_sm_timer_node {
    sh_list_t list;
    sh_timer_t *timer;
    uint32_t current_cnt;
    uint32_t destroy_cnt;
    uint8_t event_id;
    sh_sm_t *sm;
    uint8_t timer_id;
};

typedef struct sh_sm_timer_node sh_sm_timer_node_t;


struct sh_sm_state {
    sh_list_t list;
    uint8_t state_id;
    struct sh_sm_state *parent;
    sh_event_server_t *server;
    sh_list_t private_timer_head;
    uint32_t timer_bitmap;
    sh_list_t timer_node_head;
};

typedef struct sh_sm_state sh_sm_state_t;

struct sh_sm {
    sh_list_t state_list;
    sh_list_t global_timer_head;
    sh_sm_state_t *current_state;
    sh_event_map_t *map;
    sh_timer_get_tick_fn timer_get_tick;
    uint32_t timer_bitmap;
    sh_list_t timer_node_head;
};

static int sh_sm_init(sh_sm_t *sm, sh_event_type_table_t *table,
               size_t size, sh_timer_get_tick_fn fn)
{
    SH_ASSERT(sm);
    SH_ASSERT(table);
    SH_ASSERT(fn);

    sh_event_map_t *map = sh_event_map_create(table, size);
    if (map == NULL) {
        return -1;
    }

    sh_list_init(&sm->global_timer_head);
    sh_list_init(&sm->state_list);
    sh_list_init(&sm->timer_node_head);
    sm->current_state = NULL;
    sm->map = map;
    sm->timer_get_tick = fn;
    sm->timer_bitmap = 0;

    sh_timer_sys_init(fn);
    
    return 0;
}

sh_sm_t* sh_sm_create(sh_event_type_table_t *table, size_t size,
                      sh_timer_get_tick_fn fn)
{
    SH_ASSERT(table);

    sh_sm_t *sm = SH_MALLOC(sizeof(sh_sm_t));
    if (sm == NULL) {
        return NULL;
    }

    if (sh_sm_init(sm, table, size, fn)) {
        SH_FREE(sm);
        sm = NULL;
    }

    return sm;
}

static void _sh_sm_remove_state_all_timer(sh_sm_t *sm, sh_sm_state_t *state)
{
    SH_ASSERT(sm);
    SH_ASSERT(state);

    for (int i = 0; i < (sizeof(state->timer_bitmap) * 8); i++) {
        if (state->timer_bitmap & (1ul << i)) {
            sh_sm_remove_timer(sm, SH_SM_PRIVATE_TIMER, i);
        }
    }
}

static int sh_sm_remove_state_node(sh_sm_state_t *state)
{
    SH_ASSERT(state);

    sh_list_remove(&state->list);

    return 0;
}

static void _sh_sm_state_destroy(sh_sm_t *sm, sh_sm_state_t *state)
{
    SH_ASSERT(sm);
    SH_ASSERT(state);

    if (state->server) {
        sh_event_server_destroy(state->server);
    }

    _sh_sm_remove_state_all_timer(sm, state);

    sh_sm_remove_state_node(state);

    SH_FREE(state);
}

void sh_sm_destroy(sh_sm_t *sm)
{
    SH_ASSERT(sm);

    sh_list_for_each_safe(node, &sm->state_list) {
        sh_sm_state_t *state = sh_container_of(node, sh_sm_state_t, list);
        _sh_sm_state_destroy(sm, state);
    }
    sh_event_map_destroy(sm->map);
    sh_sm_remove_all_global_timer(sm);

    SH_FREE(sm);
}


static int sh_sm_state_init(sh_sm_state_t *state, 
                            sh_event_server_t *server, 
                            uint8_t state_id)
{
    SH_ASSERT(state);

    sh_list_init(&state->list);
    sh_list_init(&state->private_timer_head);
    sh_list_init(&state->timer_node_head);
    state->parent = NULL;
    state->server = server;
    state->state_id = state_id;
    state->timer_bitmap = 0;

    return 0;
}

static int sh_sm_add_state(sh_sm_t *sm, sh_sm_state_t *state)
{
    SH_ASSERT(sm);
    SH_ASSERT(state);

    sh_list_insert_before(&state->list, &sm->state_list);
    
    return 0;
}

int sh_sm_state_create(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);
    SH_ASSERT(sm->map);

    sh_event_server_t *server = sh_event_server_create(sm->map, NULL);
    if (server == NULL) {
        return -1;
    }

    sh_sm_state_t *state = SH_MALLOC(sizeof(sh_sm_state_t));
    if (state == NULL) {
        goto free_server;
    }

    sh_sm_state_init(state, server, state_id);

    sh_sm_add_state(sm, state);

    return 0;

free_server:
    sh_event_server_destroy(server);

    return -1;
}

static sh_sm_state_t* sh_sm_get_state(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);

    sh_list_for_each(node, &sm->state_list) {
        sh_sm_state_t *state = sh_container_of(node, sh_sm_state_t, list);
        if (state->state_id == state_id) {
            return state;
        }
    }

    return NULL;
}

int sh_sm_state_destroy(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);

    sh_sm_state_t *state = sh_sm_get_state(sm, state_id);
    if (state == NULL) {
        return -1;
    }

    _sh_sm_state_destroy(sm, state);

    return 0;
}

int sh_sm_state_create_with_event(sh_sm_t *sm, uint8_t state_id, uint8_t *event_buf,
                                  uint8_t buf_size, event_cb cb)
{
    if (sh_sm_state_create(sm, state_id)) {
        return -1;
    }

    for (int i = 0; i < buf_size; i++) {
        if (sh_sm_state_subscribe_event(sm, state_id, event_buf[i], cb)) {
            return -1;
        }
    }

    return 0;
}

int sh_sm_state_subscribe_event(sh_sm_t *sm, uint8_t state_id, uint8_t event_id, event_cb cb)
{
    SH_ASSERT(sm);

    sh_sm_state_t *state = sh_sm_get_state(sm, state_id);
    if (state == NULL) {
        return -1;
    }

    return sh_event_subscribe(state->server, event_id, cb);
}

int sh_sm_state_unsubscribe_event(sh_sm_t *sm, uint8_t state_id, uint8_t event_id)
{
    SH_ASSERT(sm);

    sh_sm_state_t *state = sh_sm_get_state(sm, state_id);
    if (state == NULL) {
        return -1;
    }

    return sh_event_unsubscribe(state->server, event_id);
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
        
        _sh_sm_remove_state_all_timer(sm, sm->current_state);
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

int sh_sm_publish_event(sh_sm_t *sm, uint8_t event_id)
{
    SH_ASSERT(sm);
    SH_ASSERT(sm->map);

    return sh_event_publish(sm->map, event_id);
}

static void sh_sm_timer_node_destroy(sh_sm_timer_node_t *timer_node)
{
    if (timer_node == NULL) {
        return;
    }

    sh_list_remove(&timer_node->list);

    if (timer_node->timer) {
        sh_timer_destroy(timer_node->timer);
    }

    SH_FREE(timer_node);
}

static void sh_sm_timer_overtick_cb(void *param)
{
    SH_ASSERT(param);

    sh_sm_timer_node_t *timer_node = (sh_sm_timer_node_t*)param;

    sh_sm_publish_event(timer_node->sm, timer_node->event_id);

    if (timer_node->destroy_cnt == UINT32_MAX) {
        return;
    }

    timer_node->current_cnt++;
    if (timer_node->current_cnt >= timer_node->destroy_cnt) {
        sh_sm_timer_node_destroy(timer_node);
    }
}

static sh_sm_timer_node_t* sh_sm_timer_node_create(enum sh_timer_mode mode)
{
    sh_sm_timer_node_t *timer_node = SH_MALLOC(sizeof(sh_sm_timer_node_t));
    if (timer_node == NULL) {
        return NULL;
    }

    sh_timer_t *timer = sh_timer_create(mode, sh_sm_timer_overtick_cb);
    if (timer == NULL) {
        goto free_timer_node;
    }
    
    timer_node->timer = timer;

    return timer_node;

free_timer_node:
    SH_FREE(timer_node);

    return NULL;
}

static int sh_sm_timer_get_uniqe_id(uint32_t bitmap)
{
    for (int i = 0; i < (sizeof(bitmap) * 8); i++) {
        if (!(bitmap & (1ul << i))) {
            return i;
        }
    }

    return -1;
}

static int _sh_sm_start_timer(sh_sm_t *sm, 
                              sh_list_t *timer_head,
                              sh_list_t *timer_node_head,
                              enum sh_timer_mode mode, 
                              uint32_t interval_tick, 
                              uint8_t event_id,
                              uint32_t destroy_cnt,
                              uint8_t timer_id)
{
    sh_sm_timer_node_t *timer_node = sh_sm_timer_node_create(mode);
    if (timer_node == NULL) {
        return -1;
    }

    timer_node->current_cnt = 0;
    timer_node->sm = sm;
    timer_node->event_id = event_id;
    timer_node->destroy_cnt = destroy_cnt;
    timer_node->timer_id = timer_id;
    sh_list_init(&timer_node->list);

    sh_timer_set_param(timer_node->timer, timer_node);

    if (sh_timer_start(timer_node->timer, timer_head, 
                       sm->timer_get_tick(), interval_tick))
    {
        sh_sm_timer_node_destroy(timer_node);
        return -1;
    }

    sh_list_insert_before(&timer_node->list, timer_node_head);

    return 0;
}

static int sh_sm_start_timer_and_get_id(sh_sm_t *sm, 
                                        uint32_t *bitmap,
                                        sh_list_t *timer_head,
                                        sh_list_t *timer_node_head,
                                        enum sh_timer_mode mode,
                                        uint32_t interval_tick,
                                        uint8_t event_id)
{
    SH_ASSERT(sm);
    
    int timer_id = sh_sm_timer_get_uniqe_id(*bitmap);
    if (timer_id < 0) {
        return -1;
    }

    if (_sh_sm_start_timer(sm, timer_head, timer_node_head, mode, interval_tick, event_id, 
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
                                        &sm->timer_node_head, SH_TIMER_MODE_SINGLE,
                                        interval_tick, event_id);
}

int sh_sm_start_timer(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->current_state->timer_bitmap,
                                        &sm->current_state->private_timer_head,
                                        &sm->current_state->timer_node_head,
                                        SH_TIMER_MODE_SINGLE, interval_tick, event_id);
}

#ifdef SH_SM_USE_LOOP_TIMER
int sh_sm_start_global_timer_loop(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->timer_bitmap, &sm->global_timer_head,
                                        &sm->timer_node_head, SH_TIMER_MODE_LOOP,
                                        interval_tick, event_id);
}

int sh_sm_start_timer_loop(sh_sm_t *sm, uint32_t interval_tick, uint8_t event_id)
{
    return sh_sm_start_timer_and_get_id(sm, &sm->current_state->timer_bitmap,
                                        &sm->current_state->private_timer_head,
                                        &sm->current_state->timer_node_head,
                                        SH_TIMER_MODE_LOOP, interval_tick, event_id);
}
#endif

static int sh_sm_timer_destroy(sh_list_t *timer_node_head, uint8_t timer_id)
{
    SH_ASSERT(timer_node_head);

    sh_list_for_each(node, timer_node_head) {
        sh_sm_timer_node_t *timer_node = sh_container_of(node, sh_sm_timer_node_t, list);
        if (timer_node->timer_id == timer_id) {
            sh_sm_timer_node_destroy(timer_node);
            return 0;
        }
    }

    return -1;
}

static int sh_sm_get_bitmap_and_timer_head(sh_sm_t *sm, 
                                           enum sh_sm_timer_type type, 
                                           uint32_t **bitmap, 
                                           sh_list_t **timer_node_head)
{
    if (type == SH_SM_PRIVATE_TIMER) {
        if (!sm->current_state) {
            return -1;
        }
        *bitmap = &sm->current_state->timer_bitmap;
        *timer_node_head = &sm->current_state->timer_node_head;
    } else if (type == SH_SM_GLOBAL_TIMER) {
        *bitmap = &sm->timer_bitmap;
        *timer_node_head = &sm->timer_node_head;
    } else {
        return -1;
    }

    return 0;
}

int sh_sm_remove_timer(sh_sm_t *sm, enum sh_sm_timer_type type, uint8_t timer_id)
{
    SH_ASSERT(sm);

    uint32_t *bitmap_ptr = NULL;
    sh_list_t *timer_node_head = NULL;

    /* timer id is too large */
    if (timer_id >= (uint8_t)(sizeof(sm->timer_bitmap) * 8)) {
        return -1;
    }

    if (sh_sm_get_bitmap_and_timer_head(sm, type, &bitmap_ptr, &timer_node_head)) {
        return -1;
    }

    /* timer id does not exist in the bitmap */
    if ((*bitmap_ptr & (1ul << timer_id)) == 0) {
        return -1;
    }

    if (sh_sm_timer_destroy(timer_node_head, timer_id) == 0) {
        *bitmap_ptr &= (~(1ul << timer_id));
    }
    
    return 0;
}

int sh_sm_remove_state_all_timer(sh_sm_t *sm, uint8_t state_id)
{
    SH_ASSERT(sm);

    sh_sm_state_t *state = sh_sm_get_state(sm, state_id);
    if (state == NULL) {
        return -1;
    }

    _sh_sm_remove_state_all_timer(sm, state);

    return 0;
}

void sh_sm_remove_all_global_timer(sh_sm_t *sm)
{
    SH_ASSERT(sm);

    for (int i = 0; i < (sizeof(sm->timer_bitmap) * 8); i++) {
        if (sm->timer_bitmap & (1ul << i)) {
            sh_sm_remove_timer(sm, SH_SM_GLOBAL_TIMER, i);
        }
    }
}

