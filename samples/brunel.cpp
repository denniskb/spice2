#include "matplot.h"

#include "spice/snn.h"

using namespace spice;
using namespace spice::util;

/*
The Brunel model consist of 3 neuron populations:
input neurons P, excitatory neurons E, and inhibitory neurons I.
They are connected in the following way:
P->E, P->I, E->E, E->I, I->E, I->I
The input neurons are modeled as Poisson neurons while the rest are
modeled as "leaky integrate and fire" (LIF) neurons.
The Brunel model uses simple, static synapses with a fixed weight to
connect all neuron populations.
*/

// First we define our neuron *types*, later we create the populations.
// We start with the poisson neuron, which is a "stateless" neuron.
// It only consists of an update() method which is invoked at every simulation step
// and fires randomly at a certain, average frequency.
struct poisson {
	// Spice provides you a properly seeded, large-period, high-quality RNG to avoid
	// duplicate runs when dealing with the massive parallelism of SNNs.
	// It's not necessary but advised to use it over self-instantiated RNGs.
	bool update(float dt, auto& rng) const {
		float const firing_rate = 20; // Hz
		return util::generate_canonical<float>(rng) < (firing_rate * dt);
	}
};
// Optionally, we can use the CheckNeuron() function to make sure we defined our neuron correctly.
static_assert(CheckNeuron<poisson>());

// Next, we define the LIF neuron. lif is a "stateful" neuron. In contrast to Poisson,
// it defines a nested type "neuron" with lif's attributes. Also, it's update() method
// takes an additional first parameter of type 'neuron&' so it can modify the neuron
// at each step.
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

// Finally, we define a simple "stateless" synapse that simply adds the same weight
// to the target neuron for the entire synapse population.
struct fixed_weight {
	float weight;
	void deliver(lif::neuron& to) const { to.V += weight; }
};
// Just as with neurons, we can check that we correctly defined our synapse
// using the CheckSynapse() function.
static_assert(CheckSynapse<fixed_weight>());

int main() {
	// Now it is time to create our snn. It will consist of 20K neurons total,
	// have a DT of 100us, and a *maximum* delay of 1.5ms
	int const N       = 20000;
	float const dt    = 1e-4;
	float const delay = 15e-4;

	// The third parameter is the master seed of the network.
	// From it, further seeds are generated for all the snn's components.
	snn brunel(dt, delay, {1337});
	// We define our 3 neuron populations by calling add_connection()
	// with the appropriate neuron types and population sizes.
	auto P = brunel.add_population<poisson>(N / 2);
	auto E = brunel.add_population<lif>(N * 4 / 10);
	auto I = brunel.add_population<lif>(N / 10);

	// Next we have to connect our neuron populations.
	// For each connection we call connect() with:
	// - the type of synapse to use for the connection
	// - the 2 neuron popualtions to connect (order matters, first is the source, then the destination)
	// - the topology
	// - the delay of *that particular connection* (must be <= the maximum delay specified in the snn)
	// - any parameters for the synapse, in this case its weight
	brunel.connect<fixed_weight>(P, E, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(P, I, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(E, E, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(E, I, fixed_probability(0.1), delay, {2.0 / N});
	brunel.connect<fixed_weight>(I, E, fixed_probability(0.1), delay, {-10.0 / N});
	brunel.connect<fixed_weight>(I, I, fixed_probability(0.1), delay, {-10.0 / N});

	spike_output_stream s("Brunel");
	for (Int i : range(300)) {
		// We run the simulation by repeatedly calling snn.step(),
		// which advances the simulation by DT each time.
		brunel.step();

		s << I << E << P << '\n';

		pause(0.05);
		(void)i;
	}
	return 0;
}