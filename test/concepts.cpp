#include <gtest/gtest.h>

#include "spice/concepts.h"
#include "spice/util/type_traits.h"

using namespace spice;
using namespace spice::util;

struct stateless_neuron {
	bool update(float, auto&) const { return false; }
};
struct stateless_neuron_no_const {
	bool update(float, auto&) { return false; }
};

TEST(Concepts, StatelessNeuron) {
	static_assert(all_of<StatelessNeuron<stateless_neuron>, Neuron<stateless_neuron>>);
	static_assert(none_of<StatefulNeuron<stateless_neuron>, PerNeuronInit<stateless_neuron>,
	                      PerPopulationInit<stateless_neuron>, PerPopulationUpdate<stateless_neuron>>);
	static_assert(none_of<StatelessNeuron<stateless_neuron_no_const>, Neuron<stateless_neuron_no_const>>);
}

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

TEST(Concepts, StatefulNeuron) {
	static_assert(all_of<StatefulNeuron<stateful_neuron>, Neuron<stateful_neuron>>);
	static_assert(none_of<StatelessNeuron<stateful_neuron>, PerNeuronInit<stateful_neuron>,
	                      PerPopulationInit<stateful_neuron>, PerPopulationUpdate<stateful_neuron>>);
	static_assert(none_of<StatefulNeuron<stateful_neuron_no_const>, Neuron<stateful_neuron_no_const>>);
}

struct per_neuron_init : public stateful_neuron {
	void init(neuron&, Int, auto&) const {}
};
struct per_neuron_init_no_const : public stateful_neuron {
	void init(neuron&, Int, auto&) {}
};

TEST(Concepts, PerNeuronInit) {
	static_assert(
	    all_of<PerNeuronInit<per_neuron_init>, StatefulNeuron<per_neuron_init>, Neuron<per_neuron_init>>);
	static_assert(none_of<StatelessNeuron<per_neuron_init>, PerPopulationInit<per_neuron_init>,
	                      PerPopulationUpdate<per_neuron_init>>);
	static_assert(!PerNeuronInit<per_neuron_init_no_const>);
}

struct per_population_init : public stateful_neuron {
	void init(std::span<neuron>, auto&) {}
};
struct per_population_init_const : public stateful_neuron {
	void init(std::span<neuron>, auto&) const {}
};

TEST(Concepts, PerPopulationInit) {
	static_assert(all_of<PerPopulationInit<per_population_init>, StatefulNeuron<per_population_init>,
	                     Neuron<per_population_init>>);
	static_assert(all_of<PerPopulationInit<per_population_init_const>,
	                     StatefulNeuron<per_population_init_const>, Neuron<per_population_init_const>>);
	static_assert(none_of<StatelessNeuron<per_population_init>, PerNeuronInit<per_population_init>,
	                      PerPopulationUpdate<per_population_init>>);
}

struct per_population_update {
	void update(float, auto&, std::vector<Int32>&) {}
	Int size() const { return 0; }
};
struct per_population_update_const {
	void update(float, auto&, std::vector<Int32>&) const {}
	Int size() const { return 0; }
};
struct per_population_update_no_size {
	void update(float, auto&, std::vector<Int32>&) {}
};

TEST(Concepts, PerPopulationUpdate) {
	static_assert(PerPopulationUpdate<per_population_update>);
	static_assert(PerPopulationUpdate<per_population_update_const>);
	static_assert(!PerPopulationUpdate<per_population_update_no_size>);
	static_assert(none_of<StatelessNeuron<per_population_update>, StatefulNeuron<per_population_update>,
	                      PerNeuronInit<per_population_update>, PerPopulationInit<per_population_update>>);
}

struct both_stateless_and_stateful : public stateless_neuron, public stateful_neuron {};
struct both_inits : public per_neuron_init, public per_population_init {};
struct update_and_stateless : public per_population_update, public stateless_neuron {};
struct update_and_stateful : public per_population_update, public stateful_neuron {};
struct update_and_per_neuron_init : public per_population_update, public per_neuron_init {};
struct update_and_per_population_init : public per_population_update, public per_population_init {};

TEST(Concepts, InvalidNeurons) {
	static_assert(none_of<Neuron<both_stateless_and_stateful>, Neuron<both_inits>,
	                      Neuron<update_and_stateless>, Neuron<update_and_stateful>,
	                      Neuron<update_and_per_neuron_init>, Neuron<update_and_per_population_init>>);
}