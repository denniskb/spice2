#include "benchmark/benchmark.h"

#include "spice/connectivity.h"
#include "spice/util/range.h"

using namespace spice;

static void adjlist(benchmark::State& state) {
	adj_list adj;
	for (Int i : util::range(10'000'000))
		adj.connect(rand() % 10'000, i), (void)i;

	std::vector<UInt> offsets(adj.src_count() + 1);
	std::vector<Int32> neighbors(adj.size());

	for (auto _ : state) {
		adj.fill_csr(offsets, neighbors, {});
	}
}
BENCHMARK(adjlist)->Unit(benchmark::kMillisecond);

static void fixedprob(benchmark::State& state) {
	fixed_probability fprob(10'000, 10'000, 0.1);

	std::vector<UInt> offsets(fprob.src_count() + 1);
	std::vector<Int32> neighbors(fprob.size());

	for (auto _ : state) {
		fprob.fill_csr(offsets, neighbors, {});
	}
}
BENCHMARK(fixedprob)->Unit(benchmark::kMillisecond);