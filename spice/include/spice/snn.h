#pragma once

#include <memory>
#include <vector>

#include "spice/concepts.h"
#include "spice/detail/neuron_population.h"
#include "spice/detail/synapse_population.h"
#include "spice/topology.h"
#include "spice/util/numeric.h"
#include "spice/util/random.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
class snn {
public:
	snn(float const dt, float const max_delay, util::seed_seq seed) :
	_dt(dt), _max_delay(std::round(max_delay / dt)), _seed(std::move(seed)) {}

	template <Neuron Neur>
	detail::neuron_population<Neur>* add_population(Int const size, Neur neur = {}) {
		_neurons.push_back(std::make_unique<detail::neuron_population<Neur>>(std::move(neur), size,
		                                                                     _seed, _max_delay));

		return static_cast<detail::neuron_population<Neur>*>(_neurons.back().get());
	}

	template <class Syn, Neuron SrcNeur, StatefulNeuron DstNeur>
	requires Synapse<Syn, SrcNeur, DstNeur>
	void connect(detail::neuron_population<SrcNeur>* source,
	             detail::neuron_population<DstNeur>* target, Topology& c, float const delay,
	             Syn syn = {}) {
		Int const d = std::round(delay / _dt);
		SPICE_PRE(d >= 1 && "The delay must be at least 1dt.");
		SPICE_PRE(
		    d <= _max_delay &&
		    "The delay of a synapse population may not exceed the maximum delay of the network.");

		_synapses.push_back(std::unique_ptr<detail::SynapsePopulation>(
		    new detail::synapse_population<Syn, SrcNeur, DstNeur>(
		        std::move(syn), c(source->size(), target->size()), _seed, d)));

		_connections.push_back({source, _synapses.back().get(), target});

		if constexpr (PlasticSynapse<Syn>)
			source->plastic();
	}

	template <class Syn, Neuron SrcNeur, StatefulNeuron DstNeur>
	requires Synapse<Syn, SrcNeur, DstNeur>
	void connect(detail::neuron_population<SrcNeur>* source,
	             detail::neuron_population<DstNeur>* target, Topology&& c, float const delay,
	             Syn syn = {}) {
		connect<Syn, SrcNeur, DstNeur>(source, target, c, delay, std::move(syn));
	}

	void step();

private:
	struct connection {
		detail::NeuronPopulation* from     = nullptr;
		detail::SynapsePopulation* synapse = nullptr;
		detail::NeuronPopulation* to       = nullptr;
	};

	Int _time = 0;
	float _dt;
	Int _max_delay;
	util::kahan_sum<float> _simtime;
	util::seed_seq _seed;
	std::vector<std::unique_ptr<detail::NeuronPopulation>> _neurons;
	std::vector<std::unique_ptr<detail::SynapsePopulation>> _synapses;
	std::vector<connection> _connections;
};
}