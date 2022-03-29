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
	virtual ~SynapsePopulation()                                                             = default;
	virtual void deliver(Int time, float dt, std::span<Int32 const> spikes, void* ptr, Int size,
	                     std::span<UInt const> dst_history)                                  = 0;
	virtual void update(Int time, float dt, Int src_size, std::span<UInt const> dst_history) = 0;
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

	void deliver(Int const time, float const dt, std::span<Int32 const> spikes, void* const ptr,
	             Int const size, std::span<UInt const> dst_history) override {
		SPICE_INV(ptr);
		SPICE_INV(size >= 0);

		Neur* const dst_neurons = static_cast<Neur*>(ptr);
		_update<true>(time, dt, spikes, {dst_neurons, static_cast<UInt>(size)}, dst_history);
	}

	void update(Int const time, float const dt, Int const src_size,
	            std::span<UInt const> dst_history) override {
		if constexpr (PlasticSynapse<Syn>)
			_update<false>(time, dt, util::range(src_size), {}, dst_history);
	}

private:
	detail::csr<std::conditional_t<StatefulSynapse<Syn, Neur>, Syn, util::empty_t>> _graph;
	std::vector<UInt> _ages;
	Params _params;

	template <bool Deliver>
	void _update(Int const time, float const dt, auto spikes, std::span<Neur> dst_neurons,
	             std::span<UInt const> dst_history) {
		static_assert(Deliver || PlasticSynapse<Syn>);

		for (auto src : spikes) {
			bool const pre = PlasticSynapse<Syn> ? _ages[src] >> 63 : false;
			Int const age  = PlasticSynapse<Syn> ? _ages[src] & ~(1_u64 << 63) : time + 1;

			util::invoke(time >= age, [&]<bool Outdated>() {
				for (auto edge : _graph.neighbors(src)) {
					if constexpr (PlasticSynapse<Syn> && Outdated) {
						SPICE_INV(edge.first < dst_history.size());
						// TODO: Move pre into compile-time, don't special case first update call.
						edge.second->update(dt, pre, dst_history[edge.first] & (1_u64 << (time - age)), 1);
						UInt hist  = dst_history[edge.first] & (~0_u64 >> (64 - time + age));
						Int prefix = 64 - time + age;
						while (hist) {
							// TODO: Cleanup, optimize
							Int lz = __builtin_clzl(hist) - prefix;
							edge.second->skip(dt, lz);
							edge.second->update(dt, false, true, 1);
							hist ^= 1_u64 << (63 - lz - prefix);
							prefix += lz + 1;
						}
						edge.second->skip(dt, 64 - prefix);
					}

					if constexpr (Deliver) {
						SPICE_INV(edge.first < dst_neurons.size());
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
				_ages[src] = (time + 1) | (UInt(Deliver) << 63);
		}
	}
};
}