#include "gtest/gtest.h"

#include "spice/detail/random.h"
#include "spice/detail/range.h"

using namespace spice;
using namespace spice::detail;

TEST(Detail, Random) {
	constexpr Int N = 1000000;
	auto const seed = std::random_device()();
	xoroshiro32_128p rng({seed});
	Int sum      = 0;
	double meanl = 0.0;
	double meanr = 0.0;
	for (int i : range(N)) {
		float const randl = uniform_left_inc(rng);
		float const randr = uniform_right_inc(rng);
		ASSERT_LT(randl, 1.0f) << "seed: " << seed;
		ASSERT_GT(randr, 0.0f) << "seed: " << seed;

		meanl += randl;
		meanr += randr;

		sum += rng();
	}

	EXPECT_TRUE(std::abs(meanl / N - 0.5) < 1e-3) << "Test depends on rng. May fail sporadically.";
	EXPECT_TRUE(std::abs(meanr / N - 0.5) < 1e-3) << "Test depends on rng. May fail sporadically.";
	EXPECT_TRUE(std::abs(sum / std::numeric_limits<UInt32>::max() - N / 2) < 1000)
	    << "Test depends on rng. May fail sporadically";
}