#pragma once

#include <concepts>

namespace spice {
template <class T>
concept StatelessNeuron = requires {
	{ T::update() } -> std::same_as<bool>;
};

template <class T>
concept StatefulNeuron = requires(T t) {
	requires std::default_initializable<T>;
	{ t.update() } -> std::same_as<bool>;
};

template <class T>
concept Neuron = StatelessNeuron<T> || StatefulNeuron<T>;

template <class T, class Neur>
concept StatelessSynapse = requires(Neur& n) {
	requires StatefulNeuron<Neur>;
	T::deliver(n);
};

template <class T, class Neur>
concept StatefulSynapse = requires(T t, Neur& n) {
	requires StatefulNeuron<Neur>;
	requires std::default_initializable<T>;
	t.deliver(n);
};

template <class T, class Neur>
concept Synapse = StatelessSynapse<T, Neur> || StatefulSynapse<T, Neur>;
}