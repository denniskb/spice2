#pragma once

#include <span>
#include <vector>

#include "spice/connectivity.h"
#include "spice/util/stdint.h"

namespace spice {
class adj_list : public Connectivity {
public:
	void connect(Int const src, Int const dst);

	Int size() const override;
	Int src_count() const override;
	void fill_csr(std::span<UInt> offsets, std::span<Int32> neighbors) override;

private:
	std::vector<UInt> _connections;
	Int _max_src_id = -1;
};
}