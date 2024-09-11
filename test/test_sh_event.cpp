#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_event.h"
#include "sh_lib.h"
#include "sh_mem.h"

using namespace testing;

volatile int init_cnt;
volatile int enter_cnt;
volatile int exit_cnt;

void *event_msg_p;

enum sh_event_type {
    SH_EVENT_INIT = 7,
    SH_EVENT_ENTER,
    SH_EVENT_EXIT,
};

static struct sh_event_type_table event_table[] = {
    {SH_EVENT_INIT,     "init"},
    {SH_EVENT_ENTER,    "enter"},
    {SH_EVENT_EXIT,     "exit"},
};

static char* get_event_id_name(uint8_t event_id)
{
    return sh_event_get_event_id_name(event_table, ARRAY_SIZE(event_table), event_id);
}

static void test_event_cb(const struct sh_event *e)
{
    event_msg_p = e->data;

    switch (e->id)
    {
    case SH_EVENT_INIT:
        printf("%s\r\n", get_event_id_name(SH_EVENT_INIT));
        init_cnt++;
        break;
        
    case SH_EVENT_ENTER:
        printf("%s\r\n", get_event_id_name(SH_EVENT_ENTER));
        enter_cnt++;
        break;
        
    case SH_EVENT_EXIT:
        printf("%s\r\n", get_event_id_name(SH_EVENT_EXIT));
        exit_cnt++;
        break;

    default:
        break;
    }
}

class TEST_SH_EVENT : public testing::Test {
protected:  
    void SetUp()
    {
        map = sh_event_map_create(event_table, ARRAY_SIZE(event_table));
        server1 = sh_event_server_create(map, "server1");
        server2 = sh_event_server_create(map, "server2");

        init_cnt = 0;
        enter_cnt = 0;
        exit_cnt = 0;

        event_msg_p = nullptr;

        ASSERT_NE(nullptr, map);
        ASSERT_NE(nullptr, server1);
        ASSERT_NE(nullptr, server2);

        mem_size = sh_get_free_size();
    }

    void TearDown()
    {
        EXPECT_EQ(mem_size, sh_get_free_size());
    }
    
    sh_event_map_t *map;
    sh_event_server_t *server1;
    sh_event_server_t *server2;

    int mem_size;
};

TEST_F(TEST_SH_EVENT, sync_sub_mode_test) {
    ASSERT_EQ(0, sh_event_subscribe_sync(server1, SH_EVENT_INIT, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_ENTER, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server2, SH_EVENT_INIT, test_event_cb));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_execute(server2));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_unsubscribe(server2, SH_EVENT_INIT));

    ASSERT_EQ(1, sh_list_isempty(&server1->event_queue));
    ASSERT_EQ(1, sh_list_isempty(&server2->event_queue));
}

TEST_F(TEST_SH_EVENT, async_sub_mode_test) {
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_INIT,  test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_ENTER, test_event_cb));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_list_isempty(&server1->event_queue));
    int cnt = 0;
    sh_list_for_each(node, &server1->event_queue) {
        cnt++;
    }
    EXPECT_EQ(2, cnt);

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_ENTER));

    ASSERT_EQ(1, sh_list_isempty(&server1->event_queue));
}

TEST_F(TEST_SH_EVENT, repeat_sub_and_msg_str_test) {
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_INIT, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_INIT, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_INIT, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_EXIT, test_event_cb));

    ASSERT_EQ(0, sh_event_publish_with_param(map, SH_EVENT_INIT, (void*)"init1", 6));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));
    ASSERT_EQ(0, sh_event_publish_with_param(map, SH_EVENT_INIT, (void*)"init2", 6));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(1, exit_cnt);

    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_ENTER, test_event_cb));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(1, exit_cnt);

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(1, exit_cnt);

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(2, exit_cnt);

    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_EXIT));
}

TEST_F(TEST_SH_EVENT, server_clear_msg_test) {
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_INIT, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_ENTER, test_event_cb));
    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_EXIT, test_event_cb));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));

    ASSERT_EQ(0, sh_event_server_clear_msg(server1));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));

    ASSERT_EQ(0, sh_event_execute(server1));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(2, enter_cnt);
    EXPECT_EQ(2, exit_cnt);

    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_EXIT));
}
