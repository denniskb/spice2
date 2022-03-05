#include <algorithm>

#include "matplot.h"

#include "spice/adj_list.h"
#include "spice/neuron_pool.h"
#include "spice/util/random.h"

using namespace spice;
using namespace spice::util;
using namespace matplot;

int const N    = 10000;
int const d    = 15;
float const dt = 1e-4;

struct lif {
	float V;
	int Twait;

	lif() : V(0), Twait(0) {}

	bool update() {
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

int main() {
	neuron_pool<void> poisson(N / 2, 1, [] {
		static xoroshiro32_128p rng({1337});
		static uniform_real_distribution<float> iid;

		float const firing_rate = 20; //Hz
		return iid(rng) < (firing_rate * dt);
	});
	neuron_pool<lif> excitatory(N * 4 / 10, d);
	neuron_pool<lif> inhibitory(N / 10, d);

	adj_list<void> PE(poisson.size(), excitatory.size(), 0.1, {1212321});
	adj_list<void> PI(poisson.size(), inhibitory.size(), 0.1, {8304294});
	adj_list<void> EE(excitatory.size(), excitatory.size(), 0.1, {2141241});
	adj_list<void> EI(excitatory.size(), inhibitory.size(), 0.1, {419240124});
	adj_list<void> IE(inhibitory.size(), excitatory.size(), 0.1, {912412421});
	adj_list<void> II(inhibitory.size(), inhibitory.size(), 0.1, {41092312});

	auto deliver_from_excitatory = [](lif& to) { to.V += (0.0001f * 20'000) / N; };
	auto deliver_from_inhibitory = [](lif& to) { to.V -= (0.0005f * 20'000) / N; };

	std::vector<double> x(N);
	std::vector<double> y(N);
	figure();
	xlim({0, 100});
	ylim({0, 100});
	for (Int i : range(100)) {
		if (i >= d) {
			PI.deliver<lif>(poisson.spikes(14), inhibitory.neurons(), deliver_from_excitatory);
			PE.deliver<lif>(poisson.spikes(14), excitatory.neurons(), deliver_from_excitatory);
			EE.deliver<lif>(excitatory.spikes(14), excitatory.neurons(), deliver_from_excitatory);
			EI.deliver<lif>(excitatory.spikes(14), inhibitory.neurons(), deliver_from_excitatory);
			II.deliver<lif>(inhibitory.spikes(14), inhibitory.neurons(), deliver_from_inhibitory);
			IE.deliver<lif>(inhibitory.spikes(14), excitatory.neurons(), deliver_from_inhibitory);
		}

		poisson.update(d);
		excitatory.update(d);
		inhibitory.update(d);

		x.clear();
		y.clear();
		for (auto s : poisson.spikes(0)) {
			x.push_back(s % 100);
			y.push_back(s / 100);
		}
		for (auto s : excitatory.spikes(0)) {
			x.push_back((s + N / 2) % 100);
			y.push_back((s + N / 2) / 100);
		}
		for (auto s : inhibitory.spikes(0)) {
			x.push_back((s + 9 * N / 10) % 100);
			y.push_back((s + 9 * N / 10) / 100);
		}
		scatter(x, y);
		pause(0.1);

		(void)i;
	}
	return 0;
}