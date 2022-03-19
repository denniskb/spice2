#include "spice/snn.h"

#include "spice/util/random.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);
	float const dt = _simtime += _dt;
	if (_simtime >= 1)
		_simtime = {};

	if (_iter && _iter % 64 == 0)
		for (auto& c : _connections)
			c.synapse->update(_iter, dt, c.to->history());

	if (_iter >= _delay)
		for (auto& c : _connections)
			c.synapse->deliver(_iter, dt, c.from->spikes(_delay - 1), c.to->neurons(), c.to->size(),
			                   c.from->history());

	for (auto& pop : _neurons)
		pop->update(_delay, dt, rng);

	_iter++;
}