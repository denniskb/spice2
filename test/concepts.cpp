#include <gtest/gtest.h>

#include "spice/concepts.h"
#include "spice/util/type_traits.h"

using namespace spice;
using namespace spice::util;

// Dummy neurons

struct stateless_neuron {
	bool update(float, auto&) const { return false; }
};
struct stateless_neuron_no_const {
	bool update(float, auto&) { return false; }
};

struct stateful_neuron {
	struct neuron {
		int i;
	};

	bool update(neuron&, float, auto&) const { return false; }
};
struct stateful_neuron_no_const {
	struct neuron {
		int i;
	};

	bool update(neuron&, float, auto&) { return false; }
};

struct per_neuron_init : public stateful_neuron {
	void init(neuron&, Int, auto&) const {}
};
struct per_neuron_init_no_const : public stateful_neuron {
	void init(neuron&, Int, auto&) {}
};

struct per_population_init : public stateful_neuron {
	void init(std::span<neuron>, auto&) {}
};
struct per_population_init_const : public stateful_neuron {
	void init(std::span<neuron>, auto&) const {}
};

struct per_population_update {
	void update(float, auto&, std::vector<Int32>&) {}
};
struct per_population_update_const {
	void update(float, auto&, std::vector<Int32>&) const {}
};

struct both_stateless_and_stateful : public stateless_neuron, public stateful_neuron {};
struct both_inits : public per_neuron_init, public per_population_init {};
struct update_and_stateless : public per_population_update, public stateless_neuron {};
struct update_and_stateful : public per_population_update, public stateful_neuron {};
struct update_and_per_neuron_init : public per_population_update, public per_neuron_init {};
struct update_and_per_population_init : public per_population_update, public per_population_init {};

// Dummy synapses

struct stateless_synapse {
	void deliver(stateful_neuron::neuron&) const {}
};

struct stateless_synapse_no_const {
	void deliver(stateful_neuron::neuron&) {}
};

struct stateless_synapse_desc {
	void deliver(stateful_neuron&) const {}
};

struct stateful_synapse {
	struct synapse {
		float w;
	};

	void deliver(synapse const&, stateful_neuron::neuron&) const {}
};

struct stateful_synapse_no_const {
	struct synapse {
		float w;
	};

	void deliver(synapse const&, stateful_neuron::neuron&) {}
};

struct stateful_synapse_no_const2 {
	struct synapse {
		float w;
	};

	void deliver(synapse&, stateful_neuron::neuron&) const {}
};

struct plastic_synapse : public stateful_synapse {
	void update(synapse&, float, bool, bool) const {}
	void skip(synapse&, float, Int) const {}
};

struct plastic_synapse_no_const : public stateful_synapse {
	void update(synapse&, float, bool, bool) {}
	void skip(synapse&, float, Int) const {}
};

struct plastic_synapse_no_const2 : public stateful_synapse {
	void update(synapse&, float, bool, bool) const {}
	void skip(synapse&, float, Int) {}
};

struct per_synapse_init : public stateful_synapse {
	void init(synapse&, Int, Int, auto&) const {}
};

struct per_synapse_init_no_const : public stateful_synapse {
	void init(synapse&, Int, Int, auto&) {}
};

struct both_stateless_and_stateful_syn : public stateless_synapse, public stateful_synapse {};

TEST(Concepts, StatelessNeuron) {
	static_assert(all_of<StatelessNeuron<stateless_neuron>, Neuron<stateless_neuron>>);
	static_assert(none_of<StatefulNeuron<stateless_neuron>, PerNeuronInit<stateless_neuron>,
	                      PerPopulationInit<stateless_neuron>, PerPopulationUpdate<stateless_neuron>>);
	static_assert(none_of<StatelessNeuron<stateless_neuron_no_const>, Neuron<stateless_neuron_no_const>>);
}

TEST(Concepts, StatefulNeuron) {
	static_assert(all_of<StatefulNeuron<stateful_neuron>, Neuron<stateful_neuron>>);
	static_assert(none_of<StatelessNeuron<stateful_neuron>, PerNeuronInit<stateful_neuron>,
	                      PerPopulationInit<stateful_neuron>, PerPopulationUpdate<stateful_neuron>>);
	static_assert(none_of<StatefulNeuron<stateful_neuron_no_const>, Neuron<stateful_neuron_no_const>>);
}

TEST(Concepts, PerNeuronInit) {
	static_assert(
	    all_of<PerNeuronInit<per_neuron_init>, StatefulNeuron<per_neuron_init>, Neuron<per_neuron_init>>);
	static_assert(none_of<StatelessNeuron<per_neuron_init>, PerPopulationInit<per_neuron_init>,
	                      PerPopulationUpdate<per_neuron_init>>);
	static_assert(!PerNeuronInit<per_neuron_init_no_const>);
}

