#include <line.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(Line, length) {
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(1, 0)).length(), 1);
}

TEST(Line, distance) {
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(0, 0)).distance(Point(0, 0)), 0);
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(0, 0)).distance(Point(0, 1)), 1);
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(1, 0)).distance(Point(0.5, 1)), 1);
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(1, 0)).distance(Point(-0.5, 1)), 1);
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(1, 0)).distance(Point(0, 1)), 1);
    EXPECT_DOUBLE_EQ(Line(Point(0, 0), Point(1, 0)).distance(Point(-1, 1)), 1);
}

} // namespace tests
} // namespace strategy
