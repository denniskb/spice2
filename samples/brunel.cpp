#include "matplot.h"

#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

struct poisson {
	bool update(float dt, auto& rng) const {
		float const firing_rate = 20; // Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};
static_assert(CheckNeuron<poisson>());

struct lif {
	struct neuron {
		float V   = 0;
		int Twait = 0;
	};

	bool update(neuron& n, float dt, auto) const {
		float const TmemInv = 1.0 / 0.02; // s
		float const Vrest   = 0.0;        // v
		int const Tref      = 20;         // dt
		float const Vthres  = 0.02;       // v

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
static_assert(CheckNeuron<lif>());

struct fixed_weight {
	float weight;
	void deliver(lif::neuron& to) const { to.V += weight; }
};
static_assert(CheckSynapse<fixed_weight>());

int main() {
	int const N       = 20000;
	float const dt    = 1e-4;
	float const delay = 15e-4;

	snn brunel(dt, delay, {1337});
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	brunel.connect<fixed_weight>(P, E, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(P, I, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(E, E, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(E, I, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(I, E, fixed_probability(0.1), delay, {-10.0 / N});
	brunel.connect<fixed_weight>(I, I, fixed_probability(0.1), delay, {-10.0 / N});

	spike_output_stream s("Brunel");
	for (Int i : range(300)) {
		brunel.step();

		s << I << E << P << '\n';

		pause(0.05);
		(void)i;
	}
	return 0;
}