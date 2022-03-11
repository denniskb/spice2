#include "matplot.h"

#include <thread>

namespace matplot {
void pause(double s) { std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(s * 1000))); }
}