#include <gtest/gtest.h>

#include "spice/util/meta.h"

using namespace spice::util;

TEST(Meta, Invoke) {
	invoke(false, []<bool B>() { ASSERT_TRUE(!B); });
	invoke(true, []<bool B>() { ASSERT_TRUE(B); });

	invoke(false, false, []<bool A, bool B>() { ASSERT_TRUE(!A && !B); });
	invoke(false, true, []<bool A, bool B>() { ASSERT_TRUE(!A && B); });
	invoke(true, false, []<bool A, bool B>() { ASSERT_TRUE(A && !B); });
	invoke(true, true, []<bool A, bool B>() { ASSERT_TRUE(A && B); });
}