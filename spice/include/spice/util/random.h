#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <numbers>
#include <random>
#include <type_traits>

#include "spice/util/assert.h"
#include "spice/util/stdint.h"

namespace spice::util {
namespace detail {
template <std::integral Integer>
constexpr Integer rotl(Integer const x, Int const k) {
	return (x << k) | (x >> ((sizeof(Integer) * 8) - k));
}

constexpr uint64_t fmix64(uint64_t k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;
	return k;
}

constexpr UInt128 murmur3(void const* ptr, UInt const len) {
	UInt8 const* const data = static_cast<UInt8 const*>(ptr);

	// non-zero seed (chosen arbitrarily)
	UInt128 h{0x2E4016967F18E81, 0x447567949F9AA86};

	UInt const c1 = 0x87c37b91114253d5;
	UInt const c2 = 0x4cf5ad432745937f;

	const Int nblocks  = len / 16;
	UInt const* blocks = reinterpret_cast<UInt const*>(data);
	for (Int i = 0; i < nblocks; i++) {
		UInt k1 = blocks[i * 2 + 0];
		UInt k2 = blocks[i * 2 + 1];

		k1 *= c1;
		k1 = rotl(k1, 31);
		k1 *= c2;
		h.lo ^= k1;
		h.lo = rotl(h.lo, 27);
		h.lo += h.hi;
		h.lo = h.lo * 5 + 0x52dce729;
		k2 *= c2;
		k2 = rotl(k2, 33);
		k2 *= c1;
		h.hi ^= k2;
		h.hi = rotl(h.hi, 31);
		h.hi += h.lo;
		h.hi = h.hi * 5 + 0x38495ab5;
	}

	const UInt8* tail = data + nblocks * 16;

	UInt k1 = 0;
	UInt k2 = 0;

	switch (len & 15) {
		case 15: k2 ^= UInt(tail[14]) << 48;
		case 14: k2 ^= UInt(tail[13]) << 40;
		case 13: k2 ^= UInt(tail[12]) << 32;
		case 12: k2 ^= UInt(tail[11]) << 24;
		case 11: k2 ^= UInt(tail[10]) << 16;
		case 10: k2 ^= UInt(tail[9]) << 8;
		case 9:
			k2 ^= UInt(tail[8]) << 0;
			k2 *= c2;
			k2 = rotl(k2, 33);
			k2 *= c1;
			h.hi ^= k2;
		case 8: k1 ^= UInt(tail[7]) << 56;
		case 7: k1 ^= UInt(tail[6]) << 48;
		case 6: k1 ^= UInt(tail[5]) << 40;
		case 5: k1 ^= UInt(tail[4]) << 32;
		case 4: k1 ^= UInt(tail[3]) << 24;
		case 3: k1 ^= UInt(tail[2]) << 16;
		case 2: k1 ^= UInt(tail[1]) << 8;
		case 1:
			k1 ^= UInt(tail[0]) << 0;
			k1 *= c1;
			k1 = rotl(k1, 31);
			k1 *= c2;
			h.lo ^= k1;
	};

	h.lo ^= len;
	h.hi ^= len;
	h.lo += h.hi;
	h.hi += h.lo;
	h.lo = fmix64(h.lo);
	h.hi = fmix64(h.hi);
	h.lo += h.hi;
	h.hi += h.lo;

	return h;
}

constexpr UInt128 murmur3(UInt128 k) {
	// non-zero seed (chosen arbitrarily)
	UInt128 h{0x2E4016967F18E81, 0x447567949F9AA86};

	UInt const c1 = 0x87c37b91114253d5;
	UInt const c2 = 0x4cf5ad432745937f;

	k.lo *= c1;
	k.lo = rotl(k.lo, 31);
	k.lo *= c2;
	h.lo ^= k.lo;
	h.lo = rotl(h.lo, 27);
	h.lo += h.hi;
	h.lo = h.lo * 5 + 0x52dce729;
	k.hi *= c2;
	k.hi = rotl(k.hi, 33);
	k.hi *= c1;
	h.hi ^= k.hi;
	h.hi = rotl(h.hi, 31);
	h.hi += h.lo;
	h.hi = h.hi * 5 + 0x38495ab5;
	h.lo ^= 16;
	h.hi ^= 16;
	h.lo += h.hi;
	h.hi += h.lo;
	h.lo = fmix64(h.lo);
	h.hi = fmix64(h.hi);
	h.lo += h.hi;
	h.hi += h.lo;

	return h;
}
}

// Copy-able, fixed-size seed sequence
class seed_seq {
public:
	seed_seq(std::seed_seq seq) {
		Int32* const data = reinterpret_cast<Int32*>(&_seed);
		seq.generate(data, data + 4);
	}
	constexpr seed_seq(std::initializer_list<UInt32> il) : _seed(detail::murmur3(il.begin(), 4 * il.size())) {
		SPICE_PRE(il.size() > 0 && "Please provide at least 1 seed to seed_seq");
	}

	template <std::output_iterator<Int32> OutputIt>
	constexpr void generate(OutputIt first, OutputIt last) const {
		UInt32 const* seed = reinterpret_cast<UInt32 const*>(&_seed);
		for (Int i = 0; first != last; i++, first++)
			*first = seed[i % 4];
	}

