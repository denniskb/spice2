#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <initializer_list>
#include <limits>
#include <numbers>
#include <random>
#include <type_traits>

#include "assert.h"
#include "stdint.h"

namespace spice::util {
namespace detail {
template <std::integral Integer>
inline Integer rotl(Integer const x, Int const k) {
	return (x << k) | (x >> ((sizeof(Integer) * 8) - k));
}
}

// Copy-able, fixed-size (128bit) seed_seq
class seed_seq {
public:
	static constexpr Int SIZE = 4;

	seed_seq(std::initializer_list<UInt> il) { std::seed_seq(il).generate(_seed, _seed + SIZE); }

	template <class RandomIt>
	void generate(RandomIt first, RandomIt last) {
		Int i = 0;
		while (first != last)
			*first++ = _seed[i++ % SIZE];
	}

	Int size() const { return SIZE; }

	template <class OutputIt>
	void param(OutputIt it) {
		std::copy(_seed, _seed + SIZE, it);
	}

private:
	UInt32 _seed[SIZE];
};

template <std::integral Integer, int Period, class Next>
class xoroshiro {
public:
	using result_type = Integer;

	explicit xoroshiro(seed_seq sequence) {
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
	                                   state.s3 = detail::rotl(state.s3, 11);

	                                   return result;
                                   })>;

using xoroshiro64_128p = xoroshiro<UInt, 128, decltype([](auto& state) {
	                                   UInt const result = state.s0 + state.s1;

	                                   UInt const tmp = state.s0 ^ state.s1;
	                                   state.s0       = detail::rotl(state.s0, 24) ^ tmp ^ (tmp << 16);
	                                   state.s1       = detail::rotl(tmp, 37);

	                                   return result;
                                   })>;

template <std::floating_point Real, bool LeftOpen = false>
class uniform_real_distribution {
public:
	constexpr explicit uniform_real_distribution(Real const a = 0, Real const b = 1) :
	_offset(a), _scale(b - a) {}

	constexpr Real operator()(auto& rng) const {
		constexpr std::size_t rng_size = sizeof(decltype(rng()));
		constexpr std::size_t digits   = std::numeric_limits<Real>::digits;

		UInt iid = rng();
		if (rng_size < sizeof(Real))
			iid = (iid << (8 * rng_size)) | rng();

		return std::fma(((iid >> (8 * std::max(rng_size, sizeof(Real)) - digits)) + LeftOpen) /
		                    Real(Int(1) << digits),
		                _scale, _offset);
	}

private:
	Real const _offset = 0;
	Real const _scale  = 1;
};

template <std::floating_point Real>
class exponential_distribution {
public:
	constexpr explicit exponential_distribution(Real const scale = 1) : _scale(scale) {
		SPICE_ASSERT(scale >= 0);
	}
	constexpr Real operator()(auto& rng) const { return -_scale * std::log(_iid(rng)); }

private:
	Real const _scale;
	uniform_real_distribution<Real, true> _iid;
};

template <std::floating_point Real>
class normal_distribution {
public:
	constexpr explicit normal_distribution(Real const mu = 0, Real const sigma = 1) : _mu(mu), _sigma(sigma) {
		SPICE_ASSERT(sigma >= 0);
	}

	constexpr Real operator()(auto& rng) {
		using namespace std::numbers;

		switch (_state) {
			case 0: {
				_state           = 1;
				Real const R     = std::sqrt(-2 * std::log(_iid(rng)));
				Real const Theta = 2 * pi * _iid(rng);
				_z0              = std::fma(R * std::cos(Theta), _sigma, _mu);
				_z1              = std::fma(R * std::sin(Theta), _sigma, _mu);
				return _z0;
			}
			case 1: _state = 0; return _z1;
		}
		__builtin_unreachable();
	}

private:
	bool _state = false;
	Real _z0;
	Real _z1;

	Real const _mu;
	Real const _sigma;
	uniform_real_distribution<Real, true> _iid;
};

template <std::integral Integer>
class binomial_distribution {
public:
	using Real = std::conditional_t<sizeof(Integer) == 8, double, float>;

	constexpr explicit binomial_distribution(Integer const N, Real const p) :
	_N(N), _norm(N * p, std::sqrt(N * p * (1 - p))) {
		SPICE_ASSERT(N >= 0);
		SPICE_ASSERT(0 <= p && p <= 1);
	}

	constexpr Integer operator()(auto& rng) {
		return std::min(_N, static_cast<Integer>(std::max(Real(0), std::round(_norm(rng)))));
	}

private:
	Integer const _N;
	normal_distribution<Real> _norm;
};
}