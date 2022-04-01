#pragma once

#include "util/random.h"
#include "util/stdint.h"

namespace spice {
struct sim_info {
	util::xoroshiro64_128p rng{{1337}};
	union {
		Int neuron_id = 0;
		Int dst_neuron_id;
	};
	Int src_neuron_id   = 0;
	Int network_size    = -1;
	Int population_size = -1;
};
}