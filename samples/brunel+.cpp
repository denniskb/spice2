#include <iostream>

#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

struct poisson {
	static bool update(float const dt, snn_info& info) {
		float const firing_rate = 20; //Hz
		return util::generate_canonical<float>(info.rng) < (firing_rate * dt);
	}
};

struct lif {
	float V   = 0;
	int Twait = 0;

	bool update(float const dt, snn_info&) {
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
	static void deliver(lif& to, snn_info& info) { to.V += (0.0001f * 20'000) / info.N; }
};

struct SynI {
	static void deliver(lif& to, snn_info& info) { to.V -= (0.0005f * 20'000) / info.N; }
};

struct SynPlast {
	float W     = 0.0001f;
	float Zpre  = 0;
	float Zpost = 0;

	void deliver(lif& to, snn_info&) { to.V += W; }
	void update(float const dt, bool const pre, bool const post, snn_info&) {
		float const TstdpInv = 1.0f / 0.02f;
		float const dtInv    = 1.0f / dt;

		W = std::clamp(W - pre * 0.0202f * W * std::exp(-Zpost * dtInv) +
		                   post * 0.01f * (1.0f - W) * std::exp(-Zpre * dtInv),
		               0.0f, 0.0003f);

		Zpre += pre;
		Zpost += post;

		Zpre -= Zpre * dt * TstdpInv;
		Zpost -= Zpost * dt * TstdpInv;
	}
	void skip(float const dt, Int const n, snn_info&) {
		float const TstdpInv = 1.0f / 0.02f;

		Zpre *= std::pow(1 - dt * TstdpInv, n);
		Zpost *= std::pow(1 - dt * TstdpInv, n);
	}
};

int main() {
	using namespace matplot;

	int const N       = 20000;
	float const delay = 15e-4;

	snn brunel(/*dt*/ 1e-4, delay, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<SynE>(P, E, fixed_probability(0.1), delay);
	brunel.connect<SynE>(P, I, fixed_probability(0.1), delay);
	brunel.connect<SynPlast>(E, E, fixed_probability(0.1), delay);
	brunel.connect<SynE>(E, I, fixed_probability(0.1), delay);
	brunel.connect<SynI>(I, E, fixed_probability(0.1), delay);
	brunel.connect<SynI>(I, I, fixed_probability(0.1), delay);

	for (Int i : range(300)) {
		brunel.step();

		scatter_spikes({I, E, P});
		pause(0.05);
		(void)i;
	}
	return 0;
}