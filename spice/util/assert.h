#pragma once

#ifdef SPICE_NO_ASSERT
	#define SPICE_ASSERT(X)
#else
	#define SPICE_ASSERT(X)            \
		if (__builtin_expect(!(X), 0)) \
			::spice::util::assert_failed(__FILE__, __LINE__, #X);
#endif

namespace spice::util {
void assert_failed(char const* file, int const line, char const* condition);
}