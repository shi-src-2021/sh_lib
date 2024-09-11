#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_list.h"
#include "sh_lib.h"

using namespace testing;

class TEST_SH_LIST : public testing::Test {
protected:  
    void SetUp()
    {
        sh_list_init(&head);
    }

    void TearDown()
    {
        
    }
    
    sh_list_t head;
};

TEST_F(TEST_SH_LIST, list_insert_after_test) {
    SH_LIST_INIT(node);

    sh_list_insert_after(&node, &head);

    ASSERT_EQ(&node, head.next);
    ASSERT_EQ(&node, head.prev);
    ASSERT_EQ(&head, node.next);
    ASSERT_EQ(&head, node.next);
    
    int cnt = 0;
    sh_list_for_each(_node, &head) {
        cnt++;
        ASSERT_EQ(&node, _node);
    }
    ASSERT_EQ(cnt, 1);
}

TEST_F(TEST_SH_LIST, list_insert_before_test) {
    SH_LIST_INIT(node);

    sh_list_insert_before(&node, &head);

    ASSERT_EQ(&node, head.next);
    ASSERT_EQ(&node, head.prev);
    ASSERT_EQ(&head, node.next);
    ASSERT_EQ(&head, node.next);
    
    int cnt = 0;
    sh_list_for_each(_node, &head) {
        cnt++;
        ASSERT_EQ(&node, _node);
    }
    ASSERT_EQ(cnt, 1);
}

TEST_F(TEST_SH_LIST, list_for_each_test) {
    sh_list_t nodes[10];

    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
        sh_list_init(&nodes[i]);
        sh_list_insert_before(&nodes[i], &head);
    }
    
    int index = 0;
    sh_list_for_each(_node, &head) {
        ASSERT_EQ(&nodes[index], _node);
        index++;
    }
    ASSERT_EQ(index, 10);
}

TEST_F(TEST_SH_LIST, list_for_each_safe_test) {
    sh_list_t nodes[10];

    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
        sh_list_init(&nodes[i]);
        sh_list_insert_before(&nodes[i], &head);
    }
    
    int index = 0;
    sh_list_for_each_safe(_node, &head) {
        ASSERT_EQ(&nodes[index], _node);
        sh_list_remove(&nodes[index]);
        index++;
    }
    ASSERT_EQ(index, 10);
}

TEST_F(TEST_SH_LIST, list_remove_test) {
    sh_list_t nodes[10];

    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
        sh_list_init(&nodes[i]);
        sh_list_insert_before(&nodes[i], &head);
    }
    
    int index = 0;
    sh_list_for_each_safe(_node, &head) {
        ASSERT_EQ(&nodes[index], _node);
        sh_list_remove(&nodes[index]);
        ASSERT_EQ(&nodes[index], nodes[index].next);
        ASSERT_EQ(1, sh_list_isempty(&nodes[index]));

        index++;
    }
    ASSERT_EQ(index, 10);
}

TEST_F(TEST_SH_LIST, list_isempty_test) {
    sh_list_t nodes[10];

    ASSERT_EQ(1, sh_list_isempty(&head));

    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
        sh_list_init(&nodes[i]);
        sh_list_insert_before(&nodes[i], &head);
    }
    
    int index = 0;
    sh_list_for_each(_node, &head) {
        ASSERT_EQ(&nodes[index], _node);
        index++;
    }
    ASSERT_EQ(index, 10);

    ASSERT_EQ(0, sh_list_isempty(&head));
}

