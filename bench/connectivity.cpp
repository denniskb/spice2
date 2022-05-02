#include "benchmark/benchmark.h"

#include "spice/topology.h"
#include "spice/util/range.h"

using namespace spice;

static void adjlist(benchmark::State& state) {
	adj_list adj;
	for (Int i : util::range(10'000'000))
		adj.connect(rand() % 10'000, i), (void)i;

	adj(10'000, std::numeric_limits<Int32>::max() - 1);

	std::vector<Int> offsets(adj.src_count + 1);
	std::vector<Int32> neighbors(adj.size());

	for (auto _ : state) {
		static_cast<Topology&>(adj).generate(offsets, neighbors, {1337});
	}
}
BENCHMARK(adjlist)->Unit(benchmark::kMillisecond);

static void fixedprob(benchmark::State& state) {
	fixed_probability fprob(0.1);
	fprob(10'000, 10'000);

	std::vector<Int> offsets(fprob.src_count + 1);
	std::vector<Int32> neighbors(fprob.size());

	for (auto _ : state) {
		fprob.generate(offsets, neighbors, {1337});
	}
}
BENCHMARK(fixedprob)->Unit(benchmark::kMillisecond);