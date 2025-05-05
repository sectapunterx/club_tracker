#include <gtest/gtest.h>
#include <time_utils.hpp>

TEST(TimeUtils, Parse)
{
    EXPECT_EQ(cc::Time::Parse("00:00"), cc::Time{0});
    EXPECT_EQ(cc::Time::Parse("01:00"), cc::Time{60});
    EXPECT_EQ(cc::Time::Parse("12:34"), cc::Time{754});
    EXPECT_EQ(cc::Time::Parse("23:59"), cc::Time{1439});

    EXPECT_THROW(cc::Time::Parse("24:00"), std::invalid_argument);
    EXPECT_THROW(cc::Time::Parse("12:60"), std::invalid_argument);
    EXPECT_THROW(cc::Time::Parse("12:34:56"), std::invalid_argument);
}