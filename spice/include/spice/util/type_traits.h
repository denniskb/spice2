#pragma once

#include <type_traits>

namespace spice::util {
struct empty_t {};

template <class T>
constexpr bool const is_empty_v = std::is_same_v<T, empty_t>;

template <bool... predicates>
constexpr bool none_of = (predicates + ...) == 0;

template <bool... predicates>
constexpr bool one_of = (predicates + ...) == 1;

template <bool... predicates>
constexpr bool up_to_one_of = (predicates + ...) <= 1;

template <bool... predicates>
constexpr bool any_of = (predicates + ...) >= 1;

template <bool... predicates>
constexpr bool all_of = (predicates + ...) == sizeof...(predicates);
}