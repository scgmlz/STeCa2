#include "def/numbers.h"
#include "gtest/gtest.h"

TEST(Numbers, NanInf) {
    double d;
    d = float(NAN);
    EXPECT_TRUE(qIsNaN(d));
    d = float(INF);
    EXPECT_TRUE(qIsInf(d));
    float f;
    f = float(NAN);
    EXPECT_TRUE(qIsNaN(f));
    f = float(INF);
    EXPECT_TRUE(qIsInf(f));
}
