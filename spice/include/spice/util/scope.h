#pragma once

#include <functional>

namespace spice::util {
class scope_exit {
public:
	scope_exit(std::function<void()> fn) : _fn(std::move(fn)) {}
	~scope_exit() { _fn(); }

private:
	std::function<void()> _fn;
};
}