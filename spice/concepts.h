#pragma once

#include <concepts>

namespace spice {
template <class T>
concept StatelessNeuron = requires(float dt) {
	{ T::update(dt) } -> std::same_as<bool>;
};

template <class T>
concept StatefulNeuron = requires(T t, float dt) {
	requires !StatelessNeuron<T>;
	requires std::default_initializable<T>;
	{ t.update(dt) } -> std::same_as<bool>;
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
	requires !StatelessSynapse<T, Neur>;
	requires StatefulNeuron<Neur>;
	requires std::default_initializable<T>;
	t.deliver(n);
};

template <class T, class Neur>
concept Synapse = StatelessSynapse<T, Neur> || StatefulSynapse<T, Neur>;
}