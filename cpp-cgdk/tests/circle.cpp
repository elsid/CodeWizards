#include <circle.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(Circle, has_intersection_with_circle) {
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(0, 0), 1)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(1, 0), 1)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(2, 0), 1)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(2 + 1e-8 - 1e-9, 0), 1)));
    EXPECT_FALSE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(2 + 1e-8, 0), 1)));
    EXPECT_FALSE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(3, 0), 1)));
}

TEST(Circle, has_intersection_with_line) {
    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(0, 0), Point(0, 0))));
    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(0, 0), Point(1, 0))));
    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(0, 0), Point(10, 0))));

    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(2, 1), Point(-2, 1))));
    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(2, 2), Point(-2, 2))));
    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(2, 2 + 1e-8 - 1e-9), Point(-2, 2 + 1e-8 - 1e-9))));
    EXPECT_FALSE(Circle(Point(0, 0), 2).has_intersection(Line(Point(2, 2 + 1e-8), Point(-2, 2 + 1e-8))));

    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(3, 1), Point(1, 1))));
    EXPECT_FALSE(Circle(Point(0, 0), 2).has_intersection(Line(Point(3, 1), Point(2, 1))));

    EXPECT_TRUE(Circle(Point(0, 0), 2).has_intersection(Line(Point(1, 1), Point(3, 1))));
    EXPECT_FALSE(Circle(Point(0, 0), 2).has_intersection(Line(Point(2, 1), Point(3, 1))));
}

TEST(Circle, has_intersection_with_moving_circle) {
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(0, 0), 1), Point(0, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Circle(Point(0, 0), 1), Point(1, 0)));
}

TEST(Circle, has_intersection_moving_with_moving_circle) {
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(0, 0), Circle(Point(0, 0), 1), Point(0, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(0, 0), 1), Point(0, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(0, 0), Circle(Point(0, 0), 1), Point(1, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(0, 0), 1), Point(1, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(10, 0), Circle(Point(5, 5), 1), Point(5, -5)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(2, 2), 1), Point(2, 1)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(3, 3), 1), Point(3, 0)));
    EXPECT_TRUE(Circle(Point(0, 0), 1).has_intersection(Point(3, 0), Circle(Point(3, 3), 1), Point(3, 2)));
    EXPECT_FALSE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(4, 4), 1), Point(4, 0)));
    EXPECT_FALSE(Circle(Point(0, 0), 1).has_intersection(Point(4, 0), Circle(Point(4, 4), 1), Point(4, 3)));
    EXPECT_FALSE(Circle(Point(0, 0), 1).has_intersection(Point(1, 0), Circle(Point(3, 3), 1), Point(3, 2)));
}

}
}
