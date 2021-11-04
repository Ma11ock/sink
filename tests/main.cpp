#include <gtest/gtest.h>

#include "../src/sink.hpp"

TEST(reprTest, zero)
{
    ASSERT_EQ(UINT32_C(0), Sink32(0).getValue());
}


TEST(reprTest, one)
{
    ASSERT_EQ(1.f, static_cast<float>(Sink32(1.f)));
}

TEST(convTest, one)
{
    float tmp = 1.;
    ASSERT_EQ(*reinterpret_cast<std::uint32_t*>(&tmp), Sink32(UINT32_C(1)).getValue());
}

TEST(convTest, two)
{
    float tmp = 2.;
    ASSERT_EQ(*reinterpret_cast<std::uint32_t*>(&tmp), Sink32(UINT32_C(2)).getValue());
}

TEST(convTest, three)
{
    float tmp = 3.;
    ASSERT_EQ(*reinterpret_cast<std::uint32_t*>(&tmp), Sink32(UINT32_C(3)).getValue());
}

TEST(convTest, twoPointFive)
{
    float tmp = 2.5f;
    ASSERT_EQ(*reinterpret_cast<std::uint32_t*>(&tmp), Sink32(2.5f).getValue());
}

TEST(castTest, one)
{
    ASSERT_EQ(1, static_cast<std::int32_t>(Sink32(1)));
}

TEST(castTest, two)
{
    ASSERT_EQ(2, static_cast<std::int32_t>(Sink32(2)));
}

TEST(castTest, three)
{
    ASSERT_EQ(3, static_cast<std::int32_t>(Sink32(3)));
}

TEST(add, onep0)
{
    ASSERT_EQ(1.f, static_cast<float>(Sink32(1) + Sink32(0)));
}

TEST(add, onepone)
{
    ASSERT_EQ(2.f, static_cast<float>(Sink32(1) + Sink32(1)));
}

TEST(add, oneptwo)
{
    ASSERT_EQ(3.f, static_cast<float>(Sink32(1) + Sink32(2)));
}

TEST(add, hundredphundredtwentyfive)
{
    ASSERT_EQ(125.f, static_cast<float>(Sink32(100) + Sink32(25)));
}

TEST(sub, hunredmtwentyfive)
{
    ASSERT_EQ(75.f, static_cast<float>(Sink32(100) - Sink32(25)));
}

TEST(sub, onemone)
{
    ASSERT_EQ(0.f, static_cast<float>(Sink32(1) - Sink32(1)));
}

TEST(sub, onemtwo)
{
    ASSERT_EQ(-1.f, static_cast<float>(Sink32(1) - Sink32(2)));
}

TEST(sub, onemthree)
{
    ASSERT_EQ(-2.f, static_cast<float>(Sink32(1) - Sink32(3)));
}

// Mathematical exp of a subnormal number should always be -126
TEST(frexpDenormal, zero)
{
    ASSERT_EQ(-126, Sink32(0).frexp());
}

// Frexp tests for normals.

TEST(frexp, one)
{
    ASSERT_EQ(0, Sink32(1).frexp());
}

TEST(frexp, two)
{
    ASSERT_EQ(1, Sink32(2).frexp());
}

TEST(frexp, three)
{
    ASSERT_EQ(1, Sink32(3).frexp());
}

TEST(frexp, four)
{
    ASSERT_EQ(2, Sink32(4).frexp());
}

TEST(frexp, hundred)
{
    ASSERT_EQ(6, Sink32(100).frexp());
}


TEST(frexp, fIntMax)
{
    auto s = Sink32(16777215);
    ASSERT_EQ(23, s.frexp());
}

TEST(mult, onemulone)
{
    Sink32 i = Sink32(1) * Sink32(1);
    ASSERT_EQ(1.f, static_cast<float>(i));
}

TEST(mult, onemultwo)
{
    Sink32 i = Sink32(1) * Sink32(2);
    ASSERT_EQ(2.f, static_cast<float>(i));
}

TEST(mult, zeroMultTwo)
{
    Sink32 i = Sink32(0) * Sink32(2);
    ASSERT_EQ(0.f, static_cast<float>(i));
}

TEST(mult, twoMulTwo)
{
    Sink32 i = Sink32(2) * Sink32(2);
    ASSERT_EQ(4.f, static_cast<float>(i));
}

TEST(mult, multSignTest)
{
    Sink32 i = Sink32(-1) * Sink32(2);
    ASSERT_EQ(-2.f, static_cast<float>(i));
}


TEST(div, onedivone)
{
    ASSERT_EQ(1.f, static_cast<float>(Sink32(1) / Sink32(1)));
}

int main(int argc, char *argv[])
{
    std::cout << "The size of a sink32 is " << sizeof(Sink32) << '\n';
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
