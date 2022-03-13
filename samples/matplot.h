#pragma once

#pragma GCC system_header
#include "matplot/matplot.h"

#include "spice/detail/neuron_population.h"
#include "spice/snn.h"

namespace matplot {
void pause(double s);

void scatter_spikes(std::initializer_list<spice::detail::NeuronPopulation const*> neuron_population_list,
                    bool skip_simulation_steps_without_spikes = false);
}