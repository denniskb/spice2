#pragma once

#include <numeric>
#include <span>
#include <vector>

#include "spice/concepts.h"
#include "spice/snn_info.h"
#include "spice/util/assert.h"
#include "spice/util/meta.h"
#include "spice/util/random.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
struct NeuronPopulation {
	virtual ~NeuronPopulation()                              = default;
	virtual Int size() const                                 = 0;
	virtual void update(Int delay, float dt, snn_info& info) = 0;
	virtual void* neurons()                                  = 0;
	virtual std::span<Int32 const> spikes(Int age) const     = 0;
	virtual void plastic()                                   = 0;
	virtual std::span<UInt const> history() const            = 0;
};

template <Neuron Neur, class Params = util::empty_t>
requires(util::is_empty_v<Params> ? NeuronWithoutParams<Neur> :
                                    NeuronWithParams<Neur, Params>) class neuron_population :
public NeuronPopulation {
public:
	neuron_population(Int const size, Int const delay, Params const params = {}) :
	_size(size), _params(std::move(params)) {
		SPICE_INV(size >= 0);
		SPICE_INV(delay >= 1);

		_spike_counts.reserve(delay);
		_spikes.reserve(size * delay / 100);

		if constexpr (StatefulNeuron<Neur>)
			_neurons.resize(size);
	}

	Int size() const override { return _size; }

	void update(Int const delay, float const dt, snn_info& info) override {
		SPICE_INV(delay >= 1);

		if (_spike_counts.size() == delay) {
			_spikes.erase(_spikes.begin(), _spikes.begin() + _spike_counts.front());
			_spike_counts.erase(_spike_counts.begin());
		}

		Int const spike_count = _spikes.size();
		util::invoke(_plastic, [&]<bool Plastic>() {
			for (Int const i : util::range(size())) {
				bool spiked;
				if constexpr (StatelessNeuronWithoutParams<Neur>)
					spiked = Neur::update(dt, info);
				else if constexpr (StatelessNeuronWithParams<Neur, Params>)
					spiked = Neur::update(dt, info, _params);
				else if constexpr (StatefulNeuronWithoutParams<Neur>)
					spiked = _neurons[i].update(dt, info);
				else
					spiked = _neurons[i].update(dt, info, _params);

				if constexpr (Plastic)
					_history[i] = (_history[i] << 1) | spiked;

				if (spiked)
					_spikes.push_back(i);
			}
		});

		_spike_counts.push_back(_spikes.size() - spike_count);
	}

	void* neurons() override {
		SPICE_PRE(StatefulNeuron<Neur> && "Can't return collection of stateless neurons.");
		return _neurons.data();
	}
	std::span<Neur> get_neurons() {
		static_assert(StatefulNeuron<Neur>, "Can't return collection of stateless neurons.");
		return _neurons;
	}

	std::span<Int32 const> spikes(Int age) const override {
		SPICE_PRE(0 <= age && age < _spike_counts.size());

		Int const offset = std::accumulate(_spike_counts.end() - 1 - age, _spike_counts.end(), 0);
		return {_spikes.data() + _spikes.size() - offset, static_cast<UInt>(_spike_counts.rbegin()[age])};
	}

	void plastic() {
		_history.resize(size());
		_plastic = true;
	}

	std::span<UInt const> history() const override { return _history; }

private:
	Int _size = 0;
	std::vector<Neur> _neurons;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
	bool _plastic  = false;
	Params _params = {};
};
}