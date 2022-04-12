#include "gtest/gtest.h"

#include "spice/detail/synapse_population.h"

using namespace spice;
using namespace spice::detail;
using namespace spice::util;

struct stateful_neuron {
	struct neuron {
		int received_count = 0;
	};

	bool update(neuron& n, float, auto&) const { return false; }
};
static_assert(StatefulNeuron<stateful_neuron>);

struct stateless_synapse {
	void deliver(stateful_neuron::neuron& n) const { n.received_count++; }
};
static_assert(StatelessSynapse<stateless_synapse, stateful_neuron>);

template <class Syn>
requires Synapse<Syn, stateful_neuron> synapse_population<Syn, stateful_neuron> setup() {
	seed_seq seed({1337});

	adj_list adj;
	adj.src_count = 3;
	adj.dst_count = 5;
	adj.connect(0, 0);
	adj.connect(0, 1);
	adj.connect(0, 3);
	adj.connect(1, 3);
	adj.connect(2, 4);

	return synapse_population<Syn, stateful_neuron>({}, adj, seed, 1);
}

TEST(SynapsePopulation, DeliverStateless) {
	stateful_neuron::neuron neurons[5];
	Int32 spikes[] = {0, 1};
	auto syn       = setup<stateless_synapse>();

	syn.deliver(0, 1, spikes, neurons, 10, {});

	ASSERT_EQ(neurons[0].received_count, 1);
	ASSERT_EQ(neurons[1].received_count, 1);
	ASSERT_EQ(neurons[2].received_count, 0);
	ASSERT_EQ(neurons[3].received_count, 2);
	ASSERT_EQ(neurons[4].received_count, 0);
}

struct stateful_synapse {
	struct synapse {
		int w = 2;
	};
	void deliver(synapse const& syn, stateful_neuron::neuron& n) const { n.received_count += syn.w; }
};
static_assert(StatefulSynapse<stateful_synapse, stateful_neuron>);

TEST(SynapsePopulation, DeliverStateful) {
	stateful_neuron::neuron neurons[5];
	Int32 spikes[] = {0, 1};
	auto syn       = setup<stateful_synapse>();

	syn.deliver(0, 1, spikes, neurons, 10, {});

	ASSERT_EQ(neurons[0].received_count, 2);
	ASSERT_EQ(neurons[1].received_count, 2);
	ASSERT_EQ(neurons[2].received_count, 0);
	ASSERT_EQ(neurons[3].received_count, 4);
	ASSERT_EQ(neurons[4].received_count, 0);
}

struct plastic_synapse {
	struct synapse {
		int update_count = 0;
	};

	void deliver(synapse const& syn, stateful_neuron::neuron& n) const {
		n.received_count = syn.update_count;
	}
	void update(synapse& syn, float, bool, bool) const { syn.update_count++; }
	void skip(synapse& syn, float, Int steps) const { syn.update_count += steps; }
};
static_assert(PlasticSynapse<plastic_synapse, stateful_neuron>);

TEST(SynapsePopulation, DeliverPlastic) {
	{ // Deliver
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.deliver(0, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 1);
		ASSERT_EQ(neurons[4].received_count, 1);
	}
	{ // Update followed by deliver at the same time step, only 1 update should be performed
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.update(0, 1, 3, hist);
		syn.deliver(0, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 1);
		ASSERT_EQ(neurons[4].received_count, 1);
	}
	{ // Multiple updates/deliver in the same time step, only 1 update should be performed
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.update(0, 1, 3, hist);
		syn.update(0, 1, 3, hist);
		syn.deliver(0, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 1);
		ASSERT_EQ(neurons[4].received_count, 1);
	}
	{ // Update/deliver different time steps, 2 updates should be performed
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.update(0, 1, 3, hist);
		syn.deliver(1, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 2);
		ASSERT_EQ(neurons[4].received_count, 2);
	}
	{ // Skip ahead to time step=10, 10 updates should be performed.
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.deliver(9, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 10);
		ASSERT_EQ(neurons[4].received_count, 10);
	}
	{ // Skip ahead to time step=10, 10 updates should be performed.
		stateful_neuron::neuron neurons[5];
		UInt hist[5]   = {0};
		Int32 spikes[] = {1, 2};
		auto syn       = setup<plastic_synapse>();

		syn.update(4, 1, 3, hist);
		syn.deliver(9, 1, spikes, neurons, 10, hist);

		ASSERT_EQ(neurons[3].received_count, 10);
		ASSERT_EQ(neurons[4].received_count, 10);
	}
}