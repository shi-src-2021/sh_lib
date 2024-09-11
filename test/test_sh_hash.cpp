#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "sh_hash.h"

using namespace testing;

class TEST_SH_HASH : public testing::Test {
protected:  
    void SetUp()
    {
        hash = sh_hash_create(3);
        ASSERT_TRUE(hash);

        EXPECT_EQ(0, sh_hash_add(hash, "111", (void*)"aaa"));
        EXPECT_EQ(0, sh_hash_add(hash, "222", (void*)"bbb"));
        EXPECT_EQ(0, sh_hash_add(hash, "11", (void*)"aa"));
    }

    void TearDown()
    {
        sh_hash_destroy(hash);
        hash = NULL;
    }

    sh_hash_t *hash;
};

TEST_F(TEST_SH_HASH, sh_hash_find_test) {
    char *buf = NULL;
    EXPECT_EQ(0, sh_hash_find(hash, "111", (void**)&buf));
    EXPECT_EQ(buf, "aaa");

    buf = NULL;
    EXPECT_EQ(0, sh_hash_find(hash, "222", (void**)&buf));
    EXPECT_EQ(buf, "bbb");

    buf = NULL;
    EXPECT_EQ(0, sh_hash_find(hash, "11", (void**)&buf));
    EXPECT_EQ(buf, "aa");

    EXPECT_EQ(-1, sh_hash_find(hash, "333", NULL));
}

TEST_F(TEST_SH_HASH, sh_hash_add_overflow_test) {
    EXPECT_EQ(-1, sh_hash_add(hash, "1111", (void*)"aa"));
}

TEST_F(TEST_SH_HASH, sh_hash_used_amount_test) {
    EXPECT_EQ(3, sh_hash_get_used_amount(hash));
    sh_hash_clear(hash);
    EXPECT_EQ(0, sh_hash_get_used_amount(hash));
}

TEST_F(TEST_SH_HASH, sh_hash_is_key_exist_test) {
    EXPECT_EQ(false, sh_hash_is_key_exist(hash, "1111"));
    EXPECT_EQ(true, sh_hash_is_key_exist(hash, "111"));
}

TEST_F(TEST_SH_HASH, sh_hash_remove_test) {
    EXPECT_EQ(3, sh_hash_get_used_amount(hash));
    EXPECT_EQ(true, sh_hash_is_key_exist(hash, "111"));
    
    sh_hash_remove(hash, "111");

    EXPECT_EQ(2, sh_hash_get_used_amount(hash));
    EXPECT_EQ(false, sh_hash_is_key_exist(hash, "111"));

    sh_hash_remove(hash, "1111");

    EXPECT_EQ(2, sh_hash_get_used_amount(hash));
    EXPECT_EQ(false, sh_hash_is_key_exist(hash, "1111"));
}



