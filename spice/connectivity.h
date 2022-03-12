#pragma once

#include <span>
#include <utility>
#include <vector>

#include "spice/util/random.h"
#include "spice/util/stdint.h"

namespace spice {
class edge_stream {
public:
	edge_stream(std::span<Int> offsets, std::span<Int32> neighbors);
	edge_stream& operator<<(std::pair<Int32, Int32> const edge);
	void flush();

private:
	std::span<Int> _offsets;
	std::span<Int32> _neighbors;
	std::pair<Int32, Int32> _prev{-1, -1};
	Int _src = 0;
	Int _dst = 0;
};

struct Connectivity {
	Int src_count = 0;
	Int dst_count = 0;

	virtual ~Connectivity() = default;
	Connectivity& operator()(Int src_count_, Int dst_count_);

	virtual Int size() const                                               = 0;
	virtual void generate(edge_stream& stream, util::seed_seq const& seed) = 0;
};

class adj_list : public Connectivity {
public:
	void connect(Int const src, Int const dst);

	Int size() const override;
	void generate(edge_stream& stream, util::seed_seq const& seed);

private:
	std::vector<UInt> _connections;
};

class fixed_probability : public Connectivity {
public:
	explicit fixed_probability(double const p);

	Int size() const override;
	void generate(edge_stream& stream, util::seed_seq const& seed);

private:
	double const _p;
};
}