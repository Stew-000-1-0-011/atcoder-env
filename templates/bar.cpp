#include "default.hpp"
#include <print>
#include <random>

using namespace stew;

int main() {
	std::uniform_int_distribution<i64> ndist{0, 1000000};
	std::uniform_int_distribution<i64> bdist{0, 1000000};

	auto e = std::random_device{};

	const auto n = ndist(e);
	const auto b = bdist(e);
	std::println("{} {}", n, b);
}