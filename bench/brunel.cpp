#include "benchmark/benchmark.h"

#include "spice/snn.h"
#include "spice/util/random.h"

using namespace spice;
using namespace spice::util;

struct poisson {
	static bool update(float const dt, util::xoroshiro64_128p& rng) {
		float const firing_rate = 20; //Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};

struct lif {
	float V   = 0;
	int Twait = 0;

	bool update(float const dt, util::xoroshiro64_128p&) {
		float const TmemInv = 1.0 / 0.02; // s
		float const Vrest   = 0.0;        // v
		int const Tref      = 20;         // dt
		float const Vthres  = 0.02f;      // v

		if (--Twait <= 0) {
			if (V > Vthres) {
				V     = Vrest;
				Twait = Tref;
				return true;
			}

			V += (Vrest - V) * (dt * TmemInv);
		}
		return false;
	}
};

struct SynE {
	static void deliver(lif& to, Int n) { to.V += (0.0001f * 20'000) / n; }
};

struct SynI {
	static void deliver(lif& to, Int n) { to.V -= (0.0005f * 20'000) / n; }
};

static void brunel(benchmark::State& state) {
	int const N       = 20000;
	float const delay = 15e-4;

	snn brunel(/*dt*/ 1e-4, delay, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<SynE>(P, E, fixed_probability(0.1), delay, N);
	brunel.connect<SynE>(P, I, fixed_probability(0.1), delay, N);
	brunel.connect<SynE>(E, E, fixed_probability(0.1), delay, N);
	brunel.connect<SynE>(E, I, fixed_probability(0.1), delay, N);
	brunel.connect<SynI>(I, E, fixed_probability(0.1), delay, N);
	brunel.connect<SynI>(I, I, fixed_probability(0.1), delay, N);

	for (auto _ : state) {
		brunel.step();
	}
}
BENCHMARK(brunel)->Unit(benchmark::kMicrosecond);