#pragma once

#include <span>
#include <vector>

#include "spice/util/stdint.h"

namespace spice {
class adj_list {
public:
	void connect(Int const src, Int const dst);

	Int size() const;
	Int src_count() const;
	void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors);

private:
	std::vector<UInt> _connections;
	Int _max_src_id = -1;
};
}