#pragma once

#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
template <class T = void>
class csr {
public:
	template <bool Const>
	class iterator_t {
	public:
		using edge_t = std::conditional_t<Const, T const, T>;

		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = std::pair<Int32, edge_t*>;

		constexpr iterator_t() = default;
		constexpr iterator_t(const iterator_t<false>& other) : _dst(other._dst), _edge(other._edge) {}

		constexpr value_type operator*() const { return {*_dst, _edge}; }

		constexpr bool operator==(const iterator_t& other) const { return _dst == other._dst; }
		constexpr bool operator!=(const iterator_t& other) const { return _dst != other._dst; }

		constexpr iterator_t& operator++() {
			_dst++;
			if constexpr (!std::is_void_v<T>)
				_edge++;

			return *this;
		}
		constexpr iterator_t operator++(int) {
			auto result = *this;
			operator++();
			return result;
		}

	private:
		friend class csr;

		Int32 const* _dst = nullptr;
		edge_t* _edge     = nullptr;

		constexpr iterator_t(Int32 const* dst, edge_t* edge) : _dst(dst), _edge(edge) {}
	};
	using iterator       = iterator_t<false>;
	using const_iterator = iterator_t<true>;

	csr(Int const src_count, Int const dst_count, double const p, util::seed_seq const& seed) {
		SPICE_PRE(src_count >= 0);
		SPICE_PRE(dst_count >= 0);
		SPICE_PRE(0 <= p && p <= 1);

		if (src_count == 0 || dst_count == 0 || p == 0)
			return;

		Int const max_degree = dst_count * p + 3 * std::sqrt(dst_count * p * (1 - p));
		_offsets.resize(src_count + 1);
		_neighbors.resize(src_count * max_degree);
		if constexpr (!std::is_void_v<T>)
			_edges.resize(_neighbors.size());

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

				_neighbors[count] = dst;

				index++;
				count++;
			}
		}
		_offsets.back() = count;
	}

	util::range_t<const_iterator> neighbors(Int const src) const {
		SPICE_PRE(0 <= src && src + 1 < _offsets.size());

		Int const first = _offsets[src];
		Int const last  = _offsets[src + 1];

		return {{_neighbors.data() + first, _edges.data() + first},
		        {_neighbors.data() + last, _edges.data() + last}};
	}

private:
	std::vector<Int> _offsets;
	std::vector<Int32> _neighbors;
	std::vector<util::nonvoid_or_empty_t<T>> _edges;
};
}