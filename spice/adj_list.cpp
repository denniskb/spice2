#include "spice/adj_list.h"

#include <algorithm>
#include <limits>

#include "spice/util/assert.h"

using namespace spice;

void adj_list::connect(Int const src, Int const dst) {
	SPICE_PRE(0 <= src && src < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst && dst < std::numeric_limits<Int32>::max());

	_max_src_id = std::max(_max_src_id, src);
	_connections.push_back(src << 32 | dst);
}

Int adj_list::size() const { return _connections.size(); }
Int adj_list::src_count() const { return _max_src_id + 1; }

void adj_list::fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors) {
	SPICE_PRE(offsets.size() > src_count());
	SPICE_PRE(neighbors.size() >= size());

	std::sort(_connections.begin(), _connections.end());

	UInt prev   = ~UInt(0);
	Int src_idx = 0;
	Int dst_idx = 0;
	for (auto const c : _connections) {
		SPICE_PRE(c != prev && "adj_list must not contain duplicate edges");

		offsets[src_idx] = dst_idx;
		src_idx += ((c >> 32) != (prev >> 32));
		prev = c;

		neighbors[dst_idx++] = c & 0xffffffff;
	}
	offsets[src_idx] = dst_idx;
}