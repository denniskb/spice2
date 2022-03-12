#pragma once

#include <span>

#include "spice/util/stdint.h"

namespace spice {
struct Connectivity {
	virtual ~Connectivity()                                                    = default;
	virtual Int size() const                                                   = 0;
	virtual Int src_count() const                                              = 0;
	virtual void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors) = 0;
};
}