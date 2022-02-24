#include "assert.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace spice::detail {
void assert_failed(char const* _file, int const line, char const* condition) {
	using std::string;

	const string file(_file);
	std::stringstream error;
	error << "Assertion failed (" << file.substr(file.find_last_of('/') + 1, file.length()) << ":"
	      << line << "): " << condition;

	throw std::logic_error(error.str());
}
}
