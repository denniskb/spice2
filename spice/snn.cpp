#include "spice/snn.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);

	if (_iter >= _delay) {
		for (auto& c : _connections)
			c.synapse->deliver(c.from->spikes(_delay - 1), c.to->neurons(), c.to->size());
	}

	for (auto& pop : _neurons)
		pop->update(_delay, _dt, rng);

	_iter++;
}