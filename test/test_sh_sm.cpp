#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_sm.h"
#include "sh_lib.h"

using namespace testing;

enum sh_event_type {
    SH_EVENT_ONE = 0,
    SH_EVENT_TWO,
    SH_EVENT_THREE,

    SH_EVENT_MAX,
};

enum sh_sm_state_e {
    SH_SM_STATE_ENTER = 0,
    SH_SM_STATE_EXECUTE,
    SH_SM_STATE_EXIT,

    SH_SM_STATE_MAX,
};

uint32_t event_cnt[SH_EVENT_MAX][SH_SM_STATE_MAX] = {0};
uint8_t event_buf[] = {SH_EVENT_ONE, SH_EVENT_TWO, SH_EVENT_THREE};

uint32_t current_tick = 0;

uint32_t get_tick_cnt(void)
{
    return current_tick;
}

void test_sleep_tick(uint32_t increase_tick)
{
    current_tick += increase_tick;
}

static void sh_sm_state_enter_cb(const sh_event_msg_t *e)
{
    char state_name[] = "enter";
    uint32_t *event_cnt_ptr = (uint32_t*)&event_cnt[SH_SM_STATE_ENTER];

    switch (e->id) {
    case SH_EVENT_ONE:
        event_cnt_ptr[0]++;
        break;
    case SH_EVENT_TWO:
        event_cnt_ptr[1]++;
        break;
    case SH_EVENT_THREE:
        event_cnt_ptr[2]++;
        break;
    default:
        break;
    }
}

static void sh_sm_state_execute_cb(const sh_event_msg_t *e)
{
    char state_name[] = "execute";
    uint32_t *event_cnt_ptr = (uint32_t*)&event_cnt[SH_SM_STATE_EXECUTE];

    switch (e->id) {
    case SH_EVENT_ONE:
        event_cnt_ptr[0]++;
        break;
    case SH_EVENT_TWO:
        event_cnt_ptr[1]++;
        break;
    case SH_EVENT_THREE:
        event_cnt_ptr[2]++;
        break;
    default:
        break;
    }
}

static void sh_sm_state_exit_cb(const sh_event_msg_t *e)
{
    char state_name[] = "exit";
    uint32_t *event_cnt_ptr = (uint32_t*)&event_cnt[SH_SM_STATE_EXIT];

    switch (e->id) {
    case SH_EVENT_ONE:
        event_cnt_ptr[0]++;
        break;
    case SH_EVENT_TWO:
        event_cnt_ptr[1]++;
        break;
    case SH_EVENT_THREE:
        event_cnt_ptr[2]++;
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
        memset(event_cnt, 0, sizeof(event_cnt));

        _free_size = sh_get_free_size();

        sm = sh_sm_create(SH_GROUP(event_buf), get_tick_cnt);
        ASSERT_TRUE(sm);

        ASSERT_EQ(0, sh_sm_state_create(sm, SH_SM_STATE_ENTER));
        ASSERT_EQ(0, sh_sm_state_create(sm, SH_SM_STATE_EXECUTE));
        ASSERT_EQ(0, sh_sm_state_create(sm, SH_SM_STATE_EXIT));

        uint8_t event_buf[] = {SH_EVENT_ONE, SH_EVENT_TWO, SH_EVENT_THREE};
        ASSERT_EQ(0, sh_sm_state_subscribe_events(sm, SH_SM_STATE_ENTER,   SH_GROUP(event_buf), sh_sm_state_enter_cb));
        ASSERT_EQ(0, sh_sm_state_subscribe_events(sm, SH_SM_STATE_EXECUTE, SH_GROUP(event_buf), sh_sm_state_execute_cb));
        ASSERT_EQ(0, sh_sm_state_subscribe_event(sm, SH_SM_STATE_EXIT, SH_EVENT_ONE, sh_sm_state_exit_cb));

        ASSERT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_ENTER));
    }

    void TearDown()
    {
        sh_sm_destroy(sm);

        EXPECT_EQ(_free_size, sh_get_free_size());
    }
    
    sh_sm_t *sm;
    int _free_size;
};

TEST_F(TEST_SH_SM, sm_subscribe_test) {
    uint32_t free_size = sh_get_free_size();
    
    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXIT));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(0, sh_sm_state_subscribe_event(sm, SH_SM_STATE_EXIT, SH_EVENT_TWO, sh_sm_state_exit_cb));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_GT(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_unsubscribe_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(0, sh_sm_state_unsubscribe_event(sm, SH_SM_STATE_ENTER, SH_EVENT_TWO));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_LT(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_trans_to_test) {
    uint32_t free_size = sh_get_free_size();
    
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXIT));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);
    
    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_repeat_publish_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(2, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(2, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(2, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(2, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_publish_which_is_not_subscribed) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXIT));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_THREE));
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(2, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_disable_state_test) {
    uint32_t free_size = sh_get_free_size();
    
    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_TWO));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(0, sh_sm_trans_to(sm, SH_SM_STATE_EXIT));

    EXPECT_EQ(0, sh_sm_publish_event(sm, SH_EVENT_ONE));
    EXPECT_EQ(0, sh_sm_handler(sm));

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXIT][SH_EVENT_THREE]);

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);
    
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_private_timer_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_timer(sm, 2000, SH_EVENT_THREE));

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_GT(free_size, sh_get_free_size());

    sh_sm_trans_to(sm, SH_SM_STATE_EXECUTE);

    EXPECT_EQ(free_size, sh_get_free_size());

    for (int i = 0; i < 1000; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_global_timer_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_global_timer(sm, 500, SH_EVENT_ONE));

    for (int i = 0; i < 400; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_THREE]);

    sh_sm_trans_to(sm, SH_SM_STATE_EXECUTE);

    for (int i = 0; i < 200; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_EXECUTE][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_private_timer_auto_destroy_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_timer(sm, 700, SH_EVENT_THREE));

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_private_timer_manual_destroy_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_timer(sm, 2000, SH_EVENT_THREE));

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_GT(free_size, sh_get_free_size());

    EXPECT_EQ(0, sh_sm_remove_timer(sm, SH_SM_PRIVATE_TIMER, 2));

    EXPECT_EQ(free_size, sh_get_free_size());
}

TEST_F(TEST_SH_SM, sm_private_timer_manual_remove_all_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_timer(sm, 2000, SH_EVENT_THREE));

    EXPECT_GT(free_size, sh_get_free_size());

    EXPECT_EQ(0, sh_sm_remove_state_all_timer(sm, SH_SM_STATE_ENTER));

    EXPECT_EQ(free_size, sh_get_free_size());

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);
}

TEST_F(TEST_SH_SM, sm_global_timer_manual_remove_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_global_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_global_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_global_timer(sm, 2000, SH_EVENT_THREE));

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);

    EXPECT_GT(free_size, sh_get_free_size());

    EXPECT_EQ(0, sh_sm_remove_timer(sm, SH_SM_GLOBAL_TIMER, 2));

    EXPECT_EQ(free_size, sh_get_free_size());

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(1, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);
}

TEST_F(TEST_SH_SM, sm_global_timer_manual_remove_all_test) {
    uint32_t free_size = sh_get_free_size();

    EXPECT_EQ(0, sh_sm_start_global_timer(sm, 500, SH_EVENT_ONE));
    EXPECT_EQ(1, sh_sm_start_global_timer(sm, 1000, SH_EVENT_TWO));
    EXPECT_EQ(2, sh_sm_start_global_timer(sm, 2000, SH_EVENT_THREE));

    EXPECT_GT(free_size, sh_get_free_size());

    sh_sm_remove_all_global_timer(sm);

    EXPECT_EQ(free_size, sh_get_free_size());

    for (int i = 0; i < 1100; i++) {
        sh_sm_handler(sm);
        test_sleep_tick(1);
    }

    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_ONE]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_TWO]);
    EXPECT_EQ(0, event_cnt[SH_SM_STATE_ENTER][SH_EVENT_THREE]);
}
