#include "sh_sm.h"
#include "sh_lib.h"

enum sh_event_type {
    SH_EVENT_1 = 1,
    SH_EVENT_2,
    SH_EVENT_3,
};

static sh_event_type_table_t event_table[] = {
    {SH_EVENT_1,    "event_1"},
    {SH_EVENT_2,    "event_2"},
    {SH_EVENT_3,    "event_3"},
};

enum sh_sm_state_e {
    SH_SM_STATE_1 = 0,
    SH_SM_STATE_2,
};

/* 定义状态机对象 */
sh_sm_t *sm = NULL;

/* 定义读取当前系统时间（ms）函数 */
uint32_t board_sh_get_tick_fn(void)
{
    // ...
}

/* 定义状态机事件发布函数，可由定时器、外部程序等调用 */
void board_sm_publish_event(uint8_t event_id)
{
    sh_sm_publish_event(sm, event_id);
}

/* 定义状态1事件处理函数 */
static void sh_sm_state_1_cb(const sh_event_msg_t *e)
{
    printf("state: [1], event: %d.", e->id);

    switch (e->id) {
    case SH_EVENT_1:
        /* 发布事件 */
        board_sm_publish_event(SH_EVENT_2);
        break;

    case SH_EVENT_2:
        /* 开启状态私有定时器，超时后会自动发布事件3 */
        sh_sm_start_timer(sm, 500, SH_EVENT_3);
        break;

    case SH_EVENT_3:
        /* 切换状态 */
        sh_sm_trans_to(sm, SH_SM_STATE_2);
        /* 发布事件 */
        board_sm_publish_event(SH_EVENT_2);
        break;

    default:
        break;
    }
}

/* 定义状态2事件处理函数 */
static void sh_sm_state_2_cb(const sh_event_msg_t *e)
{
    printf("state: [2], event: %d.", e->id);

    switch (e->id) {
    case SH_EVENT_2:
        /* 开启状态私有定时器，超时后会自动发布事件3 */
        sh_sm_start_timer(sm, 200, SH_EVENT_3);
        break;

    case SH_EVENT_3:
        /* 切换状态 */
        sh_sm_trans_to(sm, SH_SM_STATE_1);
        /* 发布事件 */
        board_sm_publish_event(SH_EVENT_2);
        break;

    default:
        break;
    }
}

int main(int argc, char **argv)
{
    /* 创建状态机并注册获取系统事件函数 */
    sm = sh_sm_create(SH_GROUP(event_table), board_sh_get_tick_fn);

    /* 创建状态1、2 */
    sh_sm_state_create(sm, SH_SM_STATE_1);
    sh_sm_state_create(sm, SH_SM_STATE_2);

    /* 状态1订阅事件组 */
    uint8_t event_buf[] = {SH_EVENT_1, SH_EVENT_2, SH_EVENT_3};
    sh_sm_state_subscribe_events(sm, SH_SM_STATE_1, SH_GROUP(event_buf), sh_sm_state_1_cb);

    /* 状态2单独订阅事件 */
    sh_sm_state_subscribe_event(sm, SH_SM_STATE_2, SH_EVENT_2, sh_sm_state_2_cb);
    sh_sm_state_subscribe_event(sm, SH_SM_STATE_2, SH_EVENT_3, sh_sm_state_2_cb);

    /* 切换状态1 */
    sh_sm_trans_to(sm, SH_SM_STATE_1);

    /* 发布事件 */
    board_sm_publish_event(SH_EVENT_1);

    while (1)
    {
        /* 定时器或者RTOS线程中周期性调用 */
        sh_sm_handler(sm);
        
        rt_thread_mdelay(1);
    }
}



