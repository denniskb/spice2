#pragma once

#include "util/random.h"
#include "util/stdint.h"

namespace spice {
struct sim_info {
	Int N;
	util::xoroshiro64_128p rng;
};
}