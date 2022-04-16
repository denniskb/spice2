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
	bool update(float, auto) const requires(per_neuron_update && !stateful && !err) { return false; }
	void update(float, auto) const requires(per_neuron_update && !stateful && err) {}
	bool update(float, auto) /* */ requires(per_neuron_update && !stateful && err) { return false; }

	bool update(stateful_neuron::neuron&, float, auto) const requires(per_neuron_update&& stateful && !err) {
		return false;
	}
	void update(stateful_neuron::neuron&, float, auto) const requires(per_neuron_update&& stateful&& err) {}
	bool update(stateful_neuron::neuron&, float, auto) /* */ requires(per_neuron_update&& stateful&& err) {
		return false;
	}

	void init(stateful_neuron::neuron&, Int, auto) const requires(stateful&& per_neuron_init && !err) {}
	void init(stateful_neuron::neuron&, Int, auto) /* */ requires(stateful&& per_neuron_init&& err) {}

	void init(std::span<stateful_neuron::neuron>, auto) /* */ requires(stateful&& per_pop_init && !err) {}
	void init(std::span<stateful_neuron::neuron>, auto) const requires(stateful&& per_pop_init && !err) {}

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