#include "gtest/gtest.h"

#include "spice/detail/neuron_population.h"

using namespace spice;
using namespace spice::detail;
using namespace spice::util;

struct stateless_neuron {
	static bool fire;
	bool update(float, auto&) const { return fire; }
};
bool stateless_neuron::fire = false;
static_assert(StatelessNeuron<stateless_neuron>);

TEST(NeuronPopulation, Stateless) {
	seed_seq seed{1337};
	xoroshiro64_128p rng(seed);
	neuron_population<stateless_neuron> pop({}, 5, seed, 2);
	pop.plastic();

	ASSERT_EQ(pop.size(), 5);

	stateless_neuron::fire = false;
	pop.update(2, 1, rng);
	ASSERT_EQ(pop.spikes(0).size(), 0);

	stateless_neuron::fire = true;
	pop.update(2, 1, rng);
	ASSERT_EQ(pop.spikes(0).size(), 5);
	for (Int i : range(5)) {
		ASSERT_EQ(pop.spikes(0)[i], i);
		ASSERT_EQ(pop.history()[i], 1);
	}
}

struct stateful_neuron {
	struct neuron {
		bool fired = false;
		Int id     = 0;
	};

	bool update(neuron& n, float, auto&) const {
		n.fired = true;
		return n.id % 2;
	}
};
static_assert(StatefulNeuron<stateful_neuron>);

TEST(NeuronPopulation, Stateful) {
	seed_seq seed{1337};
	xoroshiro64_128p rng(seed);
	neuron_population<stateful_neuron> pop({}, 5, seed, 1);
	pop.plastic();

	ASSERT_EQ(pop.size(), 5);
	for (auto& n : pop.get_neurons())
		ASSERT_FALSE(n.fired);

	pop.update(1, 1, rng);

	for (auto& n : pop.get_neurons())
		ASSERT_TRUE(n.fired);

	ASSERT_EQ(pop.spikes(0).size(), 0);
}

struct per_neuron_init : public stateful_neuron {
	void init(neuron& n, Int id, auto&) const { n.id = id; }
};
static_assert(PerNeuronInit<per_neuron_init>);

TEST(NeuronPopulation, PerNeuronInit) {
	seed_seq seed{1337};
	xoroshiro64_128p rng(seed);
	neuron_population<per_neuron_init> pop({}, 5, seed, 1);
	pop.plastic();

	ASSERT_EQ(pop.size(), 5);
	for (auto& n : pop.get_neurons())
		ASSERT_FALSE(n.fired);

	pop.update(1, 1, rng);

	for (auto& n : pop.get_neurons())
		ASSERT_TRUE(n.fired);

	ASSERT_EQ(pop.spikes(0).size(), 2);

	{
		Int i = 1;
		for (auto s : pop.spikes(0)) {
			ASSERT_EQ(s, i);
			i += 2;
		}
	}

	for (Int i : range(5))
		ASSERT_EQ(pop.history()[i], i % 2);
}

struct per_population_init : public stateful_neuron {
	void init(std::span<neuron> neurons, auto&) {
		for (Int i : range(neurons))
			neurons[i].id = i;
	}
};
static_assert(PerPopulationInit<per_population_init>);

TEST(NeuronPopulation, PerPopulationInit) {
	seed_seq seed{1337};
	xoroshiro64_128p rng(seed);
	neuron_population<per_population_init> pop({}, 5, seed, 1);
	pop.plastic();

	ASSERT_EQ(pop.size(), 5);
	for (auto& n : pop.get_neurons())
		ASSERT_FALSE(n.fired);

	pop.update(1, 1, rng);

	for (auto& n : pop.get_neurons())
		ASSERT_TRUE(n.fired);

	ASSERT_EQ(pop.spikes(0).size(), 2);

	{
		Int i = 1;
		for (auto s : pop.spikes(0)) {
			ASSERT_EQ(s, i);
			i += 2;
		}
	}

	for (Int i : range(5))
		ASSERT_EQ(pop.history()[i], i % 2);
}

struct per_population_update {
	void update(float, auto&, std::vector<Int32>& spikes) {
		spikes.insert(spikes.end(), {1, 3, 8});
	}
};
static_assert(PerPopulationUpdate<per_population_update>);

TEST(NeuronPopulation, PerPopulationUpdate) {
	seed_seq seed{1337};
	xoroshiro64_128p rng(seed);
	neuron_population<per_population_update> pop({}, 10, seed, 1);
	pop.plastic();

	ASSERT_EQ(pop.size(), 10);

	pop.update(1, 1, rng);
	ASSERT_EQ(pop.spikes(0).size(), 3);
	ASSERT_EQ(pop.spikes(0)[0], 1);
	ASSERT_EQ(pop.spikes(0)[1], 3);
	ASSERT_EQ(pop.spikes(0)[2], 8);

	ASSERT_EQ(pop.history()[0], 0);
	ASSERT_EQ(pop.history()[1], 1);
	ASSERT_EQ(pop.history()[2], 0);
	ASSERT_EQ(pop.history()[3], 1);
	ASSERT_EQ(pop.history()[4], 0);
	ASSERT_EQ(pop.history()[5], 0);
	ASSERT_EQ(pop.history()[6], 0);
	ASSERT_EQ(pop.history()[7], 0);
	ASSERT_EQ(pop.history()[8], 1);
	ASSERT_EQ(pop.history()[9], 0);
}