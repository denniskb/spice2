#include "gtest/gtest.h"

#include <concepts>

#include "spice/detail/csr.h"

using namespace spice;
using namespace spice::detail;

TEST(CSR, Iterator) {
	static_assert(std::input_iterator<csr<>::iterator>);
	static_assert(std::input_iterator<csr<>::const_iterator>);
	static_assert(std::input_iterator<csr<int>::iterator>);
	static_assert(std::input_iterator<csr<int>::const_iterator>);
}

TEST(CSR, Empty) {
	adj_list adj;
	adj(1, 0);
	csr c(adj, {1337});

	ASSERT_EQ(c.neighbors(0).size(), 0);
}

TEST(CSR, Custom) {
	adj_list adj;

	ASSERT_EQ(adj.size(), 0);

	adj.connect(0, 0);
	adj.connect(0, 2);
	adj.connect(0, 7);
	adj.connect(0, 9);
	adj.connect(1, 1);
	adj.connect(1, 2);
	adj(2, 10);

	csr c(adj, {1337});

	ASSERT_EQ(c.neighbors(0).size(), 4);
	ASSERT_EQ(c.neighbors(1).size(), 2);

	{
		std::vector<std::pair<Int32, void*>> neighbors(c.neighbors(0).begin(), c.neighbors(0).end());
		ASSERT_EQ(neighbors[0].first, 0);
		ASSERT_EQ(neighbors[1].first, 2);
		ASSERT_EQ(neighbors[2].first, 7);
		ASSERT_EQ(neighbors[3].first, 9);
	}

	{
		std::vector<std::pair<Int32, void*>> neighbors(c.neighbors(1).begin(), c.neighbors(1).end());
		ASSERT_EQ(neighbors[0].first, 1);
		ASSERT_EQ(neighbors[1].first, 2);
	}
}

TEST(CSR, Payload) {
	adj_list adj;
	adj.connect(0, 0);
	adj(1, 1);
	csr<int> c(adj, {1337});

	std::vector<std::pair<Int32, int*>> neighbors(c.neighbors(0).begin(), c.neighbors(0).end());
	ASSERT_EQ(neighbors.size(), 1);
}