#pragma once

#include <string>

#define SPICE_ASSERT(...) ::spice::detail::spice_assert(__FILE__, __LINE__, __VA_ARGS__)

namespace spice::detail {
void spice_assert(std::string const& file, int const line, bool const condition,
                  std::string const& message = "");
}