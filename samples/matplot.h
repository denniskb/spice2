#pragma once

#include <memory>
#include <string>

#include "spice/detail/neuron_population.h"

#ifdef SPICE_USE_MATPLOT
	#pragma GCC system_header
	#include "matplot/matplot.h"
#endif

void pause(double s);

class spike_output_stream {
public:
	explicit spike_output_stream(std::string const& model_name, bool const skip_steps_without_spikes = false);
	~spike_output_stream();

	spike_output_stream& operator<<(spice::detail::NeuronPopulation const* population);
	spike_output_stream& operator<<(char const c);

private:
	struct impl;
	std::unique_ptr<impl> _impl;
};