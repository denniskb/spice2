#pragma once

#include <concepts>
#include <random>
#include <span>

#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
template <class T>
concept StatelessNeuron = std::default_initializable<T>;

template <class T>
concept StatefulNeuron = requires {
	requires std::default_initializable<T>;
	typename T::neuron;
	requires std::default_initializable<typename T::neuron>;
};

template <class T>
concept PerNeuronInit = requires(T const t, typename T::neuron& n, Int id, std::mt19937& rng) {
	requires StatefulNeuron<T>;
	t.init(n, id, rng);
};

template <class T>
concept PerPopulationInit = requires(T t, std::span<typename T::neuron> n, std::mt19937& rng) {
	requires StatefulNeuron<T>;
	t.init(n, rng);
};

template <class T>
concept PerNeuronUpdate = requires(T const t, float dt, std::mt19937& rng) {
	requires(StatefulNeuron<T> ?
	             requires(typename T::neuron & n) {
		             { t.update(n, dt, rng) } -> std::same_as<bool>;
	             } :
                 requires {
		             { t.update(dt, rng) } -> std::same_as<bool>;
	             });
};

template <class T>
concept PerPopulationUpdate = requires(T t, float dt, std::mt19937& rng,
                                       std::vector<Int32>& out_spikes) {
	requires std::default_initializable<T>;
	requires std::copy_constructible<T> || std::move_constructible<T>;
	t.update(dt, rng, out_spikes);
};

template <class T>
concept Neuron = (PerPopulationUpdate<T> ?
                      util::none_of<StatefulNeuron<T>, PerNeuronUpdate<T>, PerNeuronInit<T>,
                                    PerPopulationInit<T>> :
                      (StatelessNeuron<T> || StatefulNeuron<T>)&&PerNeuronUpdate<T> &&
                          util::up_to_one_of<PerNeuronInit<T>, PerPopulationInit<T>>);

template <class T>
concept StatelessSynapse = std::default_initializable<T>;

template <class T>
concept StatefulSynapse = requires {
	requires std::default_initializable<T>;
	typename T::synapse;
	requires std::default_initializable<typename T::synapse>;
};

template <class T, class Neur>
concept DeliverTo = requires(T const t, typename Neur::neuron& n) {
	requires StatefulNeuron<Neur>;
	requires(StatefulSynapse<T> ? requires(typename T::synapse const& syn) { t.deliver(syn, n); } :
                                  requires { t.deliver(n); });
};

template <class T, class SrcNeur, class DstNeur>
concept DeliverFromTo = requires(T const t, typename SrcNeur::neuron const& src,
                                 typename DstNeur::neuron& dst) {
	requires StatefulNeuron<SrcNeur>;
	requires StatefulNeuron<DstNeur>;
	requires(StatefulSynapse<T> ?
	             requires(typename T::synapse const& syn) { t.deliver(syn, src, dst); } :
                 requires { t.deliver(src, dst); });
};

template <class T>
concept PlasticSynapse = requires(T const t, typename T::synapse& syn, float dt, bool pre,
                                  bool post, Int n) {
	requires StatefulSynapse<T>;
	t.update(syn, dt, pre, post);
	t.skip(syn, dt, n);
};

template <class T>
concept PerSynapseInit = requires(T const t, typename T::synapse& syn, Int src, Int dst,
                                  std::mt19937& rng) {
	requires StatefulSynapse<T>;
	t.init(syn, src, dst, rng);
};

template <class T, class SrcNeur, class DstNeur>
concept Synapse =
    (StatelessSynapse<T> || StatefulSynapse<T> ||
     PlasticSynapse<T>)&&util::one_of<DeliverTo<T, DstNeur>, DeliverFromTo<T, SrcNeur, DstNeur>>;

namespace detail {
template <Neuron T, bool = StatefulNeuron<T>>
struct neuron_traits;

template <Neuron T>
struct neuron_traits<T, false> {
	using type = void;
};

template <Neuron T>
struct neuron_traits<T, true> {
	using type = typename T::neuron;
};

template <Neuron T>
using neuron_traits_t = typename neuron_traits<T>::type;

template <class T, bool = StatefulSynapse<T>>
struct synapse_traits;

template <class T>
struct synapse_traits<T, false> {
	using type = void;
};

template <class T>
struct synapse_traits<T, true> {
	using type = typename T::synapse;
};

template <class T>
using synapse_traits_t = typename synapse_traits<T>::type;
}

namespace detail {
template <class T>
constexpr bool HasUpdate(auto... args) {
	return requires(T t) { t.update(args...); };
}
template <class T>
constexpr bool HasInit(auto... args) {
	return requires(T t) { t.init(args...); };
}
}

template <class T>
constexpr std::true_type CheckNeuron() {
	// TODO: Make the Has*() calls generic
	util::any_t any;

	static_assert(StatelessNeuron<T>, "Every neuron must at least conform to StatelessNeuron.");

	constexpr bool has_update =
	    detail::HasUpdate<T>(any, any) || detail::HasUpdate<T>(any, any, any);
	static_assert(has_update, "Every neuron must have an update() method.");
	static_assert(!has_update || (PerNeuronUpdate<T> || PerPopulationUpdate<T>),
	              "Your update() method has the wrong signautre.");
	static_assert(
	    !PerPopulationUpdate<T> || util::none_of<StatefulNeuron<T>, PerNeuronUpdate<T>,
	                                             PerNeuronInit<T>, PerPopulationInit<T>>,
	    "Defining a per-population update() method prohibits you from defining any of the following: "
	    "per-neuron update(), per-neuron init(), per-population init(), making your neuron stateful.");

	constexpr bool has_init = detail::HasInit<T>(any, any) || detail::HasInit<T>(any, any, any);
	static_assert(!has_init || StatefulNeuron<T>,
	              "You defined an init() method but your neuron has no state.");
	static_assert(!has_init || (PerNeuronInit<T> || PerPopulationInit<T>),
	              "Your init() method has the wrong signature.");
	static_assert(util::up_to_one_of<PerNeuronInit<T>, PerPopulationInit<T>>,
	              "Your neuron must define at most 1 init() method.");

	return {};
}
}