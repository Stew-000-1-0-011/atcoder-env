#pragma once

#include <concepts>
#include <iostream>
#include <tuple>
#include <utility>
namespace stew::input_util {
	namespace impl {
		// このヘッダを include すると、main の前に一度だけ入出力を高速化する。
		//   - sync_with_stdio(false): C の stdio との同期を切り、cin が自前のバッファで読むようにする
		//   - cin.tie(nullptr): 入力のたびに cout を書き出さないようにする
		// その代わり、次のことに注意する:
		//   - 入力で cin と scanf などの C の関数を混ぜない
		//   - 出力で cout と printf / std::println(stdout に書く) を混ぜない (順番が入れ替わることがある)
		//   - インタラクティブな問題では、出力のたびに書き出す (std::fflush(stdout) や std::flush)
		inline const bool fast_io_enabled = [] {
			std::ios::sync_with_stdio(false);
			std::cin.tie(nullptr);
			return true;
		}();
	} // namespace impl

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

		// include しただけで高速化の設定が済んでいる
		test_check(impl::fast_io_enabled);
		test_check(std::cin.tie() == nullptr);
		test_check(!std::ios::sync_with_stdio(false));  // 戻り値は直前の設定
	}
} // namespace stew::input_util
// stew:test-end
