#pragma once

#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

#include "spice/connectivity.h"
#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice::detail {
template <class T = util::empty_t>
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
			if constexpr (!std::is_empty_v<T>)
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

	csr(Connectivity& c, util::seed_seq const& seed) {
		_offsets.resize(c.src_count > 0 ? c.src_count + 1 : 0);
		_neighbors.resize(c.size());
		if constexpr (!util::is_empty_v<T>)
			_edges.resize(_neighbors.size());

		c.fill_csr(_offsets, _neighbors, seed);
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
	std::vector<T> _edges;
};
}