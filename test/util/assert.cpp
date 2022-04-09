#include <gtest/gtest.h>

#include "spice/util/assert.h"

using namespace spice::util::detail;

TEST(Assert, Assert) {
	ASSERT_THROW(assert_failed("", 0, ""), std::logic_error);

#ifdef SPICE_ASSERT_PRECONDITIONS
	ASSERT_NO_THROW(SPICE_PRE(true));
	ASSERT_THROW(SPICE_PRE(false), std::logic_error);
#endif

#ifdef SPICE_ASSERT_INVARIANTS
	ASSERT_NO_THROW(SPICE_INV(true));
	ASSERT_THROW(SPICE_INV(false), std::logic_error);
#endif
}