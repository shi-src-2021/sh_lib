#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_config.h"

using namespace testing;

uint8_t storage[200] = {0};

int test_write(sh_base_t addr, const void *data, size_t size)
{
    uint8_t *temp = (uint8_t*)data;

    for (int i = 0; i < size; i++) {
        storage[addr + i] = *(temp + i);
    }

    return 0;
}

int test_read(sh_base_t addr, void *data, size_t size)
{
    uint8_t *temp = (uint8_t*)data;

    for (int i = 0; i < size; i++) {
        *(temp + i) = storage[addr + i];
    }

    return 0;
}

class TEST_SH_CONFIG : public testing::Test {
protected:  
    sh_config_node_t nodes[8] = {0};

    void SetUp()
    {
        memset(storage, 0xff, sizeof(storage));

        EXPECT_EQ(0, sh_config_init(test_write, test_read, nodes, 0));
    }

    void TearDown()
    {
        
    }
};

TEST_F(TEST_SH_CONFIG, config_register_float_test) {
    #define test_float(x) sh_config_register_float(&x, x + 1.0f);
    
    float temp[7] = {1.324, 224.23, 123.3, 123.4, 123.12, 123.4, 1.0};
    float origin[7] = {1.324, 224.23, 123.3, 123.4, 123.12, 123.4, 1.0};

    for (int i = 0; i < 7; i++) {
        test_float(temp[i]);
        EXPECT_FLOAT_EQ(temp[i], origin[i] + 1);
    }
}

TEST_F(TEST_SH_CONFIG, config_write_float_test) {
    float pi = 3.14f;

    sh_config_register_float(&pi, 1.414f);
    EXPECT_FLOAT_EQ(pi, 1.414f);

    pi = 2.523f;
    sh_config_write(&pi);
    EXPECT_FLOAT_EQ(*(float*)&storage[nodes[0].addr], 2.523f);

    pi = 212.3f;
    sh_config_write(&pi);
    EXPECT_FLOAT_EQ(*(float*)&storage[nodes[0].addr], 212.3f);
}

TEST_F(TEST_SH_CONFIG, config_mix_type_test) {
    float pi = 3.14f;
    sh_config_register_float(&pi, 1.414f);
    EXPECT_FLOAT_EQ(pi, 1.414f);

    int8_t money = -3;
    sh_config_register_int8_t(&money, -100);
    EXPECT_EQ(money, -100);

    int32_t debt = -300000;
    sh_config_register_int32_t(&debt, -900000);
    EXPECT_EQ(debt, -900000);

    uint32_t age = 35;
    sh_config_register_uint32_t(&age, 18);
    EXPECT_EQ(age, 18);
    
    pi = 3.14f;
    sh_config_write(&pi);
    EXPECT_FLOAT_EQ(*(float*)&storage[nodes[0].addr], 3.14f);

    money = 14;
    sh_config_write(&money);
    EXPECT_FLOAT_EQ(*(int8_t*)&storage[nodes[1].addr], 14);

    debt= -10000;
    sh_config_write(&debt);
    EXPECT_FLOAT_EQ(*(int32_t*)&storage[nodes[2].addr], -10000);

    age = 40;
    sh_config_write(&age);
    EXPECT_FLOAT_EQ(*(uint32_t*)&storage[nodes[3].addr], 40);
}






