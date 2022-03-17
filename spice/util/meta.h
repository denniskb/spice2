#pragma once

#include <utility>

namespace spice::util {
template <class F, class... Args>
constexpr auto invoke(bool const b, F&& callable, Args&&... args) {
	if (b)
		std::forward<F>(callable).template operator()<true>(std::forward<Args>(args)...);
	else
		std::forward<F>(callable).template operator()<false>(std::forward<Args>(args)...);
}
}