#include "benchmark/benchmark.h"

#include "spice/neuron_population.h"
#include "spice/synapse_population.h"
#include "spice/util/random.h"

using namespace spice;
using namespace spice::util;

int const N     = 20000;
int const d     = 15;
double const DT = 1e-4;

struct poisson {
	static bool update(double dt) {
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

	bool update(double dt) {
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

static void brunel(benchmark::State& state) {
	neuron_population<poisson> P(N / 2, d);
	neuron_population<lif> E(N * 4 / 10, d);
	neuron_population<lif> I(N / 10, d);

	synapse_population<SynE, lif> PE(P.size(), E.size(), 0.1, {1212321});
	synapse_population<SynE, lif> PI(P.size(), I.size(), 0.1, {8304294});
	synapse_population<SynE, lif> EE(E.size(), E.size(), 0.1, {2141241});
	synapse_population<SynE, lif> EI(E.size(), I.size(), 0.1, {419240124});
	synapse_population<SynI, lif> IE(I.size(), E.size(), 0.1, {912412421});
	synapse_population<SynI, lif> II(I.size(), I.size(), 0.1, {41092312});

	auto deliver_from_excitatory = [](lif& to) { to.V += (0.0001f * 20'000) / N; };
	auto deliver_from_inhibitory = [](lif& to) { to.V -= (0.0005f * 20'000) / N; };

	Int i = 0;
	for (auto _ : state) {
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
		i++;
	}
}
BENCHMARK(brunel)->Unit(benchmark::kMicrosecond);