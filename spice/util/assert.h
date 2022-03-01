#pragma once

#define SPICE_ASSERT(X)                                  \
	if (__builtin_expect_with_probability(!(X), 0, 1.0)) \
		::spice::util::assert_failed(__FILE__, __LINE__, #X);

namespace spice::util {
void assert_failed(char const* file, int const line, char const* condition);
}