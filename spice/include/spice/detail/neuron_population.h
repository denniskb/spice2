#pragma once

#include <algorithm>
#include <numeric>
#include <span>
#include <vector>

#include "spice/concepts.h"
#include "spice/util/assert.h"
#include "spice/util/meta.h"
#include "spice/util/random.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
struct NeuronPopulation {
	virtual ~NeuronPopulation()                                               = default;
	virtual Int size() const                                                  = 0;
	virtual void update(Int max_delay, float dt, util::xoroshiro64_128p& rng) = 0;
	virtual void* neurons()                                                   = 0;
	virtual std::span<Int32 const> spikes(Int age) const                      = 0;
	virtual void plastic()                                                    = 0;
	virtual std::span<UInt const> history() const                             = 0;
};

template <Neuron Neur>
class per_neuron_update {
public:
	per_neuron_update() = default;
	per_neuron_update(Neur neuron, util::seed_seq& seed, Int const size) : _neuron(std::move(neuron)) {
		SPICE_INV(size >= 0);

		if constexpr (StatefulNeuron<Neur>)
			_neurons.resize(size);
		else
			_size = size;

		util::xoroshiro64_128p rng(seed++);
		if constexpr (PerNeuronInit<Neur>) {
			Int id = 0;
			for (auto& n : _neurons)
				_neuron.init(n, id++, rng);
		}
		if constexpr (PerPopulationInit<Neur>)
			_neuron.init(std::span<typename Neur::neuron>(_neurons), rng);
	}

	void update(float const dt, auto& rng, std::vector<Int32>& out_spikes) {
		for (Int const i : util::range(size())) {
			bool spiked;
			if constexpr (StatelessNeuron<Neur>)
				spiked = _neuron.update(dt, rng);
			else
				spiked = _neuron.update(_neurons[i], dt, rng);

			if (spiked)
				out_spikes.push_back(i);
		}
	}

	Int size() const {
		if constexpr (StatefulNeuron<Neur>)
			return _neurons.size();
		else
			return _size;
	}

	template <class N = Neur>
	std::enable_if_t<StatefulNeuron<N>, std::span<typename N::neuron>> neurons() {
		return _neurons;
	}

private:
	Neur _neuron;
	[[no_unique_address]] std::conditional_t<StatelessNeuron<Neur>, Int, util::empty_t> _size;
	[[no_unique_address]] std::conditional_t<StatefulNeuron<Neur>, std::vector<neuron_traits_t<Neur>>,
	                                         util::empty_t>
	    _neurons;
};

template <Neuron Neur>
class neuron_population : public NeuronPopulation {
public:
	neuron_population(Neur neuron, util::seed_seq& seed, Int const size, Int const max_delay) {
		SPICE_INV(max_delay >= 1);

		if constexpr (PerPopulationUpdate<Neur>)
			_neuron = std::move(neuron);
		else
			_neuron = {std::move(neuron), seed, size};

		_spike_counts.reserve(max_delay);
		_spikes.reserve(size * max_delay / 100);
	}

	Int size() const override { return _neuron.size(); }

	void update(Int const max_delay, float const dt, util::xoroshiro64_128p& rng) override {
		SPICE_INV(max_delay >= 1);

		if (_spike_counts.size() == max_delay) {
			_spikes.erase(_spikes.begin(), _spikes.begin() + _spike_counts.front());
			_spike_counts.erase(_spike_counts.begin());
		}

		Int const spike_count = _spikes.size();
		_neuron.update(dt, rng, _spikes);
		if (_plastic) {
			for (auto& hist : _history)
				hist <<= 1;

			for (auto spike : util::range(_spikes.begin() + spike_count, _spikes.end()))
				_history[spike] |= 1;
		}
		_spike_counts.push_back(_spikes.size() - spike_count);
	}

	void* neurons() override {
		SPICE_PRE(StatefulNeuron<Neur> && "Can't return collection of stateless neurons.");
		if constexpr (StatefulNeuron<Neur>)
			return get_neurons().data();
		else
			return nullptr;
	}
	auto get_neurons() { return _neuron.neurons(); }

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
	std::conditional_t<PerPopulationUpdate<Neur>, Neur, per_neuron_update<Neur>> _neuron;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
	bool _plastic = false;
};
}