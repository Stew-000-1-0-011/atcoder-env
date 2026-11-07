#pragma once

#include <concepts>
#include <iostream>
#include <tuple>
#include <utility>
namespace stew::input_util {
	struct InputMarker {};

	template <class T_> struct Z final : InputMarker {
		using T = T_;
		static auto after_input(T_&& v) noexcept -> T_ { return v - 1; }
	};

	template <class T_> inline auto input() noexcept {
		if constexpr(std::derived_from<T_, InputMarker>) {
			typename T_::T v;
			std::cin >> v;
			return T_::after_input(std::move(v));
		} else {
			T_ v;
			std::cin >> v;
			return v;
		}
	}

	template <class... Args_> inline auto inputs() noexcept {
		return std::tuple{input<Args_>()...};
	}
} // namespace stew::input_util

// stew:test-begin (expand.py はここから stew:test-end までを取り除く)
#include <sstream>
#include <string>
#include "utility.hpp"

namespace stew::input_util {
	inline void test() {
		using utility::test_check;

		std::istringstream iss{"3 5 abc\n7 8 9\n"};
		auto* const old = std::cin.rdbuf(iss.rdbuf());

		test_check(input<int>() == 3);
		test_check(input<Z<int>>() == 4);  // 1-indexed -> 0-indexed
		test_check(input<std::string>() == "abc");
		const auto [a, b, c] = inputs<int, Z<int>, long>();
		test_check(a == 7 && b == 7 && c == 9);

		std::cin.rdbuf(old);
	}
} // namespace stew::input_util
// stew:test-end
