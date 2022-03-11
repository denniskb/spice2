#pragma once

#include <cstddef>
#include <cstdint>

using Int8   = std::int8_t;
using Int16  = std::int16_t;
using Int32  = std::int32_t;
using Int    = std::int64_t;
using UInt8  = std::uint8_t;
using UInt16 = std::uint16_t;
using UInt32 = std::uint32_t;
using UInt   = std::uint64_t;

struct UInt128 {
	UInt lo;
	UInt hi;

	constexpr UInt128 operator+(UInt const n) const {
		UInt128 result{lo + n, hi};
		result.hi += result.lo < lo;
		return result;
	}
};