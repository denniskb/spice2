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
	virtual ~SynapsePopulation()                            = default;
	virtual void deliver(Int time, Int delay, float dt, std::span<Int32 const> spikes, void* const ptr,
	                     Int const size, std::span<UInt const> src_history,
	                     std::span<UInt const> dst_history) = 0;
	virtual void update(Int time, Int delay, float dt, std::span<UInt const> src_history,
	                    std::span<UInt const> dst_history)  = 0;
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

	void deliver(Int const time, Int const, float const dt, std::span<Int32 const> spikes, void* const ptr,
	             Int const size, std::span<UInt const>, std::span<UInt const> dst_history) override {
		SPICE_INV(ptr);
		SPICE_INV(size >= 0);
		Neur* const dst_neurons = static_cast<Neur*>(ptr);

		for (auto src : spikes) {
			bool pre = false;
			UInt age = 0;
			if constexpr (PlasticSynapse<Syn>) {
				pre = _ages[src] >> 63;
				age = _ages[src] & (~0_u64 >> 1);
			}
			for (auto edge : _graph.neighbors(src)) {
				if constexpr (PlasticSynapse<Syn>) {
					for (Int i = age; i <= time; i++)
						edge.second->update(dt, (i == age) & pre,
						                    dst_history[edge.first] & (1_u64 << (time - i)), 1);
				}

				SPICE_INV(edge.first < size);
				// TODO: Handle remaining cases
				if constexpr (StatelessSynapseWithParams<Syn, Neur, Params>)
					Syn::deliver(dst_neurons[edge.first], _params);
				else if constexpr (StatefulSynapseWithoutParams<Syn, Neur>)
					edge.second->deliver(dst_neurons[edge.first]);
			}
			if constexpr (PlasticSynapse<Syn>)
				_ages[src] = (time + 1) | (1_u64 << 63);
		}
	}

	void update(Int const time, Int const, float const dt, std::span<UInt const> src_history,
	            std::span<UInt const> dst_history) override {
		if constexpr (PlasticSynapse<Syn>) {
			for (auto src : util::range(src_history.size())) {
				bool const pre = _ages[src] >> 63;
				UInt const age = _ages[src] & (~0_u64 >> 1);
				for (auto edge : _graph.neighbors(src)) {
					for (Int i = age; i <= time; i++)
						edge.second->update(dt, (i == age) & pre,
						                    dst_history[edge.first] & (1_u64 << (time - i)), 1);
				}
				_ages[src] = time + 1;
			}
		}
	}

private:
	detail::csr<std::conditional_t<StatefulSynapse<Syn, Neur>, Syn, util::empty_t>> _graph;
	std::vector<UInt> _ages;
	Params _params;
};
}