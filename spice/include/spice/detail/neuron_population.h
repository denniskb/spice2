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

// The following adapters provide a unified interface (size(), update()) to a variety of neuron types

template <Neuron Neur>
class stateless_neuron_adapter {
public:
	stateless_neuron_adapter(Neur neuron, Int const size, util::seed_seq&) :
	_neuron(std::move(neuron)), _size(size) {
		SPICE_INV(StatelessNeuron<Neur>);
		SPICE_INV(size >= 0);
	}

	Int size() const { return _size; }

	void update(float const dt, auto& rng, std::vector<Int32>& out_spikes) {
		for (Int const i : util::range(size())) {
			if (_neuron.update(dt, rng))
				out_spikes.push_back(i);
		}
	}

private:
	Neur _neuron;
	Int _size;
};

template <Neuron Neur>
class stateful_neuron_adapter {
public:
	stateful_neuron_adapter(Neur neuron, Int const size, util::seed_seq& seed) :
	_neuron(std::move(neuron)), _neurons(size) {
		SPICE_INV(StatefulNeuron<Neur>);
		SPICE_INV(size >= 0);

		util::xoroshiro64_128p rng(seed++);

		if constexpr (PerNeuronInit<Neur>) {
			Int id = 0;
			for (auto& n : _neurons)
				neuron.init(n, id++, rng);
		} else if constexpr (PerPopulationInit<Neur>)
			neuron.init(std::span<typename Neur::neuron>(_neurons), rng);
	}

	Int size() const { return _neurons.size(); }

	void update(float const dt, auto& rng, std::vector<Int32>& out_spikes) {
		for (Int const i : util::range(size())) {
			if (_neuron.update(_neurons[i], dt, rng))
				out_spikes.push_back(i);
		}
	}

	std::span<typename Neur::neuron> neurons() { return _neurons; }

private:
	Neur _neuron;
	std::vector<neuron_traits_t<Neur>> _neurons;
};

template <Neuron Neur>
class per_pop_update_adapter : public Neur {
public:
	per_pop_update_adapter(Neur neuron, Int const size, util::seed_seq&) :
	Neur(std::move(neuron)), _size(size) {
		SPICE_INV(PerPopulationUpdate<Neur>);
		SPICE_INV(size >= 0);
	}

	Int size() const { return _size; }

	// using Neur::update()

private:
	Int _size;
};

template <Neuron Neur>
class neuron_population : public NeuronPopulation {
public:
	neuron_population(Neur neuron, Int const size, util::seed_seq& seed, Int const max_delay) :
	_neuron(std::move(neuron), size, seed) {
		SPICE_INV(max_delay >= 1);

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
		if constexpr (StatefulNeuron<Neur>)
			return get_neurons().data();
		else
			return nullptr;
	}
	auto get_neurons() {
		static_assert(StatefulNeuron<Neur>, "Can only return collections of stateful neurons.");
		return _neuron.neurons();
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
	std::conditional_t<PerPopulationUpdate<Neur>, per_pop_update_adapter<Neur>,
	                   std::conditional_t<StatefulNeuron<Neur>, stateful_neuron_adapter<Neur>,
	                                      stateless_neuron_adapter<Neur>>>
	    _neuron;
	std::vector<Int32> _spikes;
	std::vector<Int32> _spike_counts;
	std::vector<UInt> _history;
	bool _plastic = false;
};
}