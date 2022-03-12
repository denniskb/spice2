#include "spice/connectivity.h"

#include <algorithm>
#include <limits>

#include "spice/util/assert.h"
#include "spice/util/random.h"
#include "spice/util/range.h"

using namespace spice;

edge_stream::edge_stream(std::span<Int> offsets, std::span<Int32> neighbors) :
_offsets(std::move(offsets)), _neighbors(std::move(neighbors)) {}

edge_stream& edge_stream::operator<<(std::pair<Int32, Int32> const edge) {
	SPICE_PRE(_src < _offsets.size());
	SPICE_PRE(_dst < _neighbors.size());
	SPICE_PRE(edge.first < _offsets.size());
	SPICE_PRE(_prev < edge && "adj_list must not contain duplicate edges");

	_offsets[_src] = _dst;
	_src += (edge.first != _prev.first);
	_prev = edge;

	_neighbors[_dst++] = edge.second;

	return *this;
}

void edge_stream::flush() {
	SPICE_PRE(_src < _offsets.size());
	_offsets[_src] = _dst;
	*this          = edge_stream(std::move(_offsets), std::move(_neighbors));
}

Connectivity& Connectivity::operator()(Int const src_count_, Int const dst_count_) {
	SPICE_PRE(0 <= src_count_ && src_count_ < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst_count_ && dst_count_ < std::numeric_limits<Int32>::max());

	src_count = src_count_;
	dst_count = dst_count_;
	return *this;
}

void adj_list::connect(Int const src, Int const dst) {
	SPICE_PRE(0 <= src && src < std::numeric_limits<Int32>::max());
	SPICE_PRE(0 <= dst && dst < std::numeric_limits<Int32>::max());

	_connections.push_back(src << 32 | dst);
}

Int adj_list::size() const { return _connections.size(); }

void adj_list::generate(edge_stream& stream, util::seed_seq const&) {
	std::sort(_connections.begin(), _connections.end());

	for (auto c : _connections) {
		SPICE_PRE((c >> 32) < src_count && "Source index exceeds source population neuron count");
		SPICE_PRE((c & 0xffffffff) < dst_count && "Destination index exceeds target population neuron count");
		stream << std::pair{c >> 32, c & 0xffffffff};
	}
}

fixed_probability::fixed_probability(double const p) : _p(p) { SPICE_PRE(0 <= p && p <= 1); }

Int fixed_probability::size() const {
	Int const max_degree = dst_count * _p + 3 * std::sqrt(dst_count * _p * (1 - _p));
	return src_count * max_degree;
}

void fixed_probability::generate(edge_stream& stream, util::seed_seq const& seed) {
	if (src_count == 0 || dst_count == 0 || _p == 0)
		return;

	util::xoroshiro64_128p rng(seed);
	util::exponential_distribution<double> exprnd(1 / _p - 1);

	Int const max_degree = size() / src_count;
	for (Int const src : util::range(src_count)) {
		Int32 index  = 0;
		double noise = 0;
		for (;;) {
			noise += exprnd(rng);
			Int32 const dst = index + static_cast<Int32>(std::round(noise));

			if (__builtin_expect((dst >= dst_count) | (index >= max_degree), 0))
				break;

			SPICE_INV(src < src_count);
			SPICE_INV(dst < dst_count);

			stream << std::pair{src, dst};
			index++;
		}
	}
}