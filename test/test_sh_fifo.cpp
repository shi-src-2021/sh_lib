#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_fifo.h"

using namespace testing;

#define FIFO_SIZE   10

#define FIFO_USED_SIZE_TEST(used) \
            EXPECT_EQ(sh_fifo_get_unused_size(fifo), (FIFO_SIZE) - (used) - 1); \
            EXPECT_EQ(sh_fifo_get_used_size(fifo), (used));

class TEST_SH_FIFO : public testing::Test {
protected:  
    void SetUp()
    {
        mem_size = sh_get_free_size();

        fifo = sh_fifo_create(FIFO_SIZE, 1);
        ASSERT_TRUE(fifo);

        FIFO_USED_SIZE_TEST(0);
    }

    void TearDown()
    {
        sh_fifo_destroy(fifo);

        EXPECT_EQ(mem_size, sh_get_free_size());
    }

    sh_fifo_t *fifo = NULL;
    uint32_t mem_size;
    uint8_t buf_in[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t buf_out[10] = {0};
};

TEST_F(TEST_SH_FIFO, fifo_get_used_size_test) {
    EXPECT_EQ(5, sh_fifo_in(fifo, buf_in, 5));
    FIFO_USED_SIZE_TEST(5);

    EXPECT_EQ(2, sh_fifo_out(fifo, buf_out, 2));
    FIFO_USED_SIZE_TEST(5 - 2);
}

TEST_F(TEST_SH_FIFO, fifo_in_out_test) {
    EXPECT_EQ(5, sh_fifo_in(fifo, buf_in, 5));

    EXPECT_EQ(2, sh_fifo_out(fifo, buf_out, 2));
    EXPECT_EQ(buf_out[0], 0x01);
    EXPECT_EQ(buf_out[1], 0x02);

    EXPECT_EQ(2, sh_fifo_out(fifo, buf_out, 2));
    EXPECT_EQ(buf_out[0], 0x03);
    EXPECT_EQ(buf_out[1], 0x04);
}

TEST_F(TEST_SH_FIFO, fifo_in_out_wrap_test)
{
    EXPECT_EQ(8, sh_fifo_in(fifo, buf_in, 8));

    EXPECT_EQ(7, sh_fifo_out(fifo, buf_out, 7));
    FIFO_USED_SIZE_TEST(1);
    for (int i = 0; i < 7; i++) {
        EXPECT_EQ(buf_out[i], i + 1);
    }

    EXPECT_EQ(8, sh_fifo_in(fifo, buf_in, 8));
    FIFO_USED_SIZE_TEST(9);

    EXPECT_EQ(9, sh_fifo_out(fifo, buf_out, 9));
    EXPECT_EQ(buf_out[0], 8);
    for (int i = 1; i < 9; i++) {
        EXPECT_EQ(buf_out[i], i);
    }

    EXPECT_EQ(fifo->in, 6);
    EXPECT_EQ(fifo->out, 6);
}

TEST_F(TEST_SH_FIFO, fifo_in_oversize_test)
{
    EXPECT_EQ(9, sh_fifo_in(fifo, buf_in, 10));
    FIFO_USED_SIZE_TEST(9);
    
    EXPECT_EQ(0, sh_fifo_in(fifo, buf_in, 10));
}

TEST_F(TEST_SH_FIFO, fifo_out_oversize_test)
{
    EXPECT_EQ(0, sh_fifo_out(fifo, buf_out, 15));
    EXPECT_EQ(0, sh_fifo_out(fifo, buf_out, 1));

    EXPECT_EQ(9, sh_fifo_in(fifo, buf_in, 10));
    FIFO_USED_SIZE_TEST(9);

    EXPECT_EQ(9, sh_fifo_out(fifo, buf_out, 15));
    FIFO_USED_SIZE_TEST(0);
    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(buf_out[i], i + 1);
    }
}

TEST_F(TEST_SH_FIFO, fifo_in_out_for_multi_esize_test) {
    sh_fifo_t *_fifo = sh_fifo_create(10, 2);
    EXPECT_EQ(5, sh_fifo_in(_fifo, buf_in, 5));

    EXPECT_EQ(2, sh_fifo_out(_fifo, buf_out, 2));
    EXPECT_EQ(buf_out[0], 0x01);
    EXPECT_EQ(buf_out[1], 0x02);
    EXPECT_EQ(buf_out[2], 0x03);
    EXPECT_EQ(buf_out[3], 0x04);

    EXPECT_EQ(2, sh_fifo_out(_fifo, buf_out, 2));
    EXPECT_EQ(buf_out[0], 0x05);
    EXPECT_EQ(buf_out[1], 0x06);
    EXPECT_EQ(buf_out[2], 0x07);
    EXPECT_EQ(buf_out[3], 0x08);

    sh_fifo_destroy(_fifo);
}


