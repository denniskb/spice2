#pragma once

#include <span>
#include <vector>

#include "spice/util/random.h"
#include "spice/util/stdint.h"

namespace spice {
struct Connectivity {
	Int src_count = 0;
	Int dst_count = 0;

	virtual ~Connectivity()  = default;
	virtual Int size() const = 0;

	Connectivity& operator()(Int src_count_, Int dst_count_);
	void fill_csr(std::span<Int> offsets, std::span<Int32> neighbors, util::seed_seq const& seed);

private:
	virtual void do_fill_csr(std::span<Int> offsets, std::span<Int32> neighbors,
	                         util::seed_seq const& seed) = 0;
};

class adj_list : public Connectivity {
public:
	void connect(Int const src, Int const dst);

	Int size() const override;
	void do_fill_csr(std::span<Int> offsets, std::span<Int32> neighbors, util::seed_seq const& seed) override;

private:
	std::vector<UInt> _connections;
};

class fixed_probability : public Connectivity {
public:
	explicit fixed_probability(double const p);

	Int size() const override;
	void do_fill_csr(std::span<Int> offsets, std::span<Int32> neighbors, util::seed_seq const& seed) override;

private:
	double const _p;
};
}