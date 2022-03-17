#pragma once

#include <concepts>

#include "spice/util/random.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
template <class T>
concept StatelessNeuronWithoutParams = requires(float dt, util::xoroshiro64_128p& rng) {
	{ T::update(dt, rng) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatelessNeuronWithParams = requires(float dt, util::xoroshiro64_128p& rng, Params params) {
	requires std::default_initializable<Params>;
	{ T::update(dt, rng, params) } -> std::same_as<bool>;
};

template <class T>
concept StatelessNeuron = StatelessNeuronWithoutParams<T> || StatelessNeuronWithParams<T, util::any_t>;

template <class T>
concept StatefulNeuronWithoutParams = requires(T t, float dt, util::xoroshiro64_128p& rng) {
	requires !StatelessNeuron<T>;
	requires std::default_initializable<T>;
	{ t.update(dt, rng) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatefulNeuronWithParams = requires(T t, float dt, util::xoroshiro64_128p& rng, Params params) {
	requires std::default_initializable<Params>;
	requires !StatelessNeuron<T>;
	requires std::default_initializable<T>;
	{ t.update(dt, rng, params) } -> std::same_as<bool>;
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

template <class T>
concept PlasticSynapse = requires(T t, float dt, bool pre, bool post, Int n, util::any_t params) {
	requires(
	    requires { t.update(dt, pre, post, n); } || requires { t.update(dt, pre, post, n, params); });
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