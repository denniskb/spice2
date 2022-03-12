#pragma once

#include <span>
#include <vector>

#include "spice/util/random.h"
#include "spice/util/stdint.h"

namespace spice {
struct Connectivity {
	virtual ~Connectivity()                           = default;
	virtual Int size() const                          = 0;
	virtual Int src_count() const                     = 0;
	virtual void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors,
	                      util::seed_seq const& seed) = 0;
};

class adj_list : public Connectivity {
public:
	void connect(Int const src, Int const dst);

	Int size() const override;
	Int src_count() const override;
	void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors, util::seed_seq const& seed) override;

private:
	std::vector<UInt> _connections;
	Int _max_src_id = -1;
};

class fixed_probability : public Connectivity {
public:
	explicit fixed_probability(Int const src_count, Int const dst_count, double const p);

	Int size() const;
	Int src_count() const;
	void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors, util::seed_seq const& seed);

private:
	Int const _src_count;
	Int const _dst_count;
	double const _p;
};
}