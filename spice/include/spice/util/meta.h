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

// TODO: Write generic version
template <class F, class... Args>
constexpr auto invoke(bool const a, bool const b, F&& callable, Args&&... args) {
	if (a) {
		if (b)
			std::forward<F>(callable).template operator()<true, true>(std::forward<Args>(args)...);
		else
			std::forward<F>(callable).template operator()<true, false>(std::forward<Args>(args)...);
	} else {
		if (b)
			std::forward<F>(callable).template operator()<false, true>(std::forward<Args>(args)...);
		else
			std::forward<F>(callable).template operator()<false, false>(
			    std::forward<Args>(args)...);
	}
}
}