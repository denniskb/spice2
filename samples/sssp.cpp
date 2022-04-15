#include "spice/snn.h"

using namespace spice;

//      3 2 3
//   1.---*---.3
//  1/         \1
// 0*           *4
//  1\         /1
//    *-------*
//    5   5   6

// clang-format off
static Int adj_matrix[7][7] = {
	{0, 1, 0, 0, 0, 1, 0},
	{0, 0, 3, 0, 0, 0, 0},
	{0, 0, 0, 3, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 5},
	{0, 0, 0, 0, 1, 0, 0}
};
// clang-format on

struct vertex {
	Int src;

	struct neuron {
		Int distance = std::numeric_limits<Int>::max();
		Int previous = -1;
		bool fire    = false;
	};

	void init(std::span<neuron> neurons, auto&) {
		neurons[src].distance = 0;
		neurons[src].previous = src;
		neurons[src].fire     = true;
	}

	bool update(neuron& n, float, auto&) const {
		bool const result = n.fire;
		n.fire            = false;
		return result;
	}
};
static_assert(PerPopulationInit<vertex>);

struct edge {
	struct synapse {
		Int weight;
	};

	void init(synapse& syn, Int src, Int dst, auto&) const { syn.weight = adj_matrix[src][dst]; }

	void deliver(synapse const&, vertex::neuron const&, vertex::neuron&) const {
		// TODO: Make source neuron available inside deliver!
	}
};
static_assert(StatefulSynapse<edge> && DeliverFromTo<edge, vertex, vertex>);

int main() { return 0; }