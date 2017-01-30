#include <target.hpp>

#include <gtest/gtest.h>

namespace strategy {
namespace tests {

using namespace testing;

TEST(Target, not_equals) {
    EXPECT_FALSE(Target() != Target());
    EXPECT_TRUE(Target(Id<model::Bonus>(1)) != Target());
    EXPECT_TRUE(Target(Id<model::Bonus>(1)) != Target(Id<model::Building>(1)));
    EXPECT_TRUE(Target(Id<model::Bonus>(1)) != Target(Id<model::Bonus>(2)));
    EXPECT_FALSE(Target(Id<model::Bonus>(1)) != Target(Id<model::Bonus>(1)));
}

}
}
