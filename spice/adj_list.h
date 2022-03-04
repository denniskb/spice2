#pragma once

#include <algorithm>
#include <cmath>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "util/assert.h"
#include "util/random.h"
#include "util/range.h"
#include "util/stdint.h"

namespace spice {
template <class Synapse>
class adj_list {
public:
	template <bool Const>
	class iterator_t {
	public:
		using synapse_t = std::conditional_t<Const, Synapse const, Synapse>;
		struct edge {
			Int src;
			Int dst;
			synapse_t* syn;
		};

		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = edge;

		iterator_t() = default;
		iterator_t(const iterator_t<false>& other) : _edge(other._edge), _synapse(other._synapse) {}

		edge operator*() const { return {Int(*_edge >> 32), Int(*_edge & 0xffffffff), _synapse}; }

		bool operator==(const iterator_t& other) const { return *_edge >= other._sentinel; }
		bool operator!=(const iterator_t& other) const { return *_edge < other._sentinel; }

		iterator_t& operator++() {
			_edge++;
			if constexpr (!std::is_void_v<Synapse>)
				_synapse++;

			return *this;
		}
		iterator_t operator++(int) {
			auto result = *this;
			operator++();
			return result;
		}

	private:
		friend class adj_list;

		UInt const* _edge   = nullptr;
		synapse_t* _synapse = nullptr;
		UInt _sentinel      = 0;

		iterator_t(UInt const* e, synapse_t* syn, UInt sent) : _edge(e), _synapse(syn), _sentinel(sent){};
	};
	using iterator       = iterator_t<false>;
	using const_iterator = iterator_t<true>;

	adj_list(Int const src_count, Int const dst_count, double const p, util::seed_seq seed = {1337}) {
		SPICE_ASSERT(0 <= src_count && src_count < std::numeric_limits<Int32>::max());
		SPICE_ASSERT(0 <= dst_count && dst_count < std::numeric_limits<Int32>::max());
		SPICE_ASSERT(0 <= p && p <= 1);

		if (p > 0) {
			_edges.reserve(src_count * dst_count * p);

			if constexpr (!std::is_void_v<Synapse>)
				_synapses.reserve(src_count * dst_count * p);

			util::xoroshiro64_128p rng(seed);
			util::exponential_distribution<double> exprnd(1 / p - 1);

			for (UInt const src : util::range(src_count)) {
				Int index    = 0;
				double noise = 0;
				for (;;) {
					noise += exprnd(rng);
					Int const dst = index++ + std::round(noise);

					if (__builtin_expect(dst >= dst_count, 0))
						break;

					_edges.push_back((src << 32) | dst);

					if constexpr (!std::is_void_v<Synapse>)
						_synapses.push_back({});
				}
			}
		}
		// Add sentinel so as to avoid range checks
		_edges.push_back(std::numeric_limits<UInt>::max());
	}

	Int size() const { return _edges.size() - 1; }

	iterator begin() { return {_edges.data(), _synapses.data(), _edges.front()}; }
	const_iterator begin() const { return {_edges.data(), _synapses.data(), _edges.front()}; }
	const_iterator cbegin() const { return begin(); }
	iterator end() { return {nullptr, nullptr, std::numeric_limits<UInt>::max()}; }
	const_iterator end() const { return {nullptr, nullptr, std::numeric_limits<UInt>::max()}; }
	const_iterator cend() const { return end(); }

	util::range_t<const_iterator> neighbors(Int const src) const {
		SPICE_ASSERT(0 <= src && src < std::numeric_limits<Int32>::max());

		Int const index = (std::lower_bound(_edges.begin(), _edges.end(), src << 32) - _edges.begin());
		return {{_edges.data() + index, _synapses.data() + index, 0},
		        {nullptr, nullptr, UInt(src + 1) << 32}};
	}

private:
	struct empty_synapse {};

	std::vector<UInt> _edges;
	std::vector<std::conditional_t<std::is_void_v<Synapse>, empty_synapse, Synapse>> _synapses;
};
}