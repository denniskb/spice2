#pragma once

#include <concepts>

#include "util/stdint.h"
#include "util/type_traits.h"

namespace spice {
template <class T>
concept StatelessNeuronWithoutParams = requires(float dt) {
	{ T::update(dt) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatelessNeuronWithParams = requires(float dt, Params params) {
	requires std::default_initializable<Params>;
	{ T::update(dt, params) } -> std::same_as<bool>;
};

template <class T>
concept StatelessNeuron = StatelessNeuronWithoutParams<T> || StatelessNeuronWithParams<T, util::any_t>;

template <class T>
concept StatefulNeuronWithoutParams = requires(T t, float dt) {
	requires !StatelessNeuron<T>;
	requires std::default_initializable<T>;
	{ t.update(dt) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatefulNeuronWithParams = requires(T t, float dt, Params params) {
	requires std::default_initializable<Params>;
	requires !StatelessNeuron<T>;
	requires std::default_initializable<T>;
	{ t.update(dt, params) } -> std::same_as<bool>;
};

template <class T>
concept StatefulNeuron = StatefulNeuronWithoutParams<T> || StatefulNeuronWithParams<T, util::any_t>;

template <class T>
concept NeuronWithoutParams = StatelessNeuronWithoutParams<T> || StatefulNeuronWithoutParams<T>;

template <class T, class Params>
concept NeuronWithParams = StatelessNeuronWithParams<T, Params> || StatefulNeuronWithParams<T, Params>;

template <class T>
concept Neuron = StatelessNeuron<T> || StatefulNeuron<T>;

template <class T, class Neur>
concept StatelessSynapseWithoutParams = requires(Neur& n) {
	requires StatefulNeuron<Neur>;
	T::deliver(n);
};

template <class T, class Neur, class Params>
concept StatelessSynapseWithParams = requires(Neur& n, Params params) {
	requires std::default_initializable<Params>;
	requires StatefulNeuron<Neur>;
	T::deliver(n, params);
};

template <class T, class Neur>
concept StatelessSynapse =
    StatelessSynapseWithoutParams<T, Neur> || StatelessSynapseWithParams<T, Neur, util::any_t>;

template <class T, class Neur>
concept StatefulSynapseWithoutParams = requires(T t, Neur& n) {
	requires StatefulNeuron<Neur>;
	requires !StatelessSynapse<T, Neur>;
	requires std::default_initializable<T>;
	t.deliver(n);
};

template <class T, class Neur, class Params>
concept StatefulSynapseWithParams = requires(T t, Neur& n, Params params) {
	requires std::default_initializable<Params>;
	requires StatefulNeuron<Neur>;
	requires !StatelessSynapse<T, Neur>;
	requires std::default_initializable<T>;
	t.deliver(n, params);
};

template <class T, class Neur>
concept StatefulSynapse =
    StatefulSynapseWithoutParams<T, Neur> || StatefulSynapseWithParams<T, Neur, util::any_t>;

template <class T, class Neur>
concept SynapseWithoutParams =
    StatelessSynapseWithoutParams<T, Neur> || StatefulSynapseWithoutParams<T, Neur>;

template <class T, class Neur, class Params>
concept SynapseWithParams =
    StatelessSynapseWithParams<T, Neur, Params> || StatefulSynapseWithParams<T, Neur, Params>;

template <class T, class Neur>
concept Synapse = StatelessSynapse<T, Neur> || StatefulSynapse<T, Neur>;
}