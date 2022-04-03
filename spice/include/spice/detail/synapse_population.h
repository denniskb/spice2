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

template <class Syn, StatefulNeuron Neur>
requires Synapse<Syn, Neur>
class synapse_population : public SynapsePopulation {
public:
	synapse_population(Syn syn, Connectivity& c, util::seed_seq& seed, Int const delay) :
	_syn(std::move(syn)), _graph(c, seed++), _delay(delay) {
		SPICE_PRE(delay >= 1);

		if constexpr (PerSynapseInit<Syn, Neur>) {
			util::xoroshiro64_128p rng(seed++);
			for (auto src : util::range(c.src_count)) {
				for (auto edge : _graph.neighbors(src)) {
					_syn.init(*edge.second, src, edge.first, rng);
				}
			}
		}

		if constexpr (PlasticSynapse<Syn, Neur>)
			_ages.resize(c.src_count);
	}

	void deliver(Int const time, float const dt, std::span<Int32 const> spikes, void* const ptr,
	             Int const size, std::span<UInt const> dst_history) override {
		SPICE_INV(ptr);
		SPICE_INV(size >= 0);

		auto dst_neurons = static_cast<typename Neur::neuron*>(ptr);
		_update<true>(time, dt, spikes, {dst_neurons, static_cast<UInt>(size)}, dst_history);
	}

	void update(Int const time, float const dt, Int const src_size,
	            std::span<UInt const> dst_history) override {
		if constexpr (PlasticSynapse<Syn, Neur>)
			_update<false>(time, dt, util::range(src_size), {}, dst_history);
	}

	Int delay() const override { return _delay; }

private:
	Syn _syn;
	detail::csr<synapse_traits_t<Syn, Neur>> _graph;
	std::vector<UInt> _ages;
	Int _delay;

	template <bool Deliver>
	void _update(Int const time, float const dt, auto spikes, std::span<typename Neur::neuron> dst_neurons,
	             std::span<UInt const> dst_history) {
		static_assert(Deliver || PlasticSynapse<Syn, Neur>);

		for (auto src : spikes) {
			bool const pre   = PlasticSynapse<Syn, Neur> ? _ages[src] >> 63 : false;
			Int const age    = PlasticSynapse<Syn, Neur> ? _ages[src] & ~(1_u64 << 63) : time + 1;
			Int const prefix = 63 + pre - time + age;
			UInt const mask  = ~0_u64 >> prefix;

			util::invoke(pre, time >= age, [&]<bool Pre, bool Outdated>() {
				for (auto edge : _graph.neighbors(src)) {
					if constexpr (PlasticSynapse<Syn, Neur> && Outdated) {
						SPICE_INV(edge.first < dst_history.size());
						UInt hist = dst_history[edge.first];
						if constexpr (Pre)
							_syn.update(*edge.second, dt, true, hist & (1_u64 << (time - age)));

						hist &= mask;
						Int p = prefix;
						while (hist) {
							Int const lz = __builtin_clzl(hist);
							_syn.skip(*edge.second, dt, lz - p);
							_syn.update(*edge.second, dt, false, true);
							hist ^= 1_u64 << (63 - lz);
							p = lz + 1;
						}
						_syn.skip(*edge.second, dt, 64 - p);
					}

					if constexpr (Deliver) {
						SPICE_INV(edge.first < dst_neurons.size());
						if constexpr (StatelessSynapse<Syn, Neur>)
							_syn.deliver(dst_neurons[edge.first]);
						else
							_syn.deliver(*edge.second, dst_neurons[edge.first]);
					}
				}
			});

			if constexpr (PlasticSynapse<Syn, Neur>)
				_ages[src] = (time + 1) | (UInt(Deliver) << 63);
		}
	}
};
}