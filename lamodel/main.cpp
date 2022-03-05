#include <algorithm>

#include "matplot.h"
#include "spice/util/random.h"
#include "spice/util/range.h"

using namespace spice;
using namespace spice::util;
using namespace matplot;

int main() {
	xoroshiro64_128p rng({1337});
	uniform_real_distribution<double> iid;
	std::vector<double> x(100);
	std::vector<double> y(100);

	for (Int i : range(100)) {
		for (auto& xx : x)
			xx = iid(rng);
		for (auto& yy : y)
			yy = iid(rng);

		scatter(x, y);
		pause(0.1);

		(void)i;
	}
	show();
	return 0;
}