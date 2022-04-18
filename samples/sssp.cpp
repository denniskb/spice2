#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

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
		Int distance           = std::numeric_limits<Int>::max();
		neuron const* previous = nullptr;
		bool fire              = false;
	};

	void init(std::span<neuron> neurons, auto&) {
		neurons[src].distance = 0;
		neurons[src].previous = &neurons[src];
		neurons[src].fire     = true;
	}

	bool update(neuron& n, float, auto&) const {
		bool const result = n.fire;
		n.fire            = false;
		return result;
	}
};
static_assert(CheckNeuron<vertex>());

struct edge {
	struct synapse {
		Int weight;
	};

	void init(synapse& syn, Int src, Int dst, auto&) const { syn.weight = adj_matrix[src][dst]; }

	void deliver(synapse const& syn, vertex::neuron const& src, vertex::neuron& dst) const {
		if (src.distance + syn.weight < dst.distance) {
			dst.distance = src.distance + syn.weight;
			dst.previous = &src;
			dst.fire     = true;
		}
	}
};
//static_assert(CheckNeuron<vertex>());

int main() {
	snn sssp(1, 1, {1337});
	auto vertices = sssp.add_population<vertex>(7, {0});

	adj_list adj;
	for (Int src : range(7))
		for (Int dst : range(7))
			if (adj_matrix[src][dst])
				adj.connect(src, dst);

	sssp.connect<edge>(vertices, vertices, adj, 1);

	for (Int i : range(vertices->size() - 1)) {
		sssp.step();
		(void)i;
	}

	SPICE_ASSERT(vertices->get_neurons()[0].distance == 0);
	SPICE_ASSERT(vertices->get_neurons()[4].distance == 7);

	return 0;
}