#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

/*
This sample implements the single-source-shortest-pathS (plural, SSSP) algorithm on top of Spice.
It illustrates Spice's generality and the simplicity of parallel programming when paired with
local decision-making.
It also illustrates per-population initialization of neurons.
*/

// For simplicity, we hard-code our graph as an adjacency matrix.
// A more realistic scenario would be to read a graph description (such as GraphML) from file,
// which is just as possible and just as easy to do.

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

	// This flavor of the init() method takes a span of all neurons inside the 'vertex'
	// population. This is much more convenient than performing per-neuron initialization
	// when the initial state is coming from a central location (such as a file) to begin with.
	// Consider the alternative: Each of the (hundreds of thousands of) neurons would have to
	// open the same file and seek to the neuron's offset inside the population. Not only would
	// this be slow, it would also be combersome to write.
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

// In contrast to Brunel+, 'edge' is a stateful but NON-plastic synapse.
struct edge {
	struct synapse {
		Int weight;
	};

	void init(synapse& syn, Int src, Int dst, auto&) const { syn.weight = adj_matrix[src][dst]; }

	// SSSP's deliver() method is different from the deliver() method of all other models in that
	// it accepts and additional 'src' parameter, giving it access to the firing neuron's state.
	// Once again, the user simply defines a different 'flavor' (a different signature) of deliver()
	// and Spice adapts the simulation accordingly (aka design by introspection).
	void deliver(synapse const& syn, vertex::neuron const& src, vertex::neuron& dst) const {
		if (src.distance + syn.weight < dst.distance) {
			dst.distance = src.distance + syn.weight;
			dst.previous = &src;
			dst.fire     = true;
		}
	}
};
static_assert(CheckSynapse<edge>());

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