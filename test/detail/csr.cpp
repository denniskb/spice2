#include "gtest/gtest.h"

#include "spice/detail/csr.h"
#include "spice/util/type_traits.h"

using namespace spice;
using namespace spice::detail;

static_assert(std::input_iterator<csr<>::iterator>);
static_assert(std::input_iterator<csr<>::const_iterator>);
static_assert(std::input_iterator<csr<int>::iterator>);
static_assert(std::input_iterator<csr<int>::const_iterator>);