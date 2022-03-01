#include "gtest/gtest.h"

#include "spice/detail/range.h"

using namespace spice;
using namespace spice::detail;

TEST(Detail, Range) {
	{
		range r(0);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 0);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}

	{
		range r(10);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 10);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++);

		ASSERT_EQ(count, 10);
	}

	{
		range r(2, 7);
		ASSERT_EQ(*r.begin(), 2);
		ASSERT_EQ(*r.end(), 7);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++ + 2);

		ASSERT_EQ(count, 5);
	}

	{
		range r(7, -3);
		ASSERT_EQ(*r.begin(), 7);
		ASSERT_EQ(*r.end(), 7);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}
}