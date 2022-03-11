#include "gtest/gtest.h"

#include "spice/neuron_population.h"

using namespace spice;
using namespace spice::util;

struct stateless_neuron {
	static bool fire;
	static bool update(float) { return fire; }
};
bool stateless_neuron::fire = false;

struct stateful_neuron {
	int i = 0;
	bool update(float dt) {
		i++;
		return dt >= 0.5f;
	}
};
struct stateless_neuron_with_params {
	static bool update(float, bool p) { return p; }
};
struct stateful_neuron_with_params {
	int i = 0;
	bool update(float, bool p) {
		i++;
		return p;
	}
};

TEST(NeuronPopulation, Stateless) {
	neuron_population<stateless_neuron> pop(5, 2);
	stateless_neuron::fire = false;
	pop.update(2, 0);

	ASSERT_EQ(pop.spikes(0).size(), 0);

	stateless_neuron::fire = true;
	pop.update(2, 0);
	ASSERT_EQ(pop.spikes(1).size(), 0);
	ASSERT_EQ(pop.spikes(0).size(), 5);
	for (Int i : range(5))
		ASSERT_EQ(pop.spikes(0)[i], i), (void)i;

	for (Int i : range(5))
		ASSERT_EQ(pop.history()[i], 1), (void)i;
}

TEST(NeuronPopulation, StatelessWithParams) {
	{
		neuron_population<stateless_neuron_with_params, bool> pop(5, 1, false);
		pop.update(1, 0);
		ASSERT_EQ(pop.spikes(0).size(), 0);

		for (Int i : range(5))
			ASSERT_EQ(pop.history()[i], 0), (void)i;
	}
	{
		neuron_population<stateless_neuron_with_params, bool> pop(5, 1, true);
		pop.update(1, 0);
		ASSERT_EQ(pop.spikes(0).size(), 5);
		for (Int i : range(5))
			ASSERT_EQ(pop.spikes(0)[i], i), (void)i;

		for (Int i : range(5))
			ASSERT_EQ(pop.history()[i], 1), (void)i;
	}
}

TEST(NeuronPopulation, Stateful) {
	neuron_population<stateful_neuron> pop(5, 1);
	for (Int i : range(5))
		ASSERT_EQ(pop.get_neurons()[i].i, 0), (void)i;

	pop.update(1, 1);
	ASSERT_EQ(pop.spikes(0).size(), 5);
	for (Int i : range(5))
		ASSERT_EQ(pop.spikes(0)[i], i), (void)i;

	for (Int i : range(5))
		ASSERT_EQ(pop.get_neurons()[i].i, 1), (void)i;

	pop.update(1, 0);
	ASSERT_EQ(pop.spikes(0).size(), 0);

	for (Int i : range(5))
		ASSERT_EQ(pop.get_neurons()[i].i, 2), (void)i;

	for (Int i : range(5))
		ASSERT_EQ(pop.history()[i], 2), (void)i;
}

TEST(NeuronPopulation, StatefulWithParams) {
	{
		neuron_population<stateful_neuron_with_params, bool> pop(5, 1, false);
		for (Int i : range(5))
			ASSERT_EQ(pop.get_neurons()[i].i, 0), (void)i;

		pop.update(1, 0);
		ASSERT_EQ(pop.spikes(0).size(), 0);

		for (Int i : range(5))
			ASSERT_EQ(pop.get_neurons()[i].i, 1), (void)i;

		for (Int i : range(5))
			ASSERT_EQ(pop.history()[i], 0), (void)i;
	}
	{
		neuron_population<stateful_neuron_with_params, bool> pop(5, 1, true);
		for (Int i : range(5))
			ASSERT_EQ(pop.get_neurons()[i].i, 0), (void)i;

		pop.update(1, 0);
		ASSERT_EQ(pop.spikes(0).size(), 5);
		for (Int i : range(5))
			ASSERT_EQ(pop.spikes(0)[i], i), (void)i;

		for (Int i : range(5))
			ASSERT_EQ(pop.get_neurons()[i].i, 1), (void)i;

		for (Int i : range(5))
			ASSERT_EQ(pop.history()[i], 1), (void)i;
	}
}

static_assert(!StatelessNeuronWithoutParams<int>);
static_assert(!StatelessNeuronWithParams<int, any_t>);
static_assert(!StatefulNeuronWithoutParams<int>);
static_assert(!StatefulNeuronWithParams<int, any_t>);
static_assert(!StatelessNeuron<int>);
static_assert(!StatefulNeuron<int>);
static_assert(!NeuronWithoutParams<int>);
static_assert(!NeuronWithParams<int, any_t>);
static_assert(!Neuron<int>);

static_assert(StatelessNeuronWithoutParams<stateless_neuron>);
static_assert(!StatelessNeuronWithParams<stateless_neuron, any_t>);
static_assert(!StatefulNeuronWithoutParams<stateless_neuron>);
static_assert(!StatefulNeuronWithParams<stateless_neuron, any_t>);
static_assert(StatelessNeuron<stateless_neuron>);
static_assert(!StatefulNeuron<stateless_neuron>);
static_assert(NeuronWithoutParams<stateless_neuron>);
static_assert(!NeuronWithParams<stateless_neuron, any_t>);
static_assert(Neuron<stateless_neuron>);

static_assert(!StatelessNeuronWithoutParams<stateful_neuron>);
static_assert(!StatelessNeuronWithParams<stateful_neuron, any_t>);
static_assert(StatefulNeuronWithoutParams<stateful_neuron>);
static_assert(!StatefulNeuronWithParams<stateful_neuron, any_t>);
static_assert(!StatelessNeuron<stateful_neuron>);
static_assert(StatefulNeuron<stateful_neuron>);
static_assert(NeuronWithoutParams<stateful_neuron>);
static_assert(!NeuronWithParams<stateful_neuron, any_t>);
static_assert(Neuron<stateful_neuron>);

static_assert(!StatelessNeuronWithoutParams<stateless_neuron_with_params>);
static_assert(StatelessNeuronWithParams<stateless_neuron_with_params, any_t>);
static_assert(StatelessNeuronWithParams<stateless_neuron_with_params, int>);
static_assert(!StatefulNeuronWithoutParams<stateless_neuron_with_params>);
static_assert(!StatefulNeuronWithParams<stateless_neuron_with_params, any_t>);
static_assert(StatelessNeuron<stateless_neuron_with_params>);
static_assert(!StatefulNeuron<stateless_neuron_with_params>);
static_assert(!NeuronWithoutParams<stateless_neuron_with_params>);
static_assert(NeuronWithParams<stateless_neuron_with_params, any_t>);
static_assert(NeuronWithParams<stateless_neuron_with_params, int>);
static_assert(Neuron<stateless_neuron_with_params>);

static_assert(!StatelessNeuronWithoutParams<stateful_neuron_with_params>);
static_assert(!StatelessNeuronWithParams<stateful_neuron_with_params, any_t>);
static_assert(!StatefulNeuronWithoutParams<stateful_neuron_with_params>);
static_assert(StatefulNeuronWithParams<stateful_neuron_with_params, any_t>);
static_assert(StatefulNeuronWithParams<stateful_neuron_with_params, int>);
static_assert(!StatelessNeuron<stateful_neuron_with_params>);
static_assert(StatefulNeuron<stateful_neuron_with_params>);
static_assert(!NeuronWithoutParams<stateful_neuron_with_params>);
static_assert(NeuronWithParams<stateful_neuron_with_params, any_t>);
static_assert(NeuronWithParams<stateful_neuron_with_params, int>);
static_assert(Neuron<stateful_neuron_with_params>);