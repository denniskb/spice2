#include "benchmark/benchmark.h"

#include <thread>

static void test(benchmark::State& state) {
	// Perform setup here
	for (auto _ : state) {
		// This code gets timed
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
  	}
}
// Register the function as a benchmark
BENCHMARK(test);