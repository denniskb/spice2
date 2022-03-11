#include "gtest/gtest.h"

#include <concepts>

#include "spice/detail/synapse_population.h"

using namespace spice;
using namespace spice::detail;
using namespace spice::util;

struct stateful_neuron {
	int v = 0;
	bool update(float, xoroshiro64_128p&) { return false; }
};

struct stateless_synapse {
	static void deliver(stateful_neuron&) {}
};
struct stateful_synapse {
	void deliver(stateful_neuron&) {}
};
struct stateless_synapse_with_params {
	static void deliver(stateful_neuron&, int) {}
};
struct stateful_synapse_with_params {
	void deliver(stateful_neuron&, int) {}
};

static_assert(!StatelessSynapseWithoutParams<int, stateful_neuron>);
static_assert(!StatelessSynapseWithParams<int, stateful_neuron, any_t>);
static_assert(!StatefulSynapseWithoutParams<int, stateful_neuron>);
static_assert(!StatefulSynapseWithParams<int, stateful_neuron, any_t>);
static_assert(!StatelessSynapse<int, stateful_neuron>);
static_assert(!StatefulSynapse<int, stateful_neuron>);
static_assert(!SynapseWithoutParams<int, stateful_neuron>);
static_assert(!SynapseWithParams<int, stateful_neuron, any_t>);
static_assert(!Synapse<int, stateful_neuron>);

static_assert(StatelessSynapseWithoutParams<stateless_synapse, stateful_neuron>);
static_assert(!StatelessSynapseWithParams<stateless_synapse, stateful_neuron, any_t>);
static_assert(!StatefulSynapseWithoutParams<stateless_synapse, stateful_neuron>);
static_assert(!StatefulSynapseWithParams<stateless_synapse, stateful_neuron, any_t>);
static_assert(StatelessSynapse<stateless_synapse, stateful_neuron>);
static_assert(!StatefulSynapse<stateless_synapse, stateful_neuron>);
static_assert(SynapseWithoutParams<stateless_synapse, stateful_neuron>);
static_assert(!SynapseWithParams<stateless_synapse, stateful_neuron, any_t>);
static_assert(Synapse<stateless_synapse, stateful_neuron>);

static_assert(!StatelessSynapseWithoutParams<stateful_synapse, stateful_neuron>);
static_assert(!StatelessSynapseWithParams<stateful_synapse, stateful_neuron, any_t>);
static_assert(StatefulSynapseWithoutParams<stateful_synapse, stateful_neuron>);
static_assert(!StatefulSynapseWithParams<stateful_synapse, stateful_neuron, any_t>);
static_assert(!StatelessSynapse<stateful_synapse, stateful_neuron>);
static_assert(StatefulSynapse<stateful_synapse, stateful_neuron>);
static_assert(SynapseWithoutParams<stateful_synapse, stateful_neuron>);
static_assert(!SynapseWithParams<stateful_synapse, stateful_neuron, any_t>);
static_assert(Synapse<stateful_synapse, stateful_neuron>);

static_assert(!StatelessSynapseWithoutParams<stateless_synapse_with_params, stateful_neuron>);
static_assert(StatelessSynapseWithParams<stateless_synapse_with_params, stateful_neuron, any_t>);
static_assert(StatelessSynapseWithParams<stateless_synapse_with_params, stateful_neuron, int>);
static_assert(!StatefulSynapseWithoutParams<stateless_synapse_with_params, stateful_neuron>);
static_assert(!StatefulSynapseWithParams<stateless_synapse_with_params, stateful_neuron, any_t>);
static_assert(StatelessSynapse<stateless_synapse_with_params, stateful_neuron>);
static_assert(!StatefulSynapse<stateless_synapse_with_params, stateful_neuron>);
static_assert(!SynapseWithoutParams<stateless_synapse_with_params, stateful_neuron>);
static_assert(SynapseWithParams<stateless_synapse_with_params, stateful_neuron, any_t>);
static_assert(SynapseWithParams<stateless_synapse_with_params, stateful_neuron, int>);
static_assert(Synapse<stateless_synapse_with_params, stateful_neuron>);

static_assert(!StatelessSynapseWithoutParams<stateful_synapse_with_params, stateful_neuron>);
static_assert(!StatelessSynapseWithParams<stateful_synapse_with_params, stateful_neuron, any_t>);
static_assert(!StatefulSynapseWithoutParams<stateful_synapse_with_params, stateful_neuron>);
static_assert(StatefulSynapseWithParams<stateful_synapse_with_params, stateful_neuron, any_t>);
static_assert(StatefulSynapseWithParams<stateful_synapse_with_params, stateful_neuron, int>);
static_assert(!StatelessSynapse<stateful_synapse_with_params, stateful_neuron>);
static_assert(StatefulSynapse<stateful_synapse_with_params, stateful_neuron>);
static_assert(!SynapseWithoutParams<stateful_synapse_with_params, stateful_neuron>);
static_assert(SynapseWithParams<stateful_synapse_with_params, stateful_neuron, any_t>);
static_assert(SynapseWithParams<stateful_synapse_with_params, stateful_neuron, int>);
static_assert(Synapse<stateful_synapse_with_params, stateful_neuron>);

static_assert(std::input_iterator<synapse_population<stateless_synapse, stateful_neuron>::iterator>);
static_assert(std::input_iterator<synapse_population<stateful_synapse, stateful_neuron>::const_iterator>);
static_assert(
    std::input_iterator<synapse_population<stateless_synapse_with_params, stateful_neuron, int>::iterator>);
static_assert(std::input_iterator<
              synapse_population<stateful_synapse_with_params, stateful_neuron, int>::const_iterator>);