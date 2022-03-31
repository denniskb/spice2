#pragma once

#include <memory>
#include <vector>

#include "spice/concepts.h"
#include "spice/connectivity.h"
#include "spice/detail/neuron_population.h"
#include "spice/detail/synapse_population.h"
#include "spice/util/numeric.h"
#include "spice/util/random.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
class snn {
public:
	snn(float const dt, float const max_delay, util::seed_seq seq) :
	_dt(dt), _max_delay(std::round(max_delay / dt)), _seed(std::move(seq)) {}

	template <Neuron Neur, class Params = util::empty_t>
	detail::neuron_population<Neur, Params>* add_population(Int const size, Params const params = {}) {
		_neurons.push_back(
		    std::make_unique<detail::neuron_population<Neur, Params>>(size, _max_delay, std::move(params)));

		return static_cast<detail::neuron_population<Neur, Params>*>(_neurons.back().get());
	}

	template <class Syn, class Params = util::empty_t, StatefulNeuron Neur, class NeurParams>
	requires(util::is_empty_v<Params> ?
	             SynapseWithoutParams<Syn, Neur> :
                 SynapseWithParams<Syn, Neur,
	                               Params>) void connect(detail::NeuronPopulation* source,
	                                                     detail::neuron_population<Neur, NeurParams>* target,
	                                                     Connectivity& c, float const delay,
	                                                     Params const params = {}) {
		static_assert(
		    !requires { &Syn::update; } || PlasticSynapse<Syn>,
		    "It looks like you're trying to define a plastic synapse (your synapse has an update() method). "
		    "However, your synapse type does not conform to the PlasticSynapse concept.");

		Int const d = std::round(delay / _dt);
		SPICE_PRE(d <= _max_delay &&
		          "The delay of a synapse population may not exceed the maximum delay of the network.");

		_synapses.push_back(
		    std::unique_ptr<detail::SynapsePopulation>(new detail::synapse_population<Syn, Neur, Params>(
		        c(source->size(), target->size()), _seed++, d, std::move(params))));

		_connections.push_back({source, _synapses.back().get(), target});

		if constexpr (PlasticSynapse<Syn>)
			source->plastic();
	}

	template <class Syn, class Params = util::empty_t, StatefulNeuron Neur, class NeurParams>
	void connect(detail::NeuronPopulation* source, detail::neuron_population<Neur, NeurParams>* target,
	             Connectivity&& c, float const delay, Params const params = {}) {
		connect<Syn, Params, Neur, NeurParams>(source, target, c, delay, std::move(params));
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