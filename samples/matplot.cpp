#include "matplot.h"

#include <thread>

using namespace matplot;

static matplot::figure_handle _figure(void const* id) {
	static std::map<void const*, matplot::figure_handle> map;

	auto result = map.find(id);
	if (result != map.end())
		return matplot::figure(result->second);
	else
		return map.emplace(id, matplot::figure(true)).first->second;
}

static void _scatter_spikes(spice::detail::NeuronPopulation const* pop, Int width, Int offset) {
	static std::vector<double> x;
	static std::vector<double> y;

	x.reserve(pop->spikes(0).size());
	y.reserve(pop->spikes(0).size());

	x.clear();
	y.clear();

	for (auto s : pop->spikes(0)) {
		x.push_back((s + offset) % width);
		y.push_back((s + offset) / width);
	}

	auto l = scatter(x, y);
	l->marker_style(line_spec::marker_style::point);
}

namespace matplot {
void pause(double s) { std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(s * 1000))); }

void scatter_spikes(std::initializer_list<spice::detail::NeuronPopulation const*> pops, bool skip) {
	Int N = 0;
	Int S = 0;
	for (auto pop : pops) {
		N += pop->size();
		S += pop->spikes(0).size();
	}

	if (skip && S == 0)
		return;

	Int const width = std::ceil(std::sqrt(N));

	auto h = _figure(pops.begin());
	cla();
	hold(true);
	xlim({0, double(width)});
	ylim({0, double(width)});

	Int offset = 0;
	for (auto pop : pops) {
		_scatter_spikes(pop, width, offset);
		offset += pop->size();
	}
	h->draw();
}
}