#include "benchmark/benchmark.h"

#include "spice/connectivity.h"
#include "spice/util/range.h"

using namespace spice;

static void adjlist(benchmark::State& state) {
	adj_list adj;
	for (Int i : util::range(10'000'000))
		adj.connect(rand() % 10'000, i), (void)i;

	adj(10'000, std::numeric_limits<Int32>::max() - 1);

	std::vector<Int> offsets(adj.src_count + 1);
	std::vector<Int32> neighbors(adj.size());
	edge_stream es(offsets, neighbors);

	for (auto _ : state) {
		adj.generate(es, {1337});
		es.flush();
	}
}
BENCHMARK(adjlist)->Unit(benchmark::kMillisecond);

static void fixedprob(benchmark::State& state) {
	fixed_probability fprob(0.1);
	fprob(10'000, 10'000);

	std::vector<Int> offsets(fprob.src_count + 1);
	std::vector<Int32> neighbors(fprob.size());
	edge_stream es(offsets, neighbors);

	for (auto _ : state) {
		fprob.generate(es, {1337});
		es.flush();
	}
}
BENCHMARK(fixedprob)->Unit(benchmark::kMillisecond);