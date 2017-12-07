#include "gtest/gtest.h"
#include "typ/typ_ij.h"
#include "typ/typ_json.h"

using typ::IJ;

TEST(IJ, Trivia) {
    IJ ij1;
    EXPECT_EQ(0, ij1.i);
    EXPECT_EQ(0, ij1.j);
    IJ ij2(2, 3);
    EXPECT_EQ(2, ij2.i);
    EXPECT_EQ(3, ij2.j);
}

TEST(IJ, Compare) {
    IJ ij(1, 2), ij1(1, 2), ij2(1, 0), ij3(2, 2);
    EXPECT_EQ(0, ij.compare(ij));
    EXPECT_EQ(0, ij.compare(ij1));
    EXPECT_EQ(1, ij.compare(ij2));
    EXPECT_EQ(-1, ij.compare(ij3));

    EXPECT_EQ(ij, ij1);
    EXPECT_NE(ij, ij2);
}

TEST(IJ, Json) {
    IJ ij(-1, 2), ij1;
    ij1.loadJson(ij.saveJson());
    EXPECT_EQ(ij, ij1);
}
