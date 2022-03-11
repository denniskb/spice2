#pragma once

#include <memory>

#include "neuron_population.h"
#include "synapse_population.h"
#include "util/random.h"

namespace spice {
class snn {
public:
	snn(float const dt, Int const max_delay, util::seed_seq seq) :
	_dt(dt), _delay(max_delay), _seed(std::move(seq)) {}

	template <Neuron Neur, class Params = void>
	neuron_population<Neur, Params>* add_population(Int const size,
	                                                util::nonvoid_or_empty_t<Params> const params = {}) {
		_neurons.push_back(
		    std::make_unique<neuron_population<Neur, Params>>(size, _delay, std::move(params)));

		return static_cast<neuron_population<Neur, Params>*>(_neurons.back().get());
	}

	template <class Syn, StatefulNeuron Neur, class NeurParams>
	requires SynapseWithoutParams<Syn, Neur>
	void connect(NeuronPopulation const* source, neuron_population<Neur, NeurParams> const* target,
	             double const p) {
		_synapses.push_back(std::unique_ptr<SynapsePopulation>(
		    new synapse_population<Syn, Neur>(source->size(), target->size(), p, _seed++)));

		_connections.push_back({source, _synapses.back().get(), target});
	}

	template <class Syn, class Params, StatefulNeuron Neur, class NeurParams>
	requires SynapseWithParams<Syn, Neur, Params>
	void connect(NeuronPopulation* source, neuron_population<Neur, NeurParams>* target, double const p,
	             Params const params) {
		_synapses.push_back(std::unique_ptr<SynapsePopulation>(new synapse_population<Syn, Neur, Params>(
		    source->size(), target->size(), p, _seed++, std::move(params))));

		_connections.push_back({source, _synapses.back().get(), target});
	}

	void step() {
		util::xoroshiro64_128p rng(_seed++);

		if (_iter >= _delay) {
			for (auto& c : _connections)
				c.synapse->deliver(c.from->spikes(_delay - 1), c.to->neurons(), c.to->size());
		}

		for (auto& pop : _neurons)
			pop->update(_delay, _dt, rng);

		_iter++;
	}

private:
	struct connection {
		NeuronPopulation* from     = nullptr;
		SynapsePopulation* synapse = nullptr;
		NeuronPopulation* to       = nullptr;
	};

	Int _iter = 0;
	float _dt;
	Int _delay;
	util::seed_seq _seed;
	std::vector<std::unique_ptr<NeuronPopulation>> _neurons;
	std::vector<std::unique_ptr<SynapsePopulation>> _synapses;
	std::vector<connection> _connections;
};
}