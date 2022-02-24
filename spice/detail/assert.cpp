#include "assert.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace spice::detail {
void spice_assert(std::string const& file, int const line, bool const condition,
                  std::string const& message) {
	using std::string;

	if (!condition) {
		std::stringstream error;
		error << "Assertion failed (" << file.substr(file.find_last_of('/') + 1, file.length())
		      << ":" << line << ")";

		if (!message.empty())
			error << ": " << message;

		throw std::logic_error(error.str());
	}
}
}
