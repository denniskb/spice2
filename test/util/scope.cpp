#include "gtest/gtest.h"

#include "spice/util/scope.h"
#include "spice/util/stdint.h"

using namespace spice;
using namespace spice::util;

TEST(Detail, ScopeExit) {
	Int i = 7;
	{
		scope_exit _([&i] { i = 42; });
		ASSERT_EQ(i, 7);
	}
	ASSERT_EQ(i, 42);
}