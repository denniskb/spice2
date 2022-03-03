#include "gtest/gtest.h"

#include <numbers>

#include "spice/util/random.h"
#include "spice/util/range.h"

using namespace spice;
using namespace spice::util;

static constexpr Int n_choose_k(Int const n, Int const k) {
	SPICE_ASSERT(0 <= k && k <= n);
	SPICE_ASSERT(0 <= n && n <= 20);

	constexpr Int const factorial[21] = {1,
	                                     1,
	                                     2,
	                                     6,
	                                     24,
	                                     120,
	                                     720,
	                                     5040,
	                                     40320,
	                                     362880,
	                                     3628800,
	                                     39916800,
	                                     479001600,
	                                     6227020800,
	                                     87178291200,
	                                     1307674368000,
	                                     20922789888000,
	                                     355687428096000,
	                                     6402373705728000,
	                                     121645100408832000,
	                                     2432902008176640000};

	return factorial[n] / (factorial[k] * factorial[n - k]);
}
static constexpr double bino_cdf(Int const x, Int const n, double const p) {
	double result = 0;
	for (Int i : range(std::min(n, x) + 1))
		result += n_choose_k(n, i) * std::pow(p, i) * std::pow(1 - p, n - i);

	return result;
}

template <class RNG>
void test_random_number_distribution(auto&& cdf, auto&& dist, double const a, double const b) {
	auto const seed = std::random_device()();
	RNG rng({seed});

	double kolmogorov_smirnov = 0;
	for (Int i : range(100)) {
		double const x = std::lerp(a, b, i * 0.01);
		Int is         = 0;
		for (Int j : range(1000)) {
			is += (dist(rng) <= x);
			(void)j;
		}
		kolmogorov_smirnov = std::max(kolmogorov_smirnov, std::abs(1e-3 * is - cdf(x)));
	}

	EXPECT_LE(kolmogorov_smirnov, 0.06) << "failing seed: " << seed;
}

TEST(Random, Xoroshiro32_128_UniformRealDistributionFloat) {
	test_random_number_distribution<xoroshiro32_128p>([](double x) { return 0.5 * x - 0.5; },
	                                                  uniform_real_distribution<float>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro32_128_UniformRealDistributionFloatTrue) {
	test_random_number_distribution<xoroshiro32_128p>(
	    [](double x) { return 0.5 * x - 0.5; }, uniform_real_distribution<float, true>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro32_128_UniformRealDistributionDouble) {
	test_random_number_distribution<xoroshiro32_128p>(
	    [](double x) { return 0.5 * x - 0.5; }, uniform_real_distribution<double>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro32_128_UniformRealDistributionDoubleTrue) {
	test_random_number_distribution<xoroshiro32_128p>([](double x) { return 0.5 * x - 0.5; },
	                                                  uniform_real_distribution<double, true>(1, 3),
	                                                  1, 3);
}
TEST(Random, Xoroshiro64_128_UniformRealDistributionFloat) {
	test_random_number_distribution<xoroshiro64_128p>([](double x) { return 0.5 * x - 0.5; },
	                                                  uniform_real_distribution<float>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro64_128_UniformRealDistributionFloatTrue) {
	test_random_number_distribution<xoroshiro64_128p>(
	    [](double x) { return 0.5 * x - 0.5; }, uniform_real_distribution<float, true>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro64_128_UniformRealDistributionDouble) {
	test_random_number_distribution<xoroshiro64_128p>(
	    [](double x) { return 0.5 * x - 0.5; }, uniform_real_distribution<double>(1, 3), 1, 3);
}
TEST(Random, Xoroshiro64_128_UniformRealDistributionDoubleTrue) {
	test_random_number_distribution<xoroshiro64_128p>([](double x) { return 0.5 * x - 0.5; },
	                                                  uniform_real_distribution<double, true>(1, 3),
	                                                  1, 3);
}

TEST(Random, ExponentialDistribution) {
	test_random_number_distribution<xoroshiro64_128p>([](double x) { return 1 - std::exp(-5 * x); },
	                                                  exponential_distribution<double>(0.2), 0, 1);
}

TEST(Random, NormalDistribution) {
	test_random_number_distribution<xoroshiro64_128p>(
	    [](double x) { return 0.5 * (1 + std::erf((x + 5) / (3 * std::sqrt(2)))); },
	    normal_distribution<double>(-5, 3), -15, 5);
}

TEST(Random, BinomialDistribution) {
	test_random_number_distribution<xoroshiro64_128p>([](double x) { return bino_cdf(x, 10, 0.7); },
	                                                  binomial_distribution<Int>(10, 0.7), 0, 14);
}