#include <math.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

TEST(normalize_angle, all) {
    EXPECT_DOUBLE_EQ(normalize_angle(0.3 * M_PI), 0.3 * M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(M_PI), M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(-M_PI), -M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(2 * M_PI), 0);
    EXPECT_DOUBLE_EQ(normalize_angle(3 * M_PI), -M_PI);
    EXPECT_DOUBLE_EQ(normalize_angle(-3 * M_PI), M_PI);
}

}

namespace math {
namespace tests {

using namespace testing;

TEST(sin, all) {
    for (int i = 0; i < 100; ++i) {
        const auto angle = i * 2 * M_PI / 100;
        EXPECT_NEAR(sin(angle), std::sin(angle), 1e-15);
    }
}

TEST(cos, all) {
    for (int i = 0; i < 100; ++i) {
        const auto angle = i * 2 * M_PI / 100;
        EXPECT_NEAR(cos(angle), std::cos(angle), 1e-15);
    }
}

}
}
}
