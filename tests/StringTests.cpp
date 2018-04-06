#include "io/MotString.h"
#include "gtest/gtest.h"

using mot::MotString;

TEST(MotString, LiteralEquality)
{
    {
        auto a = MotString("test");
        auto b = MotString("test");

        EXPECT_TRUE(a.IsEqualTo(&a));
        EXPECT_TRUE(a.IsEqualTo(&b));
        EXPECT_TRUE(b.IsEqualTo(&b));
        EXPECT_TRUE(b.IsEqualTo(&a));
    }

    {
        auto a = MotString("one");
        auto b = MotString("two");
    }

    {
        auto a = MotString("test");
        auto b = MotString("TEST");
        auto c = MotString("TeSt");

        EXPECT_FALSE(a.IsEqualTo(&b));
        EXPECT_FALSE(a.IsEqualTo(&c));
        EXPECT_FALSE(b.IsEqualTo(&a));
        EXPECT_FALSE(b.IsEqualTo(&c));
        EXPECT_FALSE(c.IsEqualTo(&a));
        EXPECT_FALSE(c.IsEqualTo(&b));
    }
}

TEST(MotString, LiteralCaseInsensitive)
{
    auto a = MotString("test");
    auto b = MotString("TEST");
    auto c = MotString("TeSt");

    EXPECT_TRUE(a.IsCaseInsensitiveEqualTo(&b));
    EXPECT_TRUE(a.IsCaseInsensitiveEqualTo(&c));
    EXPECT_TRUE(b.IsCaseInsensitiveEqualTo(&a));
    EXPECT_TRUE(b.IsCaseInsensitiveEqualTo(&c));
    EXPECT_TRUE(c.IsCaseInsensitiveEqualTo(&a));
    EXPECT_TRUE(c.IsCaseInsensitiveEqualTo(&b));
}
