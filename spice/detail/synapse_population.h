#pragma once

#include <span>
#include <type_traits>

#include "spice/concepts.h"
#include "spice/connectivity.h"
#include "spice/detail/csr.h"
#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
struct SynapsePopulation {
	virtual ~SynapsePopulation()                                                    = default;
	virtual void deliver(std::span<Int32 const> spikes, void* pool, Int size) const = 0;
};

template <class Syn, StatefulNeuron Neur, class Params = util::empty_t>
requires(util::is_empty_v<Params> ? SynapseWithoutParams<Syn, Neur> :
                                    SynapseWithParams<Syn, Neur, Params>) class synapse_population :
public SynapsePopulation {
public:
	synapse_population(Connectivity& c, util::seed_seq const& seed, Params const params = {}) :
	_graph(c, seed), _params(std::move(params)) {}

	void deliver(std::span<Int32 const> spikes, void* const ptr, Int const size) const override {
		SPICE_PRE(ptr);
		SPICE_PRE(size >= 0);
		Neur* const pool = static_cast<Neur*>(ptr);

		for (auto spike : spikes)
			for (auto edge : _graph.neighbors(spike)) {
				SPICE_INV(edge.first < size);
				if constexpr (StatelessSynapseWithoutParams<Syn, Neur>)
					Syn::deliver(pool[edge.first]);
				else if constexpr (StatelessSynapseWithParams<Syn, Neur, Params>)
					Syn::deliver(pool[edge.first], _params);
				else if constexpr (StatefulSynapseWithoutParams<Syn, Neur>)
					edge.second->deliver(pool[edge.first]);
				else
					edge.second->deliver(pool[edge.first], _params);
			}
	}

private:
	detail::csr<std::conditional_t<StatefulSynapse<Syn, Neur>, Syn, util::empty_t>> _graph;
	Params _params;
};
}