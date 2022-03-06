#include "benchmark/benchmark.h"

#include "spice/synapse_population.h"

using namespace spice;

struct stateful_neuron {
	bool update(double) { return false; }
};
struct stateless_synapse {
	static void deliver(stateful_neuron&) {}
};

static void generate(benchmark::State& state) {
	for (auto _ : state) {
		synapse_population<stateless_synapse, stateful_neuron> pop(10'000, 10'000, 0.1);
	}
}
BENCHMARK(generate)->Unit(benchmark::kMillisecond);