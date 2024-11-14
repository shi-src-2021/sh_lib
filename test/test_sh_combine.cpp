#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <stdarg.h>

#include "sh_combine.h"

using namespace testing;




int add_test2(int a, int b)
{
    return a + b;
}

int add_test3(int a, int b, int c)
{
    return a + b + c;
}

int add_test4(int a, int b, int c, int d)
{
    return a + b + c + d;
}

#define add_test(...)   SH_OVERRIDE(add_test, __VA_ARGS__)

TEST(TEST_SH_COMBINE, sh_override_test) {
    EXPECT_EQ(10, add_test(4, 6));
    EXPECT_EQ(15, add_test(4, 6, 5));
    EXPECT_EQ(22, add_test(4, 6, 5, 7));
}




int _sum(int count, ...) {
    int total = 0;
    
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }

    va_end(args);

    return total;
}

#define sum(...)    FN_OVERLOAD(_sum, __VA_ARGS__)

TEST(TEST_SH_COMBINE, function_overload_test) {
    EXPECT_EQ(10, sum(4, 6));
    EXPECT_EQ(15, sum(4, 6, 5));
    EXPECT_EQ(22, sum(4, 6, 5, 7));
}