	constexpr UInt128 seed() const { return _seed; }

	constexpr seed_seq operator++(int) {
		auto result = *this;
		_seed       = detail::murmur3(_seed);
		return result;
	}

	constexpr seed_seq stream(UInt id) const { return seed_seq(detail::murmur3(_seed + (id + 1))); }

private:
	UInt128 _seed;

	constexpr explicit seed_seq(UInt128 const seed) : _seed(seed) {}
};

template <std::integral Integer, int Period>
class xoroshiro {
public:
	using result_type = Integer;

	constexpr explicit xoroshiro(seed_seq const& seq) : _state(seq.seed()) {}

	constexpr Integer min() { return 0; }
	constexpr Integer max() { return std::numeric_limits<Integer>::max(); }

protected:
	struct state2 {
		constexpr state2(UInt128 const s) : s0(s.lo), s1(s.hi) {}
		Integer s0;
		Integer s1;
	};
	struct state4 {
		constexpr state4(UInt128 const s) : s0(s.lo), s1(s.lo >> 32), s2(s.hi), s3(s.hi >> 32) {}
		Integer s0;
		Integer s1;
		Integer s2;
		Integer s3;
	};
	std::conditional_t<Period / (sizeof(Integer) * 8) == 2, state2, state4> _state;
};

struct xoroshiro32_128p : public xoroshiro<UInt32, 128> {
	using xoroshiro::xoroshiro;

	UInt32 operator()() {
		UInt32 const result = _state.s0 + _state.s3;
		UInt32 const t      = _state.s1 << 9;

		_state.s2 ^= _state.s0;
		_state.s3 ^= _state.s1;
		_state.s1 ^= _state.s2;
		_state.s0 ^= _state.s3;

		_state.s2 ^= t;
		_state.s3 = detail::rotl(_state.s3, 11);

		return result;
	}
};

struct xoroshiro64_128p : public xoroshiro<UInt, 128> {
	using xoroshiro::xoroshiro;

	UInt operator()() {
		UInt const result = _state.s0 + _state.s1;

		UInt const tmp = _state.s0 ^ _state.s1;
		_state.s0      = detail::rotl(_state.s0, 24) ^ tmp ^ (tmp << 16);
		_state.s1      = detail::rotl(tmp, 37);

		return result;
	}
};

template <std::floating_point Real, bool LeftOpen = false>
constexpr Real generate_canonical(auto& rng) {
	constexpr std::size_t rng_size = sizeof(decltype(rng()));
	constexpr std::size_t digits   = std::numeric_limits<Real>::digits;

	UInt iid = rng();
	if constexpr (rng_size < sizeof(Real))
		iid = (iid << (8 * rng_size)) | rng();

	return ((iid >> (8 * std::max(rng_size, sizeof(Real)) - digits)) + LeftOpen) / Real(1_u64 << digits);
}

template <std::floating_point Real, bool LeftOpen = false>
class uniform_real_distribution {
public:
	constexpr explicit uniform_real_distribution(Real const a = 0, Real const b = 1) :
	_offset(a), _scale(b - a) {}

	constexpr Real operator()(auto& rng) const {
		return std::fma(generate_canonical<Real, LeftOpen>(rng), _scale, _offset);
	}

private:
	Real _offset = 0;
	Real _scale  = 1;
};

template <std::floating_point Real>
class exponential_distribution {
public:
	constexpr explicit exponential_distribution(Real const scale = 1) : _scale(scale) {
		SPICE_PRE(scale >= 0);
	}
	constexpr Real operator()(auto& rng) const {
		return -_scale * std::log(generate_canonical<Real, true>(rng));
	}

private:
	Real _scale;
};

template <std::floating_point Real>
class normal_distribution {
public:
	constexpr explicit normal_distribution(Real const mu = 0, Real const sigma = 1) : _mu(mu), _sigma(sigma) {
		SPICE_PRE(sigma >= 0);
	}

	constexpr Real operator()(auto& rng) {
		using namespace std::numbers;

		switch (_state) {
			case 0: {
				_state           = 1;
				Real const R     = std::sqrt(-2 * std::log(generate_canonical<Real, true>(rng)));
				Real const Theta = 2 * pi * generate_canonical<Real, false>(rng);
				_z0              = std::fma(R * std::cos(Theta), _sigma, _mu);
				_z1              = std::fma(R * std::sin(Theta), _sigma, _mu);
				return _z0;
			}
			case 1: _state = 0; return _z1;
		}
	}

private:
	bool _state = false;
	Real _z0;
	Real _z1;

	Real _mu;
	Real _sigma;
};

template <std::integral Integer>
class binomial_distribution {
public:
	using Real = std::conditional_t<sizeof(Integer) == 8, double, float>;

	constexpr explicit binomial_distribution(Integer const N, Real const p) :
	_N(N), _norm(N * p, std::sqrt(N * p * (1 - p))) {
		SPICE_PRE(N >= 0);
		SPICE_PRE(0 <= p && p <= 1);
	}

	constexpr Integer operator()(auto& rng) {
		return std::min(_N, static_cast<Integer>(std::max(Real(0), std::round(_norm(rng)))));
	}

private:
	Integer _N;
	normal_distribution<Real> _norm;
};
}