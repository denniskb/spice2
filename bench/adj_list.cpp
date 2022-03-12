#include "benchmark/benchmark.h"

#include "spice/adj_list.h"
#include "spice/util/range.h"

using namespace spice;

static void adjlist(benchmark::State& state) {
	adj_list adj;
	for (Int i : util::range(1000'000))
		adj.connect(rand() % 1000, rand()), (void)i;

	std::vector<UInt> offsets(adj.src_count() + 1);
	std::vector<Int32> neighbors(adj.size());

	for (auto _ : state) {
		adj.fill_csr(offsets, neighbors);
	}
}
BENCHMARK(adjlist)->Unit(benchmark::kMillisecond);