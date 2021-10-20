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

int main(int argc, char *argv[])
{
    std::cout << "The size of a sink32 is " << sizeof(Sink32) << '\n';
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
