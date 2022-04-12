#include <gtest/gtest.h>

#include <spice/util/type_traits.h>

using namespace spice::util;

TEST(TypeTraits, Empty) {
	static_assert(std::is_empty_v<empty_t>);
	static_assert(is_empty_v<empty_t>);
	static_assert(!is_empty_v<int>);
}

TEST(TypeTraits, Optional) {
	static_assert(std::is_same_v<optional_t<int, true>, int>);
	static_assert(std::is_same_v<optional_t<int, false>, empty_t>);
}

TEST(TypeTraits, NoneOf) {
	static_assert(none_of<false>);
	static_assert(none_of<false, false, false>);
	static_assert(!none_of<true>);
	static_assert(!none_of<true, false, false>);
}

TEST(TypeTraits, OneOf) {
	static_assert(!one_of<false>);
	static_assert(!one_of<false, false, false>);
	static_assert(one_of<true>);
	static_assert(one_of<true, false, false>);
	static_assert(!one_of<true, true>);
	static_assert(!one_of<true, false, true>);
}

TEST(TypeTraits, UpToOneOf) {
	static_assert(up_to_one_of<false>);
	static_assert(up_to_one_of<true>);
	static_assert(up_to_one_of<false, true>);
	static_assert(!up_to_one_of<true, true>);
	static_assert(!up_to_one_of<false, true, true>);
}

TEST(TypeTraits, AnyOf) {
	static_assert(any_of<true>);
	static_assert(any_of<true, false>);
	static_assert(any_of<true, false, true>);
	static_assert(!any_of<false>);
	static_assert(!any_of<false, false>);
}

TEST(TypeTraits, AllOf) {
	static_assert(all_of<true>);
	static_assert(all_of<true, true>);
	static_assert(!all_of<false>);
	static_assert(!all_of<false, true>);
}