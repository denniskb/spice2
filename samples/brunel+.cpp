#include <algorithm>

#include "matplot.h"

#include "spice/snn.h"

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
	static void deliver(lif& to, Int const N) { to.V += (0.0001f * 20'000) / N; }
};

struct SynI {
	static void deliver(lif& to, Int const N) { to.V -= (0.0005f * 20'000) / N; }
};

struct SynPlast {
	float W     = 0.0001f;
	float Zpre  = 0;
	float Zpost = 0;

	void deliver(lif& to) { to.V += W; }
	void update(float const dt, bool const pre, bool const post, Int const n) {
		if (n) { // post() + update()
			Zpost += post;

			float const TstdpInv = 1.0f / 0.02f;
			float const dtInv    = 1.0f / dt;

			W = std::clamp(W - pre * 0.0202f * W * std::exp(-Zpost * dtInv) +
			                   post * 0.01f * (1.0f - W) * std::exp(-Zpre * dtInv),
			               0.0f, 0.0003f);

			// TODO: Decay before or after weight update?
			Zpre -= Zpre * dt * TstdpInv;
			Zpost -= Zpost * dt * TstdpInv;
		} else // pre()
			Zpre += pre;
	}
};

int main() {
	using namespace matplot;

	int const N     = 20000;
	int const delay = 15;
	float const DT  = 1e-4;

	snn brunel(DT, delay, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<SynE>(P, E, fixed_probability(0.1), N);
	brunel.connect<SynE>(P, I, fixed_probability(0.1), N);
	brunel.connect<SynPlast>(E, E, fixed_probability(0.1));
	brunel.connect<SynE>(E, I, fixed_probability(0.1), N);
	brunel.connect<SynI>(I, E, fixed_probability(0.1), N);
	brunel.connect<SynI>(I, I, fixed_probability(0.1), N);

	for (Int i : range(300)) {
		brunel.step();

		scatter_spikes({I, E, P});

		(void)i;
	}
	return 0;
}