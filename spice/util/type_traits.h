#pragma once

#include <type_traits>

namespace spice::util {
struct empty_t {};

template <class T>
using nonvoid_or_empty_t = std::conditional_t<std::is_void_v<T>, empty_t, T>;

struct any_t {
	template <class To>
	constexpr operator To() const {
		return {};
	}
};
}