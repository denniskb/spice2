#include <algorithm>

#include "matplot.h"

#include "spice/snn.h"

using namespace spice;
using namespace spice::util;
using namespace matplot;

struct poisson {
	static bool update(float const dt, util::xoroshiro64_128p& rng) {
		float const firing_rate = 20; //Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};

struct lif {
	float V;
	int Twait;

	lif() : V(0), Twait(0) {}

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

int main() {
	int const N    = 10000;
	int const d    = 15;
	float const DT = 1e-4;

	snn brunel(DT, d, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<SynE>(P, E, 0.1, N);
	brunel.connect<SynE>(P, I, 0.1, N);
	brunel.connect<SynE>(E, E, 0.1, N);
	brunel.connect<SynE>(E, I, 0.1, N);
	brunel.connect<SynI>(I, E, 0.1, N);
	brunel.connect<SynI>(I, I, 0.1, N);

	std::vector<double> x(N);
	std::vector<double> y(N);
	figure();
	xlim({0, 100});
	ylim({0, 100});
	for (Int i : range(200)) {
		brunel.step();

		x.clear();
		y.clear();
		for (auto s : P->spikes(0)) {
			x.push_back(s % 100);
			y.push_back(s / 100);
		}
		for (auto s : E->spikes(0)) {
			x.push_back((s + N / 2) % 100);
			y.push_back((s + N / 2) / 100);
		}
		for (auto s : I->spikes(0)) {
			x.push_back((s + 9 * N / 10) % 100);
			y.push_back((s + 9 * N / 10) / 100);
		}
		scatter(x, y);
		pause(0.05);

		(void)i;
	}
	return 0;
}