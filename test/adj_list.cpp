#include "gtest/gtest.h"

#include "spice/adj_list.h"

using namespace spice;
using namespace spice::util;

TEST(AdjList, Empty) {
	{
		adj_list<void> adj(0, 0, 0);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(0).size(), 0);
	}

	{
		adj_list<void> adj(0, 0, 0.5);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(0).size(), 0);
	}

	{
		adj_list<void> adj(0, 13, 0);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(7).size(), 0);
	}

	{
		adj_list<void> adj(0, 13, 0.5);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(5).size(), 0);
	}

	{
		adj_list<void> adj(11, 0, 0);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(3).size(), 0);
	}

	{
		adj_list<void> adj(11, 0, 0.5);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(2).size(), 0);
	}

	{
		adj_list<void> adj(11, 13, 0);
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(adj.neighbors(1).size(), 0);
	}
}

TEST(AdjList, Ctor) {
	{
		adj_list<void> adj(1, 5, 1);
		ASSERT_EQ(adj.size(), 5);
		ASSERT_EQ(adj.neighbors(0).size(), 5);
		ASSERT_EQ(adj.neighbors(1).size(), 0);

		UInt i = 0;
		for (UInt n : adj.neighbors(0))
			ASSERT_EQ(n, i++);
	}
}