TEST(Concepts, PerPopulationInit) {
	static_assert(all_of<PerPopulationInit<per_population_init>, StatefulNeuron<per_population_init>,
	                     Neuron<per_population_init>>);
	static_assert(all_of<PerPopulationInit<per_population_init_const>,
	                     StatefulNeuron<per_population_init_const>, Neuron<per_population_init_const>>);
	static_assert(none_of<StatelessNeuron<per_population_init>, PerNeuronInit<per_population_init>,
	                      PerPopulationUpdate<per_population_init>>);
}

TEST(Concepts, PerPopulationUpdate) {
	static_assert(PerPopulationUpdate<per_population_update>);
	static_assert(PerPopulationUpdate<per_population_update_const>);
	static_assert(none_of<StatelessNeuron<per_population_update>, StatefulNeuron<per_population_update>,
	                      PerNeuronInit<per_population_update>, PerPopulationInit<per_population_update>>);
}

TEST(Concepts, InvalidNeurons) {
	static_assert(none_of<Neuron<both_stateless_and_stateful>, Neuron<both_inits>,
	                      Neuron<update_and_stateless>, Neuron<update_and_stateful>,
	                      Neuron<update_and_per_neuron_init>, Neuron<update_and_per_population_init>>);
}

TEST(Concepts, StatelessSynapse) {
	static_assert(all_of<StatelessSynapse<stateless_synapse, stateful_neuron>,
	                     Synapse<stateless_synapse, stateful_neuron>>);
	static_assert(none_of<StatelessSynapse<stateless_synapse_no_const, stateful_neuron>,
	                      StatelessSynapse<stateless_synapse_desc, stateful_neuron>,
	                      Synapse<stateless_synapse_no_const, stateful_neuron>,
	                      Synapse<stateless_synapse_desc, stateful_neuron>>);
	static_assert(!StatefulSynapse<stateless_synapse, stateful_neuron>);
}

TEST(Concepts, StatefulSynapse) {
	static_assert(all_of<StatefulSynapse<stateful_synapse, stateful_neuron>,
	                     Synapse<stateful_synapse, stateful_neuron>>);
	static_assert(none_of<StatefulSynapse<stateful_synapse_no_const, stateful_neuron>,
	                      Synapse<stateful_synapse_no_const, stateful_neuron>,
	                      StatefulSynapse<stateful_synapse_no_const2, stateful_neuron>,
	                      Synapse<stateful_synapse_no_const2, stateful_neuron>>);
	static_assert(!StatelessSynapse<stateful_synapse, stateful_neuron>);
}

TEST(Concepts, PlasticSynapse) {
	static_assert(
	    all_of<PlasticSynapse<plastic_synapse, stateful_neuron>,
	           StatefulSynapse<plastic_synapse, stateful_neuron>, Synapse<plastic_synapse, stateful_neuron>>);
	static_assert(none_of<PlasticSynapse<plastic_synapse_no_const, stateful_neuron>,
	                      PlasticSynapse<plastic_synapse_no_const2, stateful_neuron>,
	                      StatelessSynapse<plastic_synapse_no_const, stateful_neuron>,
	                      StatelessSynapse<plastic_synapse_no_const2, stateful_neuron>>);
	static_assert(all_of<StatefulSynapse<plastic_synapse_no_const, stateful_neuron>,
	                     Synapse<plastic_synapse_no_const, stateful_neuron>,
	                     StatefulSynapse<plastic_synapse_no_const2, stateful_neuron>,
	                     Synapse<plastic_synapse_no_const2, stateful_neuron>>);
	static_assert(none_of<PlasticSynapse<stateless_synapse, stateful_neuron>,
	                      PlasticSynapse<stateful_synapse, stateful_neuron>>);
}

TEST(Concepts, PerSynapseInit) {
	static_assert(all_of<PerSynapseInit<per_synapse_init, stateful_neuron>,
	                     StatefulSynapse<per_synapse_init, stateful_neuron>,
	                     Synapse<per_synapse_init, stateful_neuron>>);
	static_assert(!PerSynapseInit<per_synapse_init_no_const, stateful_neuron>);
	static_assert(all_of<StatefulSynapse<per_synapse_init_no_const, stateful_neuron>,
	                     Synapse<per_synapse_init_no_const, stateful_neuron>>);
}

TEST(Concepts, InvalidSynapses) { static_assert(!Synapse<both_stateless_and_stateful_syn, stateful_neuron>); }