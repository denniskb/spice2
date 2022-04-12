#include <algorithm>

#include "matplot.h"

#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

struct lif {
	struct neuron {
		float V     = -0.06;
		float Gex   = 0;
		float Gin   = 0;
		Int32 Twait = 0;
	};

	bool update(neuron& n, float const dt, auto&) const {
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
		if (--n.Twait <= 0) {
			if (n.V > Vthres) {
				n.V     = Vrest;
				n.Twait = Tref;
				spiked  = true;
			} else
				n.V += ((Vrest - n.V) + n.Gex * (Eex - n.V) + n.Gin * (Ein - n.V) + Ibg) * (dt * TmemInv);
		}

		n.Gex -= n.Gex * (dt * TexInv);
		n.Gin -= n.Gin * (dt * TinInv);

		return spiked;
	}
};

struct SynE {
	Int N2;

	void deliver(lif::neuron& to) const {
		float const W = 0.4f * (16'000'000.0 / N2); // siemens
		to.Gex += W;
	}
};

struct SynI {
	Int N2;

	void deliver(lif::neuron& to) const {
		float const W = 5.1f * (16'000'000.0 / N2); // siemens
		to.Gin += W;
	}
};

int main() {
	using namespace matplot;

	int const N       = 4000;
	float const delay = 8e-4;

	snn vogels(/*dt*/ 1e-4, delay, {1337});
	auto E = vogels.add_population<lif>(N * 8 / 10);
	auto I = vogels.add_population<lif>(N * 2 / 10);

	vogels.connect<SynE>(E, E, fixed_probability(0.02), delay, {N * N});
	vogels.connect<SynE>(E, I, fixed_probability(0.02), delay, {N * N});
	vogels.connect<SynI>(I, E, fixed_probability(0.02), delay, {N * N});
	vogels.connect<SynI>(I, I, fixed_probability(0.02), delay, {N * N});

	for (Int i : range(1500)) {
		vogels.step();

		scatter_spikes({I, E}, true);

		(void)i;
	}
	return 0;
}