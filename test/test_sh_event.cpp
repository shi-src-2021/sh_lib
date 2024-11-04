#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_event.h"
#include "sh_lib.h"
#include "sh_mem.h"

using namespace testing;

volatile int init_cnt;
volatile int enter_cnt;
volatile int exit_cnt;

enum sh_event_type {
    SH_EVENT_INIT = 7,
    SH_EVENT_ENTER,
    SH_EVENT_EXIT,
};

static void test_event_cb(const sh_event_msg_t *e)
{
    switch (e->id)
    {
    case SH_EVENT_INIT:
        init_cnt++;
        break;
        
    case SH_EVENT_ENTER:
        enter_cnt++;
        break;
        
    case SH_EVENT_EXIT:
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
        mem_size = sh_get_free_size();

        uint8_t event_buf[] = {SH_EVENT_INIT, SH_EVENT_ENTER, SH_EVENT_EXIT};
        map = sh_event_map_create(SH_GROUP(event_buf));
        server1 = sh_event_server_create(map, "server1");
        server2 = sh_event_server_create(map, "server2");

        init_cnt = 0;
        enter_cnt = 0;
        exit_cnt = 0;

        ASSERT_NE(nullptr, map);
        ASSERT_NE(nullptr, server1);
        ASSERT_NE(nullptr, server2);

        sh_event_server_start(server1);
        sh_event_server_start(server2);
    }

    void TearDown()
    {
        sh_event_server_destroy(server1);
        sh_event_server_destroy(server2);
        sh_event_map_destroy(map);
        
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

    EXPECT_EQ(1, sh_event_server_get_msg_count(server1));
    EXPECT_EQ(0, sh_event_server_get_msg_count(server2));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    EXPECT_EQ(1, sh_event_server_get_msg_count(server1));
    EXPECT_EQ(1, sh_event_server_get_msg_count(server2));

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));
    EXPECT_EQ(1, sh_event_server_get_msg_count(server2));

    ASSERT_EQ(0, sh_event_handler(server2));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));
    EXPECT_EQ(0, sh_event_server_get_msg_count(server2));

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

    EXPECT_EQ(2, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(1, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(0, exit_cnt);

    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));

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

    EXPECT_EQ(3, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(1, exit_cnt);

    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_subscribe(server1, SH_EVENT_ENTER, test_event_cb));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(0, enter_cnt);
    EXPECT_EQ(1, exit_cnt);
    
    EXPECT_EQ(1, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(1, exit_cnt);

    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_publish(map, SH_EVENT_EXIT));
    
    EXPECT_EQ(1, sh_event_server_get_msg_count(server1));

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(2, init_cnt);
    EXPECT_EQ(1, enter_cnt);
    EXPECT_EQ(2, exit_cnt);
    
    EXPECT_EQ(0, sh_event_server_get_msg_count(server1));

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

    ASSERT_EQ(0, sh_event_handler(server1));
    EXPECT_EQ(0, init_cnt);
    EXPECT_EQ(2, enter_cnt);
    EXPECT_EQ(2, exit_cnt);

    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_INIT));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_ENTER));
    ASSERT_EQ(0, sh_event_unsubscribe(server1, SH_EVENT_EXIT));
}

TEST_F(TEST_SH_EVENT, server_name_max_len_test) {
    char name[] = "123456789012345678901234567890";
    sh_event_server_t *server = sh_event_server_create(map, name);
    EXPECT_EQ(0, server->obj.name[SH_EVENT_NAME_MAX - 1]);
    name[SH_EVENT_NAME_MAX - 1] = '\0';
    EXPECT_STREQ(name, server->obj.name);
    sh_event_server_destroy(server);
}

