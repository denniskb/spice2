#include "gtest/gtest.h"

#include "spice/detail/csr.h"
#include "spice/util/type_traits.h"

using namespace spice;
using namespace spice::detail;

static_assert(std::input_iterator<csr<void>::iterator>);
static_assert(std::input_iterator<csr<void>::const_iterator>);
static_assert(std::input_iterator<csr<util::empty_t>::iterator>);
static_assert(std::input_iterator<csr<util::empty_t>::const_iterator>);