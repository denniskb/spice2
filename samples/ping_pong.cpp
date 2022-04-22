#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

/*
ping-pong is a simple SNN consisting of two neuron popualtions that alternate
exciting each other. It illustrates arbitrary topologies.
*/

struct neuron_desc {
	bool initial_spike;

	struct neuron {
		bool should_i_spike;
	};

	void init(neuron& n, Int, auto&) const { n.should_i_spike = initial_spike; }

	bool update(neuron& n, float, auto&) const {
		bool const result = n.should_i_spike;
		n.should_i_spike  = false;
		return result;
	}
};
static_assert(CheckNeuron<neuron_desc>());

struct synapse_desc {
	void deliver(neuron_desc::neuron& n) const { n.should_i_spike = true; }
};
static_assert(CheckSynapse<synapse_desc>());

int main() {
	Int const N = 10'000;

	snn ping_pong(1, 1, {1337});
	auto ping = ping_pong.add_population<neuron_desc>(N / 2, {true});
	auto pong = ping_pong.add_population<neuron_desc>(N / 2, {false});

	// Spice ships with a number of parametric topologies such as
	// 'fixed_probability', 'fixed_indegree'[TODO], 'fixed_connection_count'[TODO], etc.
	// Sometimes these are not enough. For that purpose, the user can create an 'adj_list'
	// and define an arbitrary topology by calling connect(int source, int target) on it.
	adj_list adj;
	for (Int i : range(N / 2)) {
		// In this case we create a simple 1:1 connection between the neurons in 'ping' and 'pong'.
		adj.connect(i, i);
	}
	ping_pong.connect<synapse_desc>(ping, pong, adj, 1);
	ping_pong.connect<synapse_desc>(pong, ping, adj, 1);

	spike_output_stream s("ping-pong");
	for (Int i : range(10)) {
		ping_pong.step();

		s << ping << pong << '\n';

		(void)i;
	}
}