#pragma once

#ifdef SPICE_ASSERT_PRECONDITIONS
	#define SPICE_PRE(X) SPICE_ASSERT(X)
#else
	#define SPICE_PRE(X) (void)(X);
#endif

#ifdef SPICE_ASSERT_INVARIANTS
	#define SPICE_INV(X) SPICE_ASSERT(X)
#else
	#define SPICE_INV(X) (void)(X);
#endif

#define SPICE_ASSERT(X)            \
	if (__builtin_expect(!(X), 0)) \
		::spice::util::detail::assert_failed(__FILE__, __LINE__, #X);

namespace spice::util::detail {
void assert_failed(char const* file, int const line, char const* condition);
}