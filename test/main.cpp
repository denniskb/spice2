#include "gtest/gtest.h"

#include "spice/spice.h"

TEST(Main, main) { ASSERT_EQ(spice::return1(), 1); }