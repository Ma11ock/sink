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

int main(int argc, char *argv[])
{
    std::cout << "The size of a sink32 is " << sizeof(Sink32) << '\n';
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
