#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_sm.h"
#include "sh_lib.h"

#include "windows.h"

using namespace testing;

enum sh_event_type {
    SH_EVENT_ONE = 7,
    SH_EVENT_TWO,
    SH_EVENT_THREE,
};

static sh_event_type_table_t event_table[] = {
    {SH_EVENT_ONE,      "one"},
    {SH_EVENT_TWO,      "two"},
    {SH_EVENT_THREE,    "three"},
};

enum sh_sm_state_e {
    SH_SM_STATE_ENTER = 0,
    SH_SM_STATE_EXECUTE,
    SH_SM_STATE_EXIT,
};

uint32_t current_tick = 0;

uint32_t get_tick_cnt(void)
{
    return current_tick;
}

void test_sleep_tick(void)
{
    current_tick++;
}

static char* get_event_id_name(uint8_t event_id)
{
    return sh_event_get_event_id_name(event_table, ARRAY_SIZE(event_table), event_id);
}

static void sh_sm_state_enter_cb(const sh_event_msg_t *e)
{
    char state_name[] = "enter";

    switch (e->id) {
    case SH_EVENT_ONE:
    case SH_EVENT_TWO:
    case SH_EVENT_THREE:
        printf("state: %s, event: %s, tick: %d\r\n", state_name, get_event_id_name(e->id), get_tick_cnt() % 10000);
        break;
    default:
        break;
    }
}

static void sh_sm_state_execute_cb(const sh_event_msg_t *e)
{
    char state_name[] = "execute";

    switch (e->id) {
    case SH_EVENT_ONE:
    case SH_EVENT_TWO:
    case SH_EVENT_THREE:
        printf("state: %s, event: %s, tick: %d\r\n", state_name, get_event_id_name(e->id), get_tick_cnt() % 10000);
        break;
    default:
        break;
    }
}

static void sh_sm_state_exit_cb(const sh_event_msg_t *e)
{
    char state_name[] = "exit";

    switch (e->id) {
    case SH_EVENT_ONE:
    case SH_EVENT_TWO:
    case SH_EVENT_THREE:
        printf("state: %s, event: %s, tick: %d\r\n", state_name, get_event_id_name(e->id), get_tick_cnt() % 10000);
        break;
    default:
        break;
    }
}

class TEST_SH_SM : public testing::Test {
protected:  
    void SetUp()
    {
        current_tick = 0;

        free_size = sh_get_free_size();
        sm = sh_sm_create(event_table, ARRAY_SIZE(event_table), get_tick_cnt);
        ASSERT_TRUE(sm);

        uint8_t event_buf[] = {SH_EVENT_ONE, SH_EVENT_TWO, SH_EVENT_THREE};
        ASSERT_EQ(0, sh_sm_state_create_with_event(sm, SH_SM_STATE_ENTER,   event_buf, ARRAY_SIZE(event_buf), sh_sm_state_enter_cb));
        ASSERT_EQ(0, sh_sm_state_create_with_event(sm, SH_SM_STATE_EXECUTE, event_buf, ARRAY_SIZE(event_buf), sh_sm_state_execute_cb));

        ASSERT_EQ(0, sh_sm_state_create(sm, SH_SM_STATE_EXIT));
        ASSERT_EQ(0, sh_sm_state_subscribe_event(sm, SH_SM_STATE_EXIT, SH_EVENT_ONE, sh_sm_state_exit_cb));

        sh_sm_trans_to(sm, SH_SM_STATE_ENTER);
    }

    void TearDown()
    {
        sh_sm_destroy(sm);

        EXPECT_EQ(free_size, sh_get_free_size());
    }
    
    sh_sm_t *sm;
    int free_size;
};

TEST_F(TEST_SH_SM, sm_subscribe_test) {
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");

    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXECUTE));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");

    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXIT));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));
    printf("-----\r\n");
}

void precise_sleep(double milliseconds) {
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!timer) {
        printf("创建计时器失败\n");
        return;
    }

    LARGE_INTEGER li;
    li.QuadPart = -(LONGLONG)(milliseconds * 10000.0);  // 转换成100纳秒间隔

    // 设置计时器
    SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);

    // 等待计时器
    WaitForSingleObject(timer, INFINITE);

    // 关闭计时器句柄
    CloseHandle(timer);
}

TEST_F(TEST_SH_SM, sm_timer_test) {

    EXPECT_EQ(0, sh_sm_start_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_timer(sm, 2000, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_start_global_timer(sm, 2000, SH_EVENT_ONE));

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick();
    }

    sh_sm_trans_to(sm, SH_SM_STATE_EXECUTE);
    printf("trans to execute, tick: %d\n", get_tick_cnt() % 10000);

    for (int i = 0; i < 1000; i++) {
        sh_sm_handler(sm);
        test_sleep_tick();
    }
    printf("end, tick: %d\n", get_tick_cnt() % 10000);
}


