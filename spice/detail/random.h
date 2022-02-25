#pragma once

#include <cmath>
#include <limits>
#include <random>
#include <type_traits>

#include "stdint.h"

namespace spice::detail {
template <class Integer>
inline Integer rotl(Integer const x, Int const k) {
	static_assert(std::is_integral_v<Integer>);

	return (x << k) | (x >> ((sizeof(Integer) * 8) - k));
}

template <class Integer, int Period, class Next>
class xoroshiro {
public:
	using result_type = Integer;

	explicit xoroshiro(std::seed_seq sequence) {
		auto* const ptr = reinterpret_cast<UInt32*>(&_state);
		sequence.generate(ptr, ptr + Period / 32);
	}

	constexpr Integer min() { return 0; }
	constexpr Integer max() { return std::numeric_limits<Integer>::max(); }
	constexpr Integer operator()() { return Next()(_state); }

private:
	struct state2 {
		Integer s0;
		Integer s1;
	};
	struct state4 {
		Integer s0;
		Integer s1;
		Integer s2;
		Integer s3;
	};
	std::conditional_t<Period / (sizeof(Integer) * 8) == 2, state2, state4> _state;
};

using xoroshiro32_128p = xoroshiro<UInt32, 128, decltype([](auto& state) {
	                                   UInt32 const result = state.s0 + state.s3;
	                                   UInt32 const t      = state.s1 << 9;

	                                   state.s2 ^= state.s0;
	                                   state.s3 ^= state.s1;
	                                   state.s1 ^= state.s2;
	                                   state.s0 ^= state.s3;

	                                   state.s2 ^= t;
	                                   state.s3 = rotl(state.s3, 11);

	                                   return result;
                                   })>;

using xoroshiro64_128p = xoroshiro<UInt, 128, decltype([](auto& state) {
	                                   UInt const result = state.s0 + state.s1;

	                                   UInt const tmp = state.s0 ^ state.s1;
	                                   state.s0       = rotl(state.s0, 24) ^ tmp ^ (tmp << 16);
	                                   state.s1       = rotl(tmp, 37);

	                                   return result;
                                   })>;

constexpr float uniform_left_inc(auto& rng) { return (rng() >> 8) / 16777216.0f; }

constexpr float uniform_right_inc(auto& rng) { return ((rng() >> 8) + 1.0f) / 16777216.0f; }

constexpr float exprnd(auto& rng, float const lambda = 1.0f) {
	return -lambda * std::log(uniform_right_inc(rng));
}
}