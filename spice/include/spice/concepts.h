#pragma once

#include <concepts>
#include <random>
#include <span>

#include "spice/util/stdint.h"
#include "spice/util/type_traits.h"

namespace spice {
template <class T>
concept StatelessNeuron = requires(T const t, float dt, std::mt19937& rng) {
	requires std::default_initializable<T>;
	{ t.update(dt, rng) } -> std::same_as<bool>;
};

template <class T>
concept StatefulNeuron = requires(T const t, typename T::neuron& n, float dt, std::mt19937& rng) {
	requires std::default_initializable<T>;
	typename T::neuron;
	requires std::default_initializable<typename T::neuron>;
	{ t.update(n, dt, rng) } -> std::same_as<bool>;
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
concept PerPopulationUpdate = requires(T t, float dt, std::mt19937& rng, std::vector<Int32>& out_spikes) {
	requires std::default_initializable<T>;
	{ t.size() } -> std::convertible_to<Int>;
	t.update(dt, rng, out_spikes);
};

template <class T>
concept Neuron = util::one_of<PerPopulationUpdate<T>, util::one_of<StatelessNeuron<T>, StatefulNeuron<T>> &&
                              util::up_to_one_of<PerNeuronInit<T>, PerPopulationInit<T>>>;

template <class T, class Neur>
concept StatelessSynapse = requires(T const t, typename Neur::neuron& n) {
	requires StatefulNeuron<Neur>;
	requires std::default_initializable<T>;
	t.deliver(n);
};

template <class T, class Neur>
concept StatefulSynapse = requires(T const t, typename T::synapse const& syn, typename Neur::neuron& n) {
	requires StatefulNeuron<Neur>;
	requires std::default_initializable<T>;
	typename T::synapse;
	requires std::default_initializable<typename T::synapse>;
	t.deliver(syn, n);
};

template <class T, class Neur>
concept Synapse = StatelessSynapse<T, Neur> || StatefulSynapse<T, Neur>;

template <class T, class Neur>
concept PlasticSynapse = requires(T const t, typename T::synapse& syn, float dt, bool pre, bool post, Int n) {
	requires StatefulSynapse<T, Neur>;
	t.update(syn, dt, pre, post);
	t.skip(syn, dt, n);
};

template <class T, class Neur>
concept PerSynapseInit = requires(T const t, typename T::synapse& syn, Int src, Int dst, std::mt19937& rng) {
	requires StatefulSynapse<T, Neur>;
	t.init(syn, src, dst, rng);
};

namespace detail {
	template <Neuron T, bool = StatefulNeuron<T>>
	struct neuron_traits {};

	template <Neuron T>
	struct neuron_traits<T, false> {
		using type = util::empty_t;
	};

	template <Neuron T>
	struct neuron_traits<T, true> {
		using type = typename T::neuron;
	};

	template <Neuron T>
	using neuron_traits_t = typename neuron_traits<T>::type;

	template <class T, StatefulNeuron Neur, bool = StatefulSynapse<T, Neur>>
	requires Synapse<T, Neur>
	struct synapse_traits {
	};

	template <class T, StatefulNeuron Neur>
	requires Synapse<T, Neur>
	struct synapse_traits<T, Neur, false> {
		using type = util::empty_t;
	};

	template <class T, StatefulNeuron Neur>
	requires Synapse<T, Neur>
	struct synapse_traits<T, Neur, true> {
		using type = typename T::synapse;
	};

	template <class T, StatefulNeuron Neur>
	using synapse_traits_t = typename synapse_traits<T, Neur>::type;
}
}