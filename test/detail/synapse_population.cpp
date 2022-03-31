#include "gtest/gtest.h"

#include <concepts>

#include "spice/detail/synapse_population.h"

using namespace spice;
using namespace spice::detail;
using namespace spice::util;

struct stateful_neuron {
	int v = 0;
	bool update(float, snn_info&) { return false; }
};

struct stateless_synapse {
	static void deliver(stateful_neuron&, snn_info&) {}
};
struct stateful_synapse {
	void deliver(stateful_neuron&, snn_info&) {}
};
struct stateless_synapse_with_params {
	static void deliver(stateful_neuron&, snn_info&, int) {}
};
struct stateful_synapse_with_params {
	void deliver(stateful_neuron&, snn_info&, int) {}
};

struct plastic_synapse : stateful_synapse {
	void update(float, bool, bool, snn_info&) {}
	void skip(float, Int, snn_info&) {}
};
struct plastic_synapse_with_params : stateful_synapse_with_params {
	void update(float, bool, bool, snn_info&, int) {}
	void skip(float, Int, snn_info&, int) {}
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

static_assert(!PlasticSynapse<stateless_synapse>);
static_assert(!PlasticSynapse<stateful_synapse>);
static_assert(!PlasticSynapse<stateless_synapse_with_params>);
static_assert(!PlasticSynapse<stateful_synapse_with_params>);
static_assert(PlasticSynapse<plastic_synapse>);
static_assert(PlasticSynapse<plastic_synapse_with_params>);
static_assert(!PlasticSynapse<int>);