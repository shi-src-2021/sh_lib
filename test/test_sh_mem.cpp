#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_mem.h"

using namespace testing;

TEST(TEST_SH_MEM, malloc_and_free_test) {
    int free_size = sh_get_free_size();
    EXPECT_EQ(-1, free_size);

    void *addr = sh_malloc(300);
    ASSERT_NE(addr, nullptr);
    free_size = sh_get_free_size();
    EXPECT_GT(free_size, 0);

    sh_free(addr);
    addr = nullptr;
}

TEST(TEST_SH_MEM, cycling_malloc_and_free_test) {
    void *test_buf[10] = {0};
    int free_size = sh_get_free_size();

    for (int i = 0; i < 10000; i++) {
        int size = rand() % 200 + 200;
        test_buf[i % 10] = sh_malloc(size);
        ASSERT_NE(test_buf[i % 10], nullptr);

        if (i % 10 == 9) {
            for (int j = 0; j < 10; j++) {
                sh_free(test_buf[j]);
                test_buf[j] = nullptr;
            }
        }
    }

    EXPECT_EQ(free_size, sh_get_free_size());
}