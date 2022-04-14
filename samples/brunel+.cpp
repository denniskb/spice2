#include <iostream>

#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

struct poisson {
	bool update(float const dt, auto& rng) const {
		float const firing_rate = 20; //Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};

struct lif {
	struct neuron {
		float V   = 0;
		int Twait = 0;
	};

	bool update(neuron& n, float const dt, auto&) const {
		float const TmemInv = 1.0 / 0.02; // s
		float const Vrest   = 0.0;        // v
		int const Tref      = 20;         // dt
		float const Vthres  = 0.02f;      // v

		if (--n.Twait <= 0) {
			if (n.V > Vthres) {
				n.V     = Vrest;
				n.Twait = Tref;
				return true;
			}

			n.V += (Vrest - n.V) * (dt * TmemInv);
		}
		return false;
	}
};

struct SynE {
	Int N;

	void deliver(lif::neuron& to) const { to.V += (0.0001f * 20'000) / N; }
};

struct SynI {
	Int N;

	void deliver(lif::neuron& to) const { to.V -= (0.0005f * 20'000) / N; }
};

struct SynPlast {
	struct synapse {
		float W     = 0.0001f;
		float Zpre  = 0;
		float Zpost = 0;
	};

	void deliver(synapse const& syn, lif::neuron& to) const { to.V += syn.W; }
	void update(synapse& syn, float const dt, bool const pre, bool const post) const {
		float const TstdpInv = 1.0f / 0.02f;
		float const dtInv    = 1.0f / dt;

		syn.W = std::clamp(syn.W - pre * 0.0202f * syn.W * std::exp(-syn.Zpost * dtInv) +
		                       post * 0.01f * (1.0f - syn.W) * std::exp(-syn.Zpre * dtInv),
		                   0.0f, 0.0003f);

		syn.Zpre += pre;
		syn.Zpost += post;

		syn.Zpre -= syn.Zpre * dt * TstdpInv;
		syn.Zpost -= syn.Zpost * dt * TstdpInv;
	}
	void skip(synapse& syn, float const dt, Int const n) const {
		float const TstdpInv = 1.0f / 0.02f;

		syn.Zpre *= std::pow(1 - dt * TstdpInv, n);
		syn.Zpost *= std::pow(1 - dt * TstdpInv, n);
	}
};
static_assert(PlasticSynapse<SynPlast, lif>);

int main() {
	int const N       = 20000;
	float const delay = 15e-4;

	snn brunel(/*dt*/ 1e-4, delay, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<SynE>(P, E, fixed_probability(0.1), delay, {N});
	brunel.connect<SynE>(P, I, fixed_probability(0.1), delay, {N});
	brunel.connect<SynPlast>(E, E, fixed_probability(0.1), delay);
	brunel.connect<SynE>(E, I, fixed_probability(0.1), delay, {N});
	brunel.connect<SynI>(I, E, fixed_probability(0.1), delay, {N});
	brunel.connect<SynI>(I, I, fixed_probability(0.1), delay, {N});

	spike_output_stream s("Brunel+");
	for (Int i : range(300)) {
		brunel.step();

		s << I << E << P << '\n';
		pause(0.05);
		(void)i;
	}
	return 0;
}