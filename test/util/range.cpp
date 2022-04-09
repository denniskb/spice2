#include "gtest/gtest.h"

#include "spice/util/range.h"

using namespace spice;
using namespace spice::util;

TEST(Range, FromNumbers) {
	{
		auto r = range(0);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 0);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}

	{
		auto r = range(10);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 10);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++);

		ASSERT_EQ(count, 10);
	}

	{
		auto r = range(2, 7);
		ASSERT_EQ(*r.begin(), 2);
		ASSERT_EQ(*r.end(), 7);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++ + 2);

		ASSERT_EQ(count, 5);
	}

	{
		auto r = range(7, -3);
		ASSERT_EQ(*r.begin(), 7);
		ASSERT_EQ(*r.end(), 7);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}
}

TEST(Range, FromContainer) {
	std::vector<int> x(3);

	int count = 0;
	for (Int i : range(x))
		count++, (void)i;

	ASSERT_EQ(count, 3);
}

TEST(Range, FromIterators) {
	std::vector<int> x(3);

	int count = 0;
	for (Int i : range(x.begin(), x.end()))
		count++, (void)i;

	ASSERT_EQ(count, 3);
}