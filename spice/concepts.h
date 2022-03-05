#pragma once

#include <concepts>

namespace spice {
template <class T>
concept Neuron = requires(T t) {
	requires std::default_initializable<T>;
	{ t.update() } -> std::same_as<bool>;
};

template <class T, class Neur>
concept Synapse = requires(T t, Neur& n) {
	t.deliver(n);
}
&&Neuron<Neur>;
}