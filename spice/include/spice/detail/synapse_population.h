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
	virtual Int delay() const                                                                = 0;
};

template <class Syn, StatefulNeuron Neur, class Params = util::empty_t>
requires(util::is_empty_v<Params> ? SynapseWithoutParams<Syn, Neur> :
                                    SynapseWithParams<Syn, Neur, Params>) class synapse_population :
public SynapsePopulation {
public:
	synapse_population(Connectivity& c, util::seed_seq const& seed, Int const delay,
	                   Params const params = {}) :
	_graph(c, seed),
	_delay(delay), _params(std::move(params)) {
		SPICE_PRE(delay >= 1);
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

	Int delay() const override { return _delay; }

private:
	detail::csr<std::conditional_t<StatefulSynapse<Syn, Neur>, Syn, util::empty_t>> _graph;
	std::vector<UInt> _ages;
	Int _delay;
	Params _params;

	template <bool Deliver>
	void _update(Int const time, float const dt, auto spikes, std::span<Neur> dst_neurons,
	             std::span<UInt const> dst_history) {
		static_assert(Deliver || PlasticSynapse<Syn>);

		for (auto src : spikes) {
			bool const pre   = PlasticSynapse<Syn> ? _ages[src] >> 63 : false;
			Int const age    = PlasticSynapse<Syn> ? _ages[src] & ~(1_u64 << 63) : time + 1;
			Int const prefix = 63 + pre - time + age;
			UInt const mask  = ~0_u64 >> prefix;

			util::invoke(pre, time >= age, [&]<bool Pre, bool Outdated>() {
				for (auto edge : _graph.neighbors(src)) {
					if constexpr (PlasticSynapse<Syn> && Outdated) {
						SPICE_INV(edge.first < dst_history.size());
						UInt hist = dst_history[edge.first];
						if constexpr (Pre)
							edge.second->update(dt, true, hist & (1_u64 << (time - age)));

						hist &= mask;
						Int p = prefix;
						while (hist) {
							Int const lz = __builtin_clzl(hist);
							edge.second->skip(dt, lz - p);
							edge.second->update(dt, false, true);
							hist ^= 1_u64 << (63 - lz);
							p = lz + 1;
						}
						edge.second->skip(dt, 64 - p);
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