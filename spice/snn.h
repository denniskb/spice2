#pragma once

#include <memory>

#include "spice/detail/neuron_population.h"
#include "spice/detail/synapse_population.h"
#include "spice/util/random.h"

namespace spice {
class snn {
public:
	snn(float const dt, Int const max_delay, util::seed_seq seq) :
	_dt(dt), _delay(max_delay), _seed(std::move(seq)) {}

	template <Neuron Neur, class Params = void>
	detail::neuron_population<Neur, Params>*
	add_population(Int const size, util::nonvoid_or_empty_t<Params> const params = {}) {
		_neurons.push_back(
		    std::make_unique<detail::neuron_population<Neur, Params>>(size, _delay, std::move(params)));

		return static_cast<detail::neuron_population<Neur, Params>*>(_neurons.back().get());
	}

	template <class Syn, StatefulNeuron Neur, class NeurParams>
	requires SynapseWithoutParams<Syn, Neur>
	void connect(detail::NeuronPopulation const* source,
	             detail::neuron_population<Neur, NeurParams> const* target, double const p) {
		_synapses.push_back(std::unique_ptr<detail::SynapsePopulation>(
		    new detail::synapse_population<Syn, Neur>(source->size(), target->size(), p, _seed++)));

		_connections.push_back({source, _synapses.back().get(), target});
	}

	template <class Syn, class Params, StatefulNeuron Neur, class NeurParams>
	requires SynapseWithParams<Syn, Neur, Params>
	void connect(detail::NeuronPopulation* source, detail::neuron_population<Neur, NeurParams>* target,
	             double const p, Params const params) {
		_synapses.push_back(
		    std::unique_ptr<detail::SynapsePopulation>(new detail::synapse_population<Syn, Neur, Params>(
		        source->size(), target->size(), p, _seed++, std::move(params))));

		_connections.push_back({source, _synapses.back().get(), target});
	}

	void step();

private:
	struct connection {
		detail::NeuronPopulation* from     = nullptr;
		detail::SynapsePopulation* synapse = nullptr;
		detail::NeuronPopulation* to       = nullptr;
	};

	Int _iter = 0;
	float _dt;
	Int _delay;
	util::seed_seq _seed;
	std::vector<std::unique_ptr<detail::NeuronPopulation>> _neurons;
	std::vector<std::unique_ptr<detail::SynapsePopulation>> _synapses;
	std::vector<connection> _connections;
};
}