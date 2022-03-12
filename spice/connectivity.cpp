#include "spice/connectivity.h"

#include <algorithm>
#include <limits>

#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/range.h"

using namespace spice;

void adj_list::connect(Int const src, Int const dst) {
	SPICE_PRE(0 <= src && src < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst && dst < std::numeric_limits<Int32>::max());

	_max_src_id = std::max(_max_src_id, src);
	_connections.push_back(src << 32 | dst);
}

Int adj_list::size() const { return _connections.size(); }
Int adj_list::src_count() const { return _max_src_id + 1; }

void adj_list::fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors, util::seed_seq const&) {
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

fixed_probability::fixed_probability(Int const src_count, Int const dst_count, double const p) :
_src_count(src_count), _dst_count(dst_count), _p(p) {
	SPICE_PRE(src_count >= 0);
	SPICE_PRE(dst_count >= 0);
	SPICE_PRE(0 <= p && p <= 1);
}

Int fixed_probability::size() const {
	Int const max_degree = _dst_count * _p + 3 * std::sqrt(_dst_count * _p * (1 - _p));
	return _src_count * max_degree;
}

Int fixed_probability::src_count() const { return _src_count; }

void fixed_probability::fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors,
                                 util::seed_seq const& seed) {
	if (_src_count == 0 || _dst_count == 0 || _p == 0)
		return;

	util::xoroshiro64_128p rng(seed);
	util::exponential_distribution<double> exprnd(1 / _p - 1);

	Int const max_degree = size() / src_count();
	Int count            = 0;
	for (UInt const src : util::range(src_count())) {
		offsets[src] = count;

		Int32 index  = 0;
		double noise = 0;
		for (;;) {
			noise += exprnd(rng);
			Int32 const dst = index + static_cast<Int32>(std::round(noise));

			if (__builtin_expect((dst >= _dst_count) | (index >= max_degree), 0))
				break;

			neighbors[count] = dst;

			index++;
			count++;
		}
	}
	offsets[src_count()] = count;
}