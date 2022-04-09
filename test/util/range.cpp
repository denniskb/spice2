#include "gtest/gtest.h"

#include "spice/util/range.h"

using namespace spice;
using namespace spice::util;

// TODO: Make int_iterator a proper iterator
//static_assert(std::random_access_iterator<int_iterator>);

TEST(Range, FromNumbers) {
	{
		auto r = range(0);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 0);
		ASSERT_EQ(r.size(), 0);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}

	{
		auto r = range(10);
		ASSERT_EQ(*r.begin(), 0);
		ASSERT_EQ(*r.end(), 10);
		ASSERT_EQ(r.size(), 10);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++);

		ASSERT_EQ(count, 10);
	}

	{
		auto r = range(2, 7);
		ASSERT_EQ(*r.begin(), 2);
		ASSERT_EQ(*r.end(), 7);
		ASSERT_EQ(r.size(), 5);

		int count = 0;
		for (Int i : r)
			ASSERT_EQ(i, count++ + 2);

		ASSERT_EQ(count, 5);
	}

	{
		auto r = range(7, -3);
		ASSERT_EQ(*r.begin(), 7);
		ASSERT_EQ(*r.end(), 7);
		ASSERT_EQ(r.size(), 0);

		int count = 0;
		for (Int i : r)
			count++, (void)i;

		ASSERT_EQ(count, 0);
	}
}

TEST(Range, FromContainer) {
	std::vector<int> x(3);
	auto r = range(x);

	ASSERT_EQ(r.size(), 3);

	int count = 0;
	for (Int i : r)
		count++, (void)i;

	ASSERT_EQ(count, 3);
}

TEST(Range, FromIterators) {
	std::vector<int> x(3);
	auto r = range(x.begin(), x.end());

	ASSERT_EQ(r.size(), 3);

	int count = 0;
	for (Int i : r)
		count++, (void)i;

	ASSERT_EQ(count, 3);
}