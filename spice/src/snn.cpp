#include "spice/snn.h"

#include "spice/sim_info.h"
#include "spice/util/random.h"

using namespace spice;

void snn::step() {
	util::xoroshiro64_128p rng(_seed++);
	float const dt = _simtime += _dt;
	if (_simtime >= 1)
		_simtime.reset();

	sim_info info{std::accumulate(_neurons.begin(), _neurons.end(), 0,
	                              [](auto i, auto const& n) { return i + n->size(); }),
	              util::xoroshiro64_128p(_seed++)};

	for (auto& pop : _neurons)
		pop->update(_max_delay, dt, info);

	if (_time % 64 == 0)
		for (auto& c : _connections)
			c.synapse->update(_time, _dt, info, c.from->size(), c.to->history());

	for (auto& c : _connections)
		if (_time >= c.synapse->delay() - 1)
			c.synapse->deliver(_time, _dt, info, c.from->spikes(c.synapse->delay() - 1), c.to->neurons(),
			                   c.to->size(), c.to->history());

	_time++;
}