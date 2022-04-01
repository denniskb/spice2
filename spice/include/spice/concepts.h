#pragma once

#include <concepts>

#include "spice/sim_info.h"
#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
template <class T>
concept StatelessNeuronWithoutParams = requires(float dt, sim_info& info) {
	{ T::update(dt, info) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatelessNeuronWithParams = requires(float dt, sim_info& info, Params params) {
	requires std::default_initializable<Params>;
	{ T::update(dt, info, params) } -> std::same_as<bool>;
};

template <class T>
concept StatelessNeuron = StatelessNeuronWithoutParams<T> || StatelessNeuronWithParams<T, util::any_t>;

namespace detail {
	template <class T>
	concept StatefulNeuronBase = requires(T t, float dt, sim_info& info) {
		requires !StatelessNeuron<T>;
		requires std::default_initializable<T>;
		requires std::copy_constructible<T>;
		requires std::copyable<T>;
	};
}

template <class T>
concept StatefulNeuronWithoutParams = requires(T t, float dt, sim_info& info) {
	requires detail::StatefulNeuronBase<T>;
	requires std::constructible_from<T, sim_info>;
	{ t.update(dt, info) } -> std::same_as<bool>;
};

template <class T, class Params>
concept StatefulNeuronWithParams = requires(T t, float dt, sim_info& info, Params params) {
	requires detail::StatefulNeuronBase<T>;
	requires std::constructible_from<T, sim_info, Params>;
	requires std::default_initializable<Params>;
	{ t.update(dt, info, params) } -> std::same_as<bool>;
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
concept StatelessSynapseWithoutParams = requires(Neur& n, sim_info& info) {
	requires StatefulNeuron<Neur>;
	T::deliver(n, info);
};

template <class T, class Neur, class Params>
concept StatelessSynapseWithParams = requires(Neur& n, sim_info& info, Params params) {
	requires std::default_initializable<Params>;
	requires StatefulNeuron<Neur>;
	T::deliver(n, info, params);
};

template <class T, class Neur>
concept StatelessSynapse =
    StatelessSynapseWithoutParams<T, Neur> || StatelessSynapseWithParams<T, Neur, util::any_t>;

namespace detail {
	template <class T, class Neur>
	concept StatefulSynapseBase = requires(T t, Neur & n, sim_info & info) {
		requires StatefulNeuron<Neur>;
		requires !StatelessSynapse<T, Neur>;
		requires std::default_initializable<T>;
		requires std::copy_constructible<T>;
		requires std::copyable<T>;
	};
}

template <class T, class Neur>
concept StatefulSynapseWithoutParams = requires(T t, Neur& n, sim_info& info) {
	requires detail::StatefulSynapseBase<T, Neur>;
	requires std::constructible_from<T, sim_info>;
	t.deliver(n, info);
};

template <class T, class Neur, class Params>
concept StatefulSynapseWithParams = requires(T t, Neur& n, sim_info& info, Params params) {
	requires detail::StatefulSynapseBase<T, Neur>;
	requires std::constructible_from<T, sim_info, Params>;
	requires std::default_initializable<Params>;
	t.deliver(n, info, params);
};

template <class T>
concept PlasticSynapse = requires(T t, float dt, bool pre, bool post, Int n, sim_info& info,
                                  util::any_t params) {
	requires(
	    requires { t.update(dt, pre, post, info); } || requires { t.update(dt, pre, post, info, params); });
	requires(
	    requires { t.skip(dt, n, info); } || requires { t.skip(dt, n, info, params); });
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