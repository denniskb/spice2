#pragma once

#include <span>
#include <type_traits>

#include "spice/concepts.h"
#include "spice/connectivity.h"
#include "spice/detail/csr.h"
#include "spice/util/assert.h"
#include "spice/util/meta.h"
#include "spice/util/random.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
struct SynapsePopulation {
	virtual ~SynapsePopulation()                                                       = default;
	virtual void deliver(Int iter, float dt, std::span<Int32 const> spikes, void* pool, Int size,
	                     std::span<UInt const> history)                                = 0;
	virtual void update(Int const iter, float const dt, std::span<UInt const> history) = 0;
};

template <class Syn, StatefulNeuron Neur, class Params = util::empty_t>
requires(util::is_empty_v<Params> ? SynapseWithoutParams<Syn, Neur> :
                                    SynapseWithParams<Syn, Neur, Params>) class synapse_population :
public SynapsePopulation {
public:
	synapse_population(Connectivity& c, util::seed_seq const& seed, Params const params = {}) :
	_graph(c, seed), _params(std::move(params)) {
		if constexpr (PlasticSynapse<Syn>)
			_ages.resize(c.src_count);
	}

	void deliver(Int const iter, float const dt, std::span<Int32 const> spikes, void* const ptr,
	             Int const size, std::span<UInt const> history) override {
		SPICE_INV(ptr);
		SPICE_INV(size >= 0);
		Neur* const dst_neurons = static_cast<Neur*>(ptr);

		_update<true>(iter, dt, spikes, dst_neurons, size, history);
	}

	void update(Int const iter, float const dt, std::span<UInt const> history) override {
		if constexpr (PlasticSynapse<Syn>)
			_update<false>(iter, dt, util::range(_ages.size()), nullptr, 0, history);
	}

private:
	detail::csr<std::conditional_t<StatefulSynapse<Syn, Neur>, Syn, util::empty_t>> _graph;
	std::vector<Int> _ages;
	Params _params;

	template <bool Deliver>
	void _update(Int const iter, float const dt, auto spikes, Neur* const dst_neurons, Int const,
	             std::span<UInt const> history) {
		for (auto src : spikes) {
			for (auto edge : _graph.neighbors(src)) {
				if constexpr (PlasticSynapse<Syn> && !Deliver) // post() + update()
					edge.second->update(dt, false, history[edge.first] & 1, 1);

				if constexpr (PlasticSynapse<Syn> && Deliver) { // pre()
					// TODO: Hard-coded delay
					edge.second->update(dt, iter >= 14, false, 0);

					if constexpr (StatefulSynapseWithoutParams<Syn, Neur>)
						edge.second->deliver(dst_neurons[edge.first]);
				}

				if constexpr (StatelessSynapseWithParams<Syn, Neur, Params> && Deliver)
					Syn::deliver(dst_neurons[edge.first], _params);
			}
		}
	}
};
}