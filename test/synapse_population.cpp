#include "gtest/gtest.h"

#include <concepts>

#include "spice/synapse_population.h"

using namespace spice;
using namespace spice::util;

struct stateful_neuron {
	bool update(double) { return false; }
};
struct stateless_synapse {
	static void deliver(stateful_neuron&) {}
};
struct stateful_synapse {
	void deliver(stateful_neuron&) {}
};
static_assert(StatelessSynapse<stateless_synapse, stateful_neuron>);
static_assert(!StatefulSynapse<stateless_synapse, stateful_neuron>);
static_assert(StatefulSynapse<stateful_synapse, stateful_neuron>);
static_assert(!StatelessSynapse<stateful_synapse, stateful_neuron>);
static_assert(Synapse<stateless_synapse, stateful_neuron>);
static_assert(Synapse<stateful_synapse, stateful_neuron>);

#if 0
TEST(AdjList, Empty) {
	auto assert_empty = [](auto const& adj) {
		ASSERT_EQ(adj.size(), 0);
		ASSERT_EQ(std::distance(adj.neighbors(0).begin(), adj.neighbors(0).end()), 0);
		ASSERT_EQ(std::distance(adj.neighbors(1).begin(), adj.neighbors(1).end()), 0);
		ASSERT_EQ(std::distance(adj.neighbors(2).begin(), adj.neighbors(2).end()), 0);
		ASSERT_EQ(std::distance(adj.neighbors(5).begin(), adj.neighbors(5).end()), 0);
		ASSERT_EQ(adj.begin(), adj.end());
		ASSERT_EQ(adj.cbegin(), adj.end());
		ASSERT_EQ(std::distance(adj.begin(), adj.end()), 0);
		ASSERT_EQ(std::distance(adj.cbegin(), adj.cend()), 0);
	};

	assert_empty(synapse_population<void>(0, 0, 0));
	assert_empty(synapse_population<void>(0, 0, 0.5));
	assert_empty(synapse_population<void>(0, 13, 0));
	assert_empty(synapse_population<void>(0, 13, 0.5));
	assert_empty(synapse_population<void>(11, 0, 0));
	assert_empty(synapse_population<void>(11, 0, 0.5));
	assert_empty(synapse_population<void>(11, 13, 0));
}

TEST(AdjList, Ctor) {
	{
		synapse_population<void> adj(2, 5, 1);
		ASSERT_EQ(adj.size(), 10);
		ASSERT_EQ(std::distance(adj.neighbors(0).begin(), adj.neighbors(0).end()), 5);
		ASSERT_EQ(std::distance(adj.neighbors(1).begin(), adj.neighbors(1).end()), 5);
		ASSERT_EQ(std::distance(adj.neighbors(2).begin(), adj.neighbors(2).end()), 0);

		{
			Int i = 0;
			for (auto n : adj.neighbors(0)) {
				ASSERT_EQ(n.src, 0);
				ASSERT_EQ(n.dst, i++);
			}
		}
		{
			Int i = 0;
			for (auto n : adj.neighbors(1)) {
				ASSERT_EQ(n.src, 1);
				ASSERT_EQ(n.dst, i++);
			}
		}
	}
}

TEST(AdjList, Distr) {
	std::random_device rd;
	UInt const seed = rd();
	synapse_population<void> adj(1, 100'000, 0.1, {seed});

	{
		Int prev_src = -1;
		Int prev_dst = -1;
		for (auto n : adj.neighbors(0)) {
			ASSERT_LE(prev_src, n.src) << "seed: " << seed;
			ASSERT_LT(prev_dst, n.dst) << "seed: " << seed;
			prev_src = n.src;
			prev_dst = n.dst;
		}
	}

	auto const neighbors = adj.neighbors(0);
	EXPECT_TRUE(9850 <= std::distance(neighbors.begin(), neighbors.end()) &&
	            std::distance(neighbors.begin(), neighbors.end()) <= 10150)
	    << "neighbors.size(): " << std::distance(neighbors.begin(), neighbors.end()) << ", seed: " << seed;

	double kolmogorov_smirnov = 0;
	for (Int x = 1000; x <= 99'000; x += 1000) {
		synapse_population<void>::iterator::edge e{0, x, nullptr};
		double const cdf =
		    std::distance(neighbors.begin(),
		                  std::lower_bound(neighbors.begin(), neighbors.end(), e,
		                                   [](auto lhs, auto rhs) {
			                                   return lhs.src < rhs.src ||
			                                          (lhs.src == rhs.src && lhs.dst < rhs.dst);
		                                   })) /
		    1e4;
		kolmogorov_smirnov = std::max(kolmogorov_smirnov, cdf - x / 1e5);
	}

	EXPECT_LT(kolmogorov_smirnov, 0.014) << "seed: " << seed;
}

TEST(AdjList, Collisions) {
	std::random_device rd;
	UInt const seed = rd();
	synapse_population<void> adj(1, 10'000, 0.99, {seed});

	Int prev_src = -1;
	Int prev_dst = -1;
	for (auto n : adj.neighbors(0)) {
		ASSERT_LE(prev_src, n.src) << "seed: " << seed;
		ASSERT_LT(prev_dst, n.dst) << "seed: " << seed;
		prev_src = n.src;
		prev_dst = n.dst;
	}
}

template <std::input_iterator It>
void test_iterator_requirements() {}
TEST(AdjList, IteratorRequirements) {
	test_iterator_requirements<synapse_population<void>::iterator>();
	test_iterator_requirements<synapse_population<void>::const_iterator>();
	test_iterator_requirements<synapse_population<int>::iterator>();
	test_iterator_requirements<synapse_population<int>::const_iterator>();
}
#endif