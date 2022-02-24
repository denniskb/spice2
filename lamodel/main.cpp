#include "spice/detail/assert.h"

int main() {
	SPICE_ASSERT(false, "This should crash");
	return 0;
}