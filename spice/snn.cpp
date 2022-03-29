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

	if (_time % 64 == 0)
		for (auto& c : _connections)
			c.synapse->update(_time, _dt, c.from->size(), c.to->history());

	if (_time >= _max_delay - 1)
		for (auto& c : _connections)
			c.synapse->deliver(_time, _dt, c.from->spikes(_max_delay - 1), c.to->neurons(), c.to->size(),
			                   c.to->history());

	_time++;
}