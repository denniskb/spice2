#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "concepts.h"
#include "util/assert.h"
#include "util/random.h"
#include "util/range.h"
#include "util/scope.h"
#include "util/stdint.h"
#include "util/type_traits.h"

namespace spice {
// TODO: Can this be written shorter?
template <class Syn, StatefulNeuron TargetNeuron>
requires Synapse<Syn, TargetNeuron>
class synapse_population {
public:
	template <bool Const>
	class iterator_t {
	public:
		using synapse_t = std::conditional_t<Const, Syn const, Syn>;
		struct edge {
			Int dst;
			synapse_t* syn;
		};

		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = edge;

		iterator_t() = default;
		iterator_t(const iterator_t<false>& other) : _edge(other._edge), _synapse(other._synapse) {}

		edge operator*() const { return {*_edge, _synapse}; }

		bool operator==(const iterator_t& other) const { return _edge == other._edge; }
		bool operator!=(const iterator_t& other) const { return _edge != other._edge; }

		iterator_t& operator++() {
			_edge++;
			if constexpr (StatefulSynapse<Syn, TargetNeuron>)
				_synapse++;

			return *this;
		}
		iterator_t operator++(int) {
			auto result = *this;
			operator++();
			return result;
		}

	private:
		friend class synapse_population;

		Int32 const* _edge  = nullptr;
		synapse_t* _synapse = nullptr;

		iterator_t(Int32 const* e, synapse_t* syn) : _edge(e), _synapse(syn){};
	};
	using iterator       = iterator_t<false>;
	using const_iterator = iterator_t<true>;

	synapse_population(Int const src_count, Int const dst_count, double const p,
	                   util::seed_seq seed = {1337}) :
	_offsets(src_count + 1) {
		SPICE_ASSERT(0 <= src_count && src_count < std::numeric_limits<Int32>::max());
		SPICE_ASSERT(0 <= dst_count && dst_count < std::numeric_limits<Int32>::max());
		SPICE_ASSERT(0 <= p && p <= 1);

		if (p == 0)
			return;

		// Allocate enough space for 3std
		Int const max_degree =
		    std::max<Int>(0, std::round(dst_count * p + 3 * std::sqrt(dst_count * p * (1 - p))));
		_edges.resize(src_count * max_degree);

		if constexpr (StatefulSynapse<Syn, TargetNeuron>)
			_synapses.resize(src_count * max_degree);

		util::xoroshiro64_128p rng(seed);
		util::exponential_distribution<double> exprnd(1 / p - 1);

		Int count = 0;
		for (UInt const src : util::range(src_count)) {
			_offsets[src] = count;

			Int32 index  = 0;
			double noise = 0;
			for (;;) {
				noise += exprnd(rng);
				Int32 const dst = index + static_cast<Int32>(std::round(noise));

				if (__builtin_expect((dst >= dst_count) | (index >= max_degree), 0))
					break;

				_edges[count] = dst;

				if constexpr (StatefulSynapse<Syn, TargetNeuron>)
					_synapses[count] = {};

				index++;
				count++;
			}
		}
		_offsets.back() = count;
	}

	util::range_t<const_iterator> neighbors(Int const src) const {
		SPICE_ASSERT(0 <= src && src < _offsets.size() - 1);

		Int const first = _offsets[src];
		Int const last  = _offsets[src + 1];

		return {{_edges.data() + first, _synapses.data() + first},
		        {_edges.data() + last, _synapses.data() + last}};
	}

	void deliver(std::span<Int32 const> spikes, std::span<TargetNeuron> pool) const {
		for (auto spike : spikes)
			for (auto edge : neighbors(spike)) {
				SPICE_ASSERT(edge.dst < pool.size());
				if constexpr (StatelessSynapse<Syn, TargetNeuron>)
					Syn::deliver(pool[edge.dst]);
				else
					edge.syn->deliver(pool[edge.dst]);
			}
	}

private:
	std::vector<Int> _offsets;
	std::vector<Int32> _edges;
	std::vector<Syn> _synapses;
};
}