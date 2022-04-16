#include "spice/snn.h"

#include "matplot.h"

using namespace spice;
using namespace spice::util;

std::vector<Int32> spikes[] = {{4, 5, 8}, {5}, {7, 8}, {5, 7}};

struct input {
	Int i = 0;

	void update(float, auto, std::vector<Int32>& out_spikes) {
		out_spikes.insert(out_spikes.end(), spikes[i].begin(), spikes[i].end());
		i = (i + 1) % 4;
	}
};

int main() {
	snn single_pop(1, 1, {1337});
	auto I = single_pop.add_population<input>(9);

	spike_output_stream s("external_input");
	for (Int i : range(20)) {
		single_pop.step();
		s << I << '\n';
		pause(0.1);
		(void)i;
	}
	return 0;
}