#pragma once

#pragma GCC system_header
#include "matplot/matplot.h"

#include <thread>

namespace matplot {
inline void pause(double s) {
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(s * 1000)));
}
}