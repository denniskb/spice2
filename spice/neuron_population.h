#pragma once

#include <functional>
#include <numeric>
#include <span>
#include <utility>
#include <variant>
#include <vector>

#include "concepts.h"
#include "util/assert.h"
#include "util/range.h"
#include "util/stdint.h"
#include "util/type_traits.h"

namespace spice {
template <Neuron Neur>
class neuron_population {
public:
	neuron_population(Int const size, Int const max_delay) {
		SPICE_ASSERT(size >= 0);
		SPICE_ASSERT(max_delay >= 1);

		_history.resize(size);

		_spike_counts.reserve(max_delay);
		_spikes.reserve(size * max_delay / 100);

		if (StatefulNeuron<Neur>)
			_neurons.resize(size);
	}

	Int size() const { return _history.size(); }

	void update(Int const max_delay, double const dt) {
		SPICE_ASSERT(max_delay >= 1);

		if (_spike_counts.size() == max_delay) {
			_spikes.erase(_spikes.begin(), _spikes.begin() + _spike_counts.front());
			_spike_counts.erase(_spike_counts.begin());
		}

		Int const spike_count = _spikes.size();
		for (Int const i : util::range(_history)) {
			bool spiked;
			if constexpr (StatelessNeuron<Neur>)
				spiked = Neur::update(dt);
			else
				spiked = _neurons[i].update(dt);

			_history[i] = (_history[i] << 1) | spiked;
			if (spiked)
				_spikes.push_back(i);
		}
		_spike_counts.push_back(_spikes.size() - spike_count);
	}

	std::span<util::nonvoid_or_empty_t<Neur>> neurons() {
		static_assert(StatefulNeuron<Neur>, "Can't return collection of stateless neurons.");
		return _neurons;
	}

	std::span<Int32 const> spikes(Int age) const {
		SPICE_ASSERT(age < _spike_counts.size());

		Int const offset = std::accumulate(_spike_counts.end() - 1 - age, _spike_counts.end(), 0);
		return {_spikes.data() + _spikes.size() - offset, static_cast<UInt>(_spike_counts.rbegin()[age])};
	}

	std::span<UInt const> history() const { return _history; }

private:
	std::vector<Neur> _neurons;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
};
}