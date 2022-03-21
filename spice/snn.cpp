#include "spice/snn.h"

#include "spice/util/random.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);
	float const dt = _simtime += _dt;
	if (_simtime >= 1)
		_simtime.reset();

	for (auto& pop : _neurons)
		pop->update(_max_delay, dt, rng);

	for (auto& c : _connections)
		c.synapse->update(_max_delay, _dt, c.from->history(), c.to->history());

	if (_iter >= _max_delay - 1)
		for (auto& c : _connections)
			c.synapse->deliver(c.from->spikes(_max_delay - 1), c.to->neurons(), c.to->size());

	_iter++;
}