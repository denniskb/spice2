#include "spice/connectivity.h"

#include <algorithm>
#include <limits>

#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/range.h"

using namespace spice;

Connectivity& Connectivity::operator()(Int const src_count_, Int const dst_count_) {
	SPICE_PRE(0 <= src_count_ && src_count_ < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst_count_ && dst_count_ < std::numeric_limits<Int32>::max());

	src_count = src_count_;
	dst_count = dst_count_;
	return *this;
}

void Connectivity::fill_csr(std::span<Int> offsets, std::span<Int32> neighbors, util::seed_seq const& seed) {
	SPICE_PRE(offsets.size() > src_count);
	SPICE_PRE(neighbors.size() >= size());

	do_fill_csr(offsets, neighbors, seed);
}

void adj_list::connect(Int const src, Int const dst) {
	SPICE_PRE(0 <= src && src < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst && dst < std::numeric_limits<Int32>::max());

	_connections.push_back(src << 32 | dst);
}

Int adj_list::size() const { return _connections.size(); }

void adj_list::do_fill_csr(std::span<Int> offsets, std::span<Int32> neighbors, util::seed_seq const&) {
	std::sort(_connections.begin(), _connections.end());

	UInt prev   = ~UInt(0);
	Int src_idx = 0;
	Int dst_idx = 0;
	for (auto const c : _connections) {
		SPICE_PRE((c >> 32) < src_count && "Source index exceeds source population neuron count");
		SPICE_PRE((c & 0xffffffff) < dst_count && "Destination index exceeds target population neuron count");
		SPICE_PRE(c != prev && "adj_list must not contain duplicate edges");

		offsets[src_idx] = dst_idx;
		src_idx += ((c >> 32) != (prev >> 32));
		prev = c;

		neighbors[dst_idx++] = c & 0xffffffff;
	}
	offsets[src_idx] = dst_idx;
}

fixed_probability::fixed_probability(double const p) : _p(p) { SPICE_PRE(0 <= p && p <= 1); }

Int fixed_probability::size() const {
	Int const max_degree = dst_count * _p + 3 * std::sqrt(dst_count * _p * (1 - _p));
	return src_count * max_degree;
}

void fixed_probability::do_fill_csr(std::span<Int> offsets, std::span<Int32> neighbors,
                                    util::seed_seq const& seed) {
	if (src_count == 0 || dst_count == 0 || _p == 0)
		return;

	util::xoroshiro64_128p rng(seed);
	util::exponential_distribution<double> exprnd(1 / _p - 1);

	Int const max_degree = size() / src_count;
	Int count            = 0;
	for (UInt const src : util::range(src_count)) {
		offsets[src] = count;

		Int32 index  = 0;
		double noise = 0;
		for (;;) {
			noise += exprnd(rng);
			Int32 const dst = index + static_cast<Int32>(std::round(noise));

			if (__builtin_expect((dst >= dst_count) | (index >= max_degree), 0))
				break;

			neighbors[count] = dst;

			index++;
			count++;
		}
	}
	offsets[src_count] = count;
}