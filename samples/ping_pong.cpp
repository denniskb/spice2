#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

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

	adj_list adj;
	for (Int i : range(N / 2)) {
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