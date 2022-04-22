#include <iostream>

#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

/*
Bruenl+ is the "plastic" version of Brunel. Meaning, some of its synaptic connections
undergo state changes over time, just like neurons. To be specific, Brunel+ is identical
with Brunel, except for its E->E connection, which uses plastic synapses (please study
the Brunel sample as a primer).
*/

struct poisson {
	bool update(float const dt, auto& rng) const {
		float const firing_rate = 20; //Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};
static_assert(CheckNeuron<poisson>());

struct lif {
	struct neuron {
		float V   = 0;
		int Twait = 0;
	};

	bool update(neuron& n, float const dt, auto&) const {
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

// A plastic synapse is a stateful synapse that additionally can change its state over time,
// represented in code by the update() and skip() methods.
struct plastic {
	// Just like stateful neurons, stateful synapses must define a nested type "synapse"
	// with the synapse's attributes.
	struct synapse {
		float W     = 1e-4;
		float Zpre  = 0;
		float Zpost = 0;
	};

	// The deliver() method takes an additional first parameter of type synapse& so
	// it can use the synapse's state to update the target neuron.
	void deliver(synapse const& syn, lif::neuron& to) const { to.V += syn.W; }

	// The update() method is called at every simulation step. 'pre' and 'post' are flags
	// that indicate whether the synapse's source or target neuron fired in the respective step.
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
	// The skip() method is used to advance the synapse 'n' steps at once when there is no
	// network activity (no pre- or post-synaptic spikes).
	// Many dynamics have closed from solutions, allowing them to 'jump' ahead arbitrary
	// amounts of time in a single computational step.
	// Alternatively, you can simply call update() 'n' times inside your skip() method.
	void skip(synapse& syn, float const dt, Int const n) const {
		float const TstdpInv = 1.0f / 0.02f;

		syn.Zpre *= std::pow(1 - dt * TstdpInv, n);
		syn.Zpost *= std::pow(1 - dt * TstdpInv, n);
	}
};
static_assert(CheckSynapse<plastic>());

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
	brunel.connect<plastic>(E, E, fixed_probability(0.1), delay);
	brunel.connect<fixed_weight>(E, I, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(I, E, fixed_probability(0.1), delay, {-10.0 / N});
	brunel.connect<fixed_weight>(I, I, fixed_probability(0.1), delay, {-10.0 / N});

	spike_output_stream s("Brunel+");
	for (Int i : range(300)) {
		brunel.step();

		s << I << E << P << '\n';
		pause(0.05);
		(void)i;
	}
	return 0;
}