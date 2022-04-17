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
		Int32 const Tref    = 50;          // dt
		float const Vrest   = -0.06;       // v
		float const Vthres  = -0.05;       // v
		float const TmemInv = 1.0f / 0.02; // s
		float const Eex     = 0.0;         // v
		float const Ein     = -0.08;       // v
		float const Ibg     = 0.02;        // v

		float const TexInv = 1.0f / 0.005; // s
		float const TinInv = 1.0f / 0.01;  // s

		bool spiked = false;
		if (--n.Twait <= 0) {
			if (n.V > Vthres) {
				n.V     = Vrest;
				n.Twait = Tref;
				spiked  = true;
			} else
				n.V += ((Vrest - n.V) + n.Gex * (Eex - n.V) + n.Gin * (Ein - n.V) + Ibg) *
				       (dt * TmemInv);
		}

		n.Gex -= n.Gex * (dt * TexInv);
		n.Gin -= n.Gin * (dt * TinInv);

		return spiked;
	}
};

struct excitatory {
	float weight;
	void deliver(lif::neuron& to) const { to.Gex += weight; }
};

struct inhibitory {
	float weight;
	void deliver(lif::neuron& to) const { to.Gin += weight; }
};

int main() {
	int const N       = 4000;
	float const dt    = 1e-4;
	float const delay = 8e-4;

	snn vogels(dt, delay, {1337});
	auto E = vogels.add_population<lif>(N * 8 / 10);
	auto I = vogels.add_population<lif>(N * 2 / 10);

	vogels.connect<excitatory>(E, E, fixed_probability(0.02), delay, {6.4e6 / (N * N)});
	vogels.connect<excitatory>(E, I, fixed_probability(0.02), delay, {6.4e6 / (N * N)});
	vogels.connect<inhibitory>(I, E, fixed_probability(0.02), delay, {8.16e7 / (N * N)});
	vogels.connect<inhibitory>(I, I, fixed_probability(0.02), delay, {8.16e7 / (N * N)});

	spike_output_stream s("Vogels", true);
	for (Int i : range(1500)) {
		vogels.step();

		s << I << E << '\n';

		(void)i;
	}
	return 0;
}