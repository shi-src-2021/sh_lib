#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <future>
#include <chrono>
#include <thread>

#include "sh_assert.h"

using namespace testing;

void async_task() {
    char buf[3] = {0};
    
    SH_ASSERT(1);
    SH_ASSERT(buf);
}

TEST(TEST_SH_ASSERT, sh_assert_test) {
    auto future = std::async(std::launch::async, async_task);
    if (future.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        FAIL();
    }

    SUCCEED();
}

