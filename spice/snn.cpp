#include "spice/snn.h"

#include "spice/util/random.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);
	float const dt = _simtime += _dt;
	if (_simtime >= 1)
		_simtime = {};

	for (auto& pop : _neurons)
		pop->update(_delay, dt, rng);

	for (auto& c : _connections)
		c.synapse->update(_iter, dt, c.to->history(), c.from->history());

	if (_iter >= _delay - 1)
		for (auto& c : _connections)
			c.synapse->deliver(_iter, dt, c.from->spikes(_delay - 1), c.to->neurons(), c.to->size(),
			                   c.from->history());

	_iter++;
}