#include "spice/snn.h"

#include "spice/util/random.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);

	if (_iter && _iter % 64 == 0)
		for (auto& c : _connections)
			c.synapse->update(_iter, _dt, c.to->history());

	if (_iter >= _delay)
		for (auto& c : _connections)
			c.synapse->deliver(_iter, _dt, c.from->spikes(_delay - 1), c.to->neurons(), c.to->size(),
			                   c.from->history());

	for (auto& pop : _neurons)
		pop->update(_delay, _dt, rng);

	_iter++;
}