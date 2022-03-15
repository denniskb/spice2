#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>

#include "spice/util/stdint.h"

namespace spice::util {
template <class It>
struct range_t {
public:
	It first;
	It last;

	constexpr It begin() const { return first; }
	constexpr It end() const { return last; }
};

struct int_iterator {
	Int i;
	constexpr Int operator*() const { return i; }
	constexpr operator Int&() { return i; }
};

inline constexpr range_t<int_iterator> range(Int min, Int max) { return {min, std::max(min, max)}; }
inline constexpr range_t<int_iterator> range(Int max) { return range(0, max); }

template <class Container>
requires requires(Container c) {
	{ c.size() } -> std::integral;
}
constexpr auto range(Container const& c) { return range(c.size()); }
}