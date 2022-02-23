#pragma once

#include <cstdio>

#include "stdint.h"

namespace spice {
inline void say_hello() { puts("Hello World\n"); }

inline int return1() { return 1; }
} // namespace spice