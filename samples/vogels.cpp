#include <algorithm>

#include "matplot.h"

#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

struct lif {
	float V;
	float Gex;
	float Gin;
	Int32 Twait;

	lif() : V(-0.06), Gex(0), Gin(0), Twait(0) {}

	bool update(float const dt, util::xoroshiro64_128p&) {
		Int32 const Tref    = 50;           // dt
		float const Vrest   = -0.06f;       // v
		float const Vthres  = -0.05f;       // v
		float const TmemInv = 1.0f / 0.02f; // s
		float const Eex     = 0.0f;         // v
		float const Ein     = -0.08f;       // v
		float const Ibg     = 0.02f;        // v

		float const TexInv = 1.0f / 0.005f; // s
		float const TinInv = 1.0f / 0.01f;  // s

		bool spiked = false;
		if (--Twait <= 0) {
			if (V > Vthres) {
				V      = Vrest;
				Twait  = Tref;
				spiked = true;
			} else
				V += ((Vrest - V) + Gex * (Eex - V) + Gin * (Ein - V) + Ibg) * (dt * TmemInv);
		}

		Gex -= Gex * (dt * TexInv);
		Gin -= Gin * (dt * TinInv);

		return spiked;
	}
};

struct SynE {
	static void deliver(lif& to, Int const N) {
		float const W = (0.4f * 16'000'000) / (N * N); // siemens
		to.Gex += W;
	}
};

struct SynI {
	static void deliver(lif& to, Int const N) {
		float const W = (5.1f * 16'000'000) / (N * N); // siemens
		to.Gin += W;
	}
};

int main() {
	using namespace matplot;

	int const N     = 4000;
	int const delay = 8;
	float const DT  = 1e-4;

	snn vogels(DT, delay, {1337});
	auto E = vogels.add_population<lif>(N * 8 / 10);
	auto I = vogels.add_population<lif>(N * 2 / 10);

	vogels.connect<SynE>(E, E, fixed_probability(0.02), N);
	vogels.connect<SynE>(E, I, fixed_probability(0.02), N);
	vogels.connect<SynI>(I, E, fixed_probability(0.02), N);
	vogels.connect<SynI>(I, I, fixed_probability(0.02), N);

	for (Int i : range(1500)) {
		vogels.step();

		scatter_spikes({I, E}, true);

		(void)i;
	}
	return 0;
}