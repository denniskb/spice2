#pragma once

#include <concepts>

namespace spice::util {
template <std::floating_point Real>
class kahan_sum {
public:
	__attribute__((optimize("-fno-fast-math"))) constexpr Real operator+=(Real delta) {
		auto const y = delta - _c;
		auto const t = _sum + y;
		_c           = (t - _sum) - y;
		_sum         = t;
		return y;
	}

	constexpr operator Real() const { return _sum; }

private:
	Real _c   = 0;
	Real _sum = 0;
};
}