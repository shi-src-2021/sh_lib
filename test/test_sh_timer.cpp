#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <windows.h>
#include <stdio.h>

#include "sh_timer.h"
#include "sh_lib.h"

using namespace testing;

#define TIMER_AMOUNT    10

sh_timer_t timer[TIMER_AMOUNT];
int timer_cnt[TIMER_AMOUNT] = {0};

uint32_t tick = 0;

static uint32_t windows_get_tick(void)
{
    return tick++;
}

static void timer_overtick_cb(void *param)
{
    if (strcmp("timer4", (char*)param) == 0) {
        sh_timer_stop(&timer[0]);
        sh_timer_stop(&timer[1]);
        sh_timer_stop(&timer[2]);
        timer_cnt[4]++;
    } else if (strcmp("timer0", (char*)param) == 0) {
        timer_cnt[0]++;
    } else if (strcmp("timer1", (char*)param) == 0) {
        timer_cnt[1]++;
    } else if (strcmp("timer2", (char*)param) == 0) {
        timer_cnt[2]++;
    } else if (strcmp("timer3", (char*)param) == 0) {
        timer_cnt[3]++;
    }
}

class TEST_SH_TIMER : public testing::Test {
protected:  
    void SetUp()
    {
        sh_list_init(&head);
        memset(timer_cnt, 0, sizeof(timer_cnt));

        sh_timer_sys_init(windows_get_tick);

        for (int i = 0; i < ARRAY_SIZE(timer); i++) {
            sh_timer_init(&timer[i], SH_TIMER_MODE_LOOP, timer_overtick_cb);
            buf[i] = (char*)malloc(20);
            sprintf(buf[i], "timer%d", i);
            sh_timer_set_param(&timer[i], buf[i]);
        }
    }

    void TearDown()
    {
        for (int i = 0; i < ARRAY_SIZE(timer); i++) {
            free(buf[i]);
        }
    }

    sh_list_t head;
    char *buf[TIMER_AMOUNT];
};

TEST_F(TEST_SH_TIMER, sh_timer_init_test) {
    EXPECT_FALSE(sh_timer_start(&timer[0], &head, 0, 100));
    EXPECT_FALSE(sh_timer_start(&timer[1], &head, 0, 200));
    EXPECT_FALSE(sh_timer_start(&timer[2], &head, 0, 500));
    EXPECT_FALSE(sh_timer_start(&timer[3], &head, 0, 400));
    EXPECT_FALSE(sh_timer_start(&timer[4], &head, 0, 1001));
    sh_timer_set_mode(&timer[3], SH_TIMER_MODE_SINGLE);
    sh_timer_set_mode(&timer[4], SH_TIMER_MODE_SINGLE);

    int i = 0;
    while (1) {
        if (i++ > 1099) {
            break;
        }
        sh_timer_loop(&head);
    }
    EXPECT_EQ(timer_cnt[0], 10);
    EXPECT_EQ(timer_cnt[1], 5);
    EXPECT_EQ(timer_cnt[2], 2);
    EXPECT_EQ(timer_cnt[3], 1);
    EXPECT_EQ(timer_cnt[4], 1);
}

TEST_F(TEST_SH_TIMER, tick_overflow_test) {
    EXPECT_TRUE(sh_timer_is_time_out(100, -100));
    EXPECT_TRUE(!sh_timer_is_time_out(-100, 100));
    EXPECT_TRUE(sh_timer_is_time_out(100, 100));
    EXPECT_TRUE(sh_timer_is_time_out(0, 0));
    EXPECT_TRUE(sh_timer_is_time_out(-1, -1));
}

#if SH_TIMER_MALLOC_ENABLE
TEST_F(TEST_SH_TIMER, timer_malloc_test) {
    int mem_size = sh_get_free_size();
    sh_timer_t *timer = sh_timer_create(SH_TIMER_MODE_SINGLE, timer_overtick_cb);
    ASSERT_TRUE(timer);
    sh_timer_destroy(timer);
    EXPECT_EQ(mem_size, sh_get_free_size());
}
#endif

