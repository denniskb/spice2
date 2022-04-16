#include "matplot.h"

#ifdef SPICE_USE_MATPLOT

	#include "matplot/matplot.h"

	#include <thread>

using namespace matplot;

void pause(double s) { std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(s * 1000))); }

static void scatter_spikes(spice::detail::NeuronPopulation const* pop, Int const width, Int const offset) {
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

struct spike_output_stream::impl {
	bool skip = false;

	figure_handle figure;
	std::vector<spice::detail::NeuronPopulation const*> pops;
};

spike_output_stream::spike_output_stream(std::string const& model_name, bool skip) : _impl(new impl()) {
	_impl->skip = skip;

	_impl->figure = figure(true);
	_impl->figure->name(model_name);
	hold(true);
}

spike_output_stream::~spike_output_stream() = default;

spike_output_stream& spike_output_stream::operator<<(spice::detail::NeuronPopulation const* pop) {
	_impl->pops.push_back(pop);
	return *this;
}

spike_output_stream& spike_output_stream::operator<<(char const c) {
	if (c == '\n') {
		Int N = 0;
		Int S = 0;
		for (auto pop : _impl->pops) {
			N += pop->size();
			S += pop->spikes(0).size();
		}

		if (!_impl->skip || S > 0) {
			Int const width = std::ceil(std::sqrt(N));

			figure(_impl->figure);
			cla();
			xlim({0, double(width)});
			ylim({0, double(width)});

			Int offset = 0;
			for (auto pop : _impl->pops) {
				scatter_spikes(pop, width, offset);
				offset += pop->size();
			}

			_impl->figure->draw();
		}

		_impl->pops.clear();
	}

	return *this;
}

#else

	#include <iostream>

void pause(double) {}

struct spike_output_stream::impl {
	bool skip         = false;
	Int offset        = 0;
	std::string delim = "\t\t[";
};

spike_output_stream::spike_output_stream(std::string const& model_name, bool skip) : _impl(new impl()) {
	_impl->skip = skip;

	std::cout << "{\n";
	std::cout << "\t\"name\": \"" << model_name << "\",\n";
	std::cout << "\t\"spikes\": [\n";
}

spike_output_stream::~spike_output_stream() { std::cout << "\n\t]\n}\n"; }

spike_output_stream& spike_output_stream::operator<<(spice::detail::NeuronPopulation const* pop) {
	if (!_impl->skip || pop->spikes(0).size() > 0) {
		for (auto s : pop->spikes(0)) {
			std::cout << _impl->delim << s + _impl->offset;
			_impl->delim = ",";
		}
		_impl->offset += pop->size();
	}

	return *this;
}

spike_output_stream& spike_output_stream::operator<<(char const c) {
	if (c == '\n') {
		if (_impl->offset > 0) {
			_impl->delim = ",\n\t\t[";
			std::cout << "]";
		}
		_impl->offset = 0;
	}
	return *this;
}

#endif