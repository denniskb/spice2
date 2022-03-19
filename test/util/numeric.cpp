#include <gtest/gtest.h>

#include "spice/util/numeric.h"
#include "spice/util/range.h"
#include "spice/util/stdint.h"

using namespace spice::util;

TEST(KahanSum, Add) {
	Int const ITER    = 1000;
	float const DELTA = 0.001f;

	float sum = 0;
	kahan_sum<float> ksum;
	float deltas = 0;
	for (Int i : range(ITER)) {
		ASSERT_FLOAT_EQ(ksum, i * DELTA);

		sum += DELTA;
		float delta = ksum += DELTA;
		deltas += delta;

		ASSERT_NEAR(delta, DELTA, DELTA / 100);
	}

	ASSERT_NE(sum, 1);
	ASSERT_EQ(ksum, 1);
	ASSERT_EQ(deltas, 1);
}