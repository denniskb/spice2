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
		SPICE_PRE(ptr);
		SPICE_PRE(size >= 0);
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
	void _update(Int const iter, float const dt, auto src_neurons, Neur* const dst_neurons, Int const size,
	             std::span<UInt const> history) {
		for (auto src : src_neurons) {
			[[maybe_unused]] Int age = iter;
			if constexpr (PlasticSynapse<Syn>) {
				SPICE_INV(src < _ages.size());
				age = _ages[src];
			}

			SPICE_INV(iter >= age);
			[[maybe_unused]] UInt const mask = ~UInt(0) >> (64 - (iter - age + (iter == age)));

			util::invoke(iter > age, [&]<bool Update>() {
				for (auto edge : _graph.neighbors(src)) {
					if constexpr (PlasticSynapse<Syn> && Update) {
						SPICE_INV(edge.first < history.size());
						UInt hist = history[edge.first] & mask;
						Int i     = iter - age;
						SPICE_INV(i <= 64);
						while (hist) {
							Int hsb = 63 - __builtin_clzl(hist);
							if constexpr (SynapseWithParams<Syn, Neur, Params>)
								edge.second->update(dt, hsb == 0, true, i - hsb, _params);
							else
								edge.second->update(dt, hsb == 0, true, i - hsb);

							hist ^= UInt(1) << hsb;
							i = hsb;
						}
						if constexpr (SynapseWithParams<Syn, Neur, Params>)
							edge.second->update(dt, i > 0, false, i, _params);
						else
							edge.second->update(dt, i > 0, false, i);
					}

					if constexpr (Deliver) {
						SPICE_INV(edge.first < size);
						if constexpr (StatelessSynapseWithoutParams<Syn, Neur>)
							Syn::deliver(dst_neurons[edge.first]);
						else if constexpr (StatelessSynapseWithParams<Syn, Neur, Params>)
							Syn::deliver(dst_neurons[edge.first], _params);
						else if constexpr (StatefulSynapseWithoutParams<Syn, Neur>)
							edge.second->deliver(dst_neurons[edge.first]);
						else
							edge.second->deliver(dst_neurons[edge.first], _params);
					}
				}
			});

			if constexpr (PlasticSynapse<Syn>)
				_ages[src] = iter;
		}
	}
};
}