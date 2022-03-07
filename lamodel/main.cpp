#include <algorithm>

#include "matplot.h"

#include "spice/neuron_population.h"
#include "spice/synapse_population.h"
#include "spice/util/random.h"

using namespace spice;
using namespace spice::util;
using namespace matplot;

int const N    = 10000;
int const d    = 15;
float const DT = 1e-4;

struct poisson {
	static bool update(float const dt) {
		static xoroshiro64_128p rng({1337});
		static uniform_real_distribution<double> iid;

		double const firing_rate = 20; //Hz
		return iid(rng) < (firing_rate * dt);
	}
};

struct lif {
	float V;
	int Twait;

	lif() : V(0), Twait(0) {}

	bool update(float const dt) {
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
	static void deliver(lif& to) { to.V += (0.0001f * 20'000) / N; }
};

struct SynI {
	static void deliver(lif& to) { to.V -= (0.0005f * 20'000) / N; }
};

int main() {
	neuron_population<poisson> P(N / 2, d);
	neuron_population<lif> E(N * 4 / 10, d);
	neuron_population<lif> I(N / 10, d);

	synapse_population<SynE, lif> PE(P.size(), E.size(), 0.1, {1212321});
	synapse_population<SynE, lif> PI(P.size(), I.size(), 0.1, {8304294});
	synapse_population<SynE, lif> EE(E.size(), E.size(), 0.1, {2141241});
	synapse_population<SynE, lif> EI(E.size(), I.size(), 0.1, {419240124});
	synapse_population<SynI, lif> IE(I.size(), E.size(), 0.1, {912412421});
	synapse_population<SynI, lif> II(I.size(), I.size(), 0.1, {41092312});

	std::vector<double> x(N);
	std::vector<double> y(N);
	figure();
	xlim({0, 100});
	ylim({0, 100});
	for (Int i : range(200)) {
		if (i >= d) {
			PI.deliver(P.spikes(14), I.neurons());
			PE.deliver(P.spikes(14), E.neurons());
			EE.deliver(E.spikes(14), E.neurons());
			EI.deliver(E.spikes(14), I.neurons());
			II.deliver(I.spikes(14), I.neurons());
			IE.deliver(I.spikes(14), E.neurons());
		}

		P.update(d, DT);
		E.update(d, DT);
		I.update(d, DT);

		x.clear();
		y.clear();
		for (auto s : P.spikes(0)) {
			x.push_back(s % 100);
			y.push_back(s / 100);
		}
		for (auto s : E.spikes(0)) {
			x.push_back((s + N / 2) % 100);
			y.push_back((s + N / 2) / 100);
		}
		for (auto s : I.spikes(0)) {
			x.push_back((s + 9 * N / 10) % 100);
			y.push_back((s + 9 * N / 10) / 100);
		}
		scatter(x, y);
		pause(0.05);

		(void)i;
	}
	return 0;
}