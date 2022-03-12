#pragma once

#include <functional>
#include <numeric>
#include <span>
#include <utility>
#include <variant>
#include <vector>

#include "spice/concepts.h"
#include "spice/util/assert.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
struct NeuronPopulation {
	virtual ~NeuronPopulation()                                           = default;
	virtual Int size() const                                              = 0;
	virtual void update(Int delay, float dt, util::xoroshiro64_128p& rng) = 0;
	virtual std::span<Int32 const> spikes(Int age) const                  = 0;
	virtual std::span<UInt const> history() const                         = 0;
	virtual void* neurons()                                               = 0;
};

template <Neuron Neur, class Params = void>
requires(std::is_void_v<Params> ? NeuronWithoutParams<Neur> :
                                  NeuronWithParams<Neur, Params>) class neuron_population :
public NeuronPopulation {
public:
	neuron_population(Int const size, Int const delay, util::nonvoid_or_empty_t<Params> params = {}) :
	_history(size), _params(std::move(params)) {
		SPICE_PRE(size >= 0);
		SPICE_PRE(delay >= 1);

		_spike_counts.reserve(delay);
		_spikes.reserve(size * delay / 100);

		if constexpr (StatefulNeuron<Neur>)
			_neurons.resize(size);
	}

	Int size() const override { return _history.size(); }

	void update(Int const delay, float const dt, util::xoroshiro64_128p& rng) override {
		SPICE_PRE(delay >= 1);

		if (_spike_counts.size() == delay) {
			_spikes.erase(_spikes.begin(), _spikes.begin() + _spike_counts.front());
			_spike_counts.erase(_spike_counts.begin());
		}

		Int const spike_count = _spikes.size();
		for (Int const i : util::range(_history)) {
			bool spiked;
			if constexpr (StatelessNeuronWithoutParams<Neur>)
				spiked = Neur::update(dt, rng);
			else if constexpr (StatelessNeuronWithParams<Neur, Params>)
				spiked = Neur::update(dt, rng, _params);
			else if constexpr (StatefulNeuronWithoutParams<Neur>)
				spiked = _neurons[i].update(dt, rng);
			else
				spiked = _neurons[i].update(dt, rng, _params);

			_history[i] = (_history[i] << 1) | spiked;
			if (spiked)
				_spikes.push_back(i);
		}
		_spike_counts.push_back(_spikes.size() - spike_count);
	}

	void* neurons() override { return _neurons.data(); }
	std::span<Neur> get_neurons() {
		static_assert(StatefulNeuron<Neur>, "Can't return collection of stateless neurons.");
		return _neurons;
	}

	std::span<Int32 const> spikes(Int age) const override {
		SPICE_PRE(age < _spike_counts.size());

		Int const offset = std::accumulate(_spike_counts.end() - 1 - age, _spike_counts.end(), 0);
		return {_spikes.data() + _spikes.size() - offset, static_cast<UInt>(_spike_counts.rbegin()[age])};
	}

	std::span<UInt const> history() const override { return _history; }

private:
	std::vector<Neur> _neurons;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
	util::nonvoid_or_empty_t<Params> _params;
};
}