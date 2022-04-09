#include <gtest/gtest.h>

#include <spice/util/stdint.h>

TEST(UInt128, Add) {
	{
		UInt128 a{.lo = 7, .hi = 2};
		auto b = a + 123;

		ASSERT_EQ(b.lo, 130);
		ASSERT_EQ(b.hi, 2);
	}

	{
		UInt128 a{.lo = ~0_u64, .hi = 2};
		auto b = a + 1;

		ASSERT_EQ(b.lo, 0);
		ASSERT_EQ(b.hi, 3);
	}
}