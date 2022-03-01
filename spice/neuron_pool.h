#pragma once

#include <numeric>
#include <span>
#include <vector>

#include "util/assert.h"
#include "util/range.h"
#include "util/stdint.h"

namespace spice {
template <class Neuron>
class neuron_pool {
public:
	neuron_pool(Int const size, Int const max_delay) : _neurons(size), _history(size) {
		SPICE_ASSERT(size >= 0);
		SPICE_ASSERT(max_delay >= 1);

		_spike_counts.reserve(max_delay);
		_spikes.reserve(size * max_delay / 100);
	}

	void update(Int const max_delay) {
		SPICE_ASSERT(max_delay >= 1);

		if (_spike_counts.size() == max_delay) {
			_spikes.erase(_spikes.begin(), _spikes.begin() + _spike_counts.front());
			_spike_counts.erase(_spike_counts.begin());
		}

		Int const spike_count = _spikes.size();
		for (Int const i : util::range(_neurons)) {
			bool const spiked = _neurons[i].update();
			_history[i]       = (_history[i] << 1) | spiked;
			if (spiked)
				_spikes.push_back(i);
		}
		_spike_counts.push_back(_spikes.size() - spike_count);
	}

	std::span<Int32 const> spikes(Int age) const {
		SPICE_ASSERT(age < _spike_counts.size());

		Int const offset = std::accumulate(_spike_counts.end() - 1 - age, _spike_counts.end(), 0);
		return {_spikes.data() + _spikes.size() - offset,
		        static_cast<UInt>(_spike_counts.rbegin()[age])};
	}

	std::span<UInt const> history() const { return _history; }

private:
	std::vector<Neuron> _neurons;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
};
}