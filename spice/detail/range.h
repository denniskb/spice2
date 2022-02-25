#pragma once

#include <algorithm>
#include <type_traits>

#include "stdint.h"

namespace spice::detail {
class range {
public:
	struct int_iterator {
		Int i;
		constexpr Int operator*() const { return i; }
		constexpr operator Int&() { return i; }
	};

	template <class Container, typename std::enable_if_t<!std::is_integral_v<Container>>* = nullptr>
	constexpr explicit range(Container const& container) : range(container.size()) {}
	constexpr explicit range(Int max) : range(0, max) {}
	constexpr range(Int min, Int max) : _min(min), _max(std::max(min, max)) {}

	constexpr int_iterator begin() const { return _min; }
	constexpr int_iterator end() const { return _max; }

private:
	int_iterator const _min;
	int_iterator const _max;
};
}