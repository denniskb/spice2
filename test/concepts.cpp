#include <gtest/gtest.h>

#include "hana.h"

#include "spice/concepts.h"
#include "spice/util/type_traits.h"

using namespace boost;
using namespace spice;
using namespace spice::util;

template <int N>
void cartesian(auto&& f) {
	auto combos = hana::cartesian_product(hana::replicate<hana::tuple_tag>(
	    hana::make_tuple(hana::bool_c<false>, hana::bool_c<true>), hana::size_c<N>));

	hana::for_each(combos, [&](auto combo) { hana::unpack(combo, f); });
}

struct stateless_neuron {};
struct stateful_neuron {
	struct neuron {};
};

template <bool stateful, bool per_neuron_init, bool per_neuron_update, bool per_pop_init, bool per_pop_update,
          bool err>
struct meta_neuron : public std::conditional_t<stateful, stateful_neuron, stateless_neuron> {
	using neuron_t = stateful_neuron::neuron;

	bool update(float, auto) const requires(per_neuron_update && !stateful && !err) { return false; }
	void update(float, auto) const requires(per_neuron_update && !stateful && err) {}
	bool update(float, auto) /* */ requires(per_neuron_update && !stateful && err) { return false; }

	bool update(neuron_t&, float, auto) const requires(per_neuron_update&& stateful && !err) { return false; }
	void update(neuron_t&, float, auto) const requires(per_neuron_update&& stateful&& err) {}
	bool update(neuron_t&, float, auto) /* */ requires(per_neuron_update&& stateful&& err) { return false; }

	void init(neuron_t&, Int, auto) const requires(stateful&& per_neuron_init && !err) {}
	void init(neuron_t&, Int, auto) /* */ requires(stateful&& per_neuron_init&& err) {}

	void init(std::span<neuron_t>, auto) /* */ requires(stateful&& per_pop_init && !err) {}
	void init(std::span<neuron_t>, auto) const requires(stateful&& per_pop_init && !err) {}

	void update(float, auto, std::vector<Int32>&) /* */ requires(per_pop_update && !err) {}
	void update(float, auto, std::vector<Int32>&) const requires(per_pop_update && !err) {}
};

TEST(Concepts, Neuron) {
	cartesian<5>([](auto stateful, auto per_neuron_init, auto per_neuron_update, auto per_pop_init,
	                auto per_pop_update) {
		// Any neuron with any erroneous signature isn't a Neuron
		static_assert(!Neuron<meta_neuron<stateful, per_neuron_init, per_neuron_update, per_pop_init,
		                                  per_pop_update, true>>);

		// Any neuron lacking an update() method isn't a Neuron
		static_assert(!Neuron<meta_neuron<stateful, per_neuron_init, false, per_pop_init, false, false>>);

		// Any neuron defining both flavors of init() isn't a Neuron
		static_assert(!Neuron<meta_neuron<true, true, per_neuron_update, true, false, false>>);

		// Any neuron defining a per-population update() method while also having traces of per-neuron update, isn't a Neuron
		if constexpr (per_pop_update && any_of < stateful, stateful && (per_neuron_init || per_pop_init),
		              per_neuron_update >)
			static_assert(!Neuron<meta_neuron<stateful, per_neuron_init, per_neuron_update, per_pop_init,
			                                  per_pop_update, false>>);

		// Only per-population update defined
		static_assert(Neuron<meta_neuron<false, false, false, false, true, false>>);

		// Valid neuron
		if constexpr (stateful ? up_to_one_of<per_neuron_init, per_pop_init> :
                                 none_of<per_neuron_init, per_pop_init>)
			static_assert(Neuron<meta_neuron<stateful, per_neuron_init, true, per_pop_init, false, false>>);
	});
}

struct stateless_synapse {};
struct stateful_synapse {
	struct synapse {};
};

template <bool stateful, bool deliver_to, bool deliver_from, bool plastic, bool per_syn_init, bool err>
struct meta_synapse : public std::conditional_t<stateful, stateful_synapse, stateless_synapse> {
	using neuron_t  = stateful_neuron::neuron;
	using synapse_t = stateful_synapse::synapse;

	void deliver(neuron_t&) const requires(!stateful && deliver_to && !err) {}
	void deliver(neuron_t&) /* */ requires(!stateful && deliver_to && err) {}

	void deliver(neuron_t const&, neuron_t&) const requires(!stateful && deliver_from && !err) {}
	void deliver(neuron_t const&, neuron_t&) /* */ requires(!stateful && deliver_from && err) {}

	void deliver(synapse_t const&, neuron_t&) const requires(stateful&& deliver_to && !err) {}
	void deliver(synapse_t const&, neuron_t&) /* */ requires(stateful&& deliver_to&& err) {}

	void deliver(synapse_t const&, neuron_t const&, neuron_t&) const
	    requires(stateful&& deliver_from && !err) {}
	void deliver(synapse_t const&, neuron_t const&, neuron_t&) /* */ requires(stateful&& deliver_from&& err) {
	}

	void update(synapse_t&, float, bool, bool) const requires(stateful&& plastic && !err) {}
	void update(synapse_t&, float, bool, bool) /* */ requires(stateful&& plastic&& err) {}
	void skip(synapse_t&, float, Int) const requires(stateful&& plastic && !err) {}
	void skip(synapse_t&, float, Int) /* */ requires(stateful&& plastic && !err) {}

	void init(synapse_t&, Int, Int, auto) const requires(stateful&& per_syn_init && !err) {}
	void init(synapse_t&, Int, Int, auto) /* */ requires(stateful&& per_syn_init&& err) {}
};

TEST(Concepts, Synapse) {
	cartesian<5>([](auto stateful, auto deliver_to, auto deliver_from, auto plastic, auto per_syn_init) {
		// Any synapse with any errenous signature is not a Synapse
		static_assert(!Synapse<meta_synapse<stateful, deliver_to, deliver_from, plastic, per_syn_init, true>,
		                       stateful_neuron, stateful_neuron>);

		// Any synapse without any deliver() method is not a Synapse
		static_assert(!Synapse<meta_synapse<stateful, false, false, plastic, per_syn_init, false>,
		                       stateful_neuron, stateful_neuron>);

		// Any synapse with both deliver() methods is not a Synapse
		static_assert(!Synapse<meta_synapse<stateful, true, true, plastic, per_syn_init, false>,
		                       stateful_neuron, stateful_neuron>);

		// PerSynapseInit is independent of the rest
		static_assert(PerSynapseInit<meta_synapse<true, deliver_to, deliver_from, plastic, true, false>>);
		static_assert(!PerSynapseInit<meta_synapse<true, deliver_to, deliver_from, plastic, true, true>>);
		static_assert(!PerSynapseInit<meta_synapse<true, deliver_to, deliver_from, plastic, false, false>>);

		// Valid synapse
		if constexpr (one_of<deliver_to, deliver_from> && (!plastic || stateful) &&
		              (!per_syn_init || stateful)) {
			static_assert(
			    Synapse<meta_synapse<stateful, deliver_to, deliver_from, plastic, per_syn_init, false>,
			            stateful_neuron, stateful_neuron>);

			static_assert(Synapse<meta_synapse<stateful, true, false, plastic, per_syn_init, false>,
			                      stateless_neuron, stateful_neuron>);

			static_assert(!Synapse<meta_synapse<stateful, false, true, plastic, per_syn_init, false>,
			                       stateless_neuron, stateful_neuron>);
		}
	});
}