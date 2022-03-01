#include "gtest/gtest.h"

#include "spice/neuron_pool.h"

using namespace spice;
using namespace spice::util;

struct manual_fire_neuron {
	static bool fire;
	bool update() { return fire; }
};
bool manual_fire_neuron::fire = false;

TEST(NeuronPool, Update) {
	{ //never
		manual_fire_neuron::fire = false;
		neuron_pool<manual_fire_neuron> pool(5, 1);
		pool.update(1);
		ASSERT_EQ(pool.spikes(0).size(), 0);

		for (UInt hist : pool.history())
			ASSERT_EQ(hist, 0);
	}

	{ //always
		manual_fire_neuron::fire = true;
		neuron_pool<manual_fire_neuron> pool(5, 1);
		pool.update(1);
		ASSERT_EQ(pool.spikes(0).size(), 5);
		for (Int i : range(5))
			ASSERT_EQ(pool.spikes(0)[i], i);

		for (UInt hist : pool.history())
			ASSERT_EQ(hist, 1);
	}

	{ //mixed
		manual_fire_neuron::fire = false;
		neuron_pool<manual_fire_neuron> pool(5, 2);
		pool.update(2);
		ASSERT_EQ(pool.spikes(0).size(), 0);
		for (UInt hist : pool.history())
			ASSERT_EQ(hist, 0);

		manual_fire_neuron::fire = true;
		pool.update(2);
		ASSERT_EQ(pool.spikes(1).size(), 0);
		ASSERT_EQ(pool.spikes(0).size(), 5);
		for (Int i : range(5))
			ASSERT_EQ(pool.spikes(0)[i], i);

		for (UInt hist : pool.history())
			ASSERT_EQ(hist, 1);

		manual_fire_neuron::fire = false;
		pool.update(2);
		ASSERT_EQ(pool.spikes(1).size(), 5);
		for (Int i : range(5))
			ASSERT_EQ(pool.spikes(1)[i], i);

		ASSERT_EQ(pool.spikes(0).size(), 0);
		for (UInt hist : pool.history())
			ASSERT_EQ(hist, 2);
	}
}