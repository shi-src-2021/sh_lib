#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_bits.h"

using namespace testing;

TEST(TEST_SH_BITS, get_bits_test) {
    EXPECT_EQ(0x34,     sh_bits_get(0x123456, 8,  8));
    EXPECT_EQ(0x14,     sh_bits_get(0x123456, 8,  5));
    EXPECT_EQ(0x01,     sh_bits_get(0x123456, 20, 8));
    EXPECT_EQ(0x01,     sh_bits_get(0x123456, 20, 1));
    EXPECT_EQ(0x56,     sh_bits_get(0x123456, 0,  8));
    EXPECT_EQ(0x06,     sh_bits_get(0x123456, 0,  4));
    EXPECT_EQ(0x02,     sh_bits_get(0x123456, 0,  2));
    EXPECT_EQ(0x45,     sh_bits_get(0x123456, 4,  8));
    EXPECT_EQ(0x12345,  sh_bits_get(0x123456, 4,  32));
    EXPECT_EQ(0x123456, sh_bits_get(0x123456, 0,  32));
}

TEST(TEST_SH_BITS, set_bits_test) {
    EXPECT_EQ(0x120056, sh_bits_set(0x123456, 8,  8, 0x00));
    EXPECT_EQ(0x12ff56, sh_bits_set(0x123456, 8,  8, 0xff));
    EXPECT_EQ(0x12347f, sh_bits_set(0x123456, 0,  7, 0xff));
    EXPECT_EQ(0x1234fe, sh_bits_set(0x123456, 1,  7, 0xff));
    EXPECT_EQ(0x2a3456, sh_bits_set(0x123456, 16, 7, 0xAA));
}
