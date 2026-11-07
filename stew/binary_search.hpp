#pragma once

#include <concepts>
#include <numeric>
#include <utility>

#include "integer_type.hpp"

namespace stew::binary_search {
	// 既定の中間値。整数以外では呼べないので、binary_search の制約で弾かれる。
	// (制約付きジェネリックラムダの decltype を既定値にすると clang がクラッシュするため、名前付きの型にしている)
	// std::midpoint は l + r が桁あふれする場合も正しく、l 側に丸める (r - l >= 2 なら l < mid < r)
	struct DefaultMiddle {
		template <std::integral Int_>
		constexpr auto operator()(const Int_& l, const Int_& r) const -> Int_ {
			return std::midpoint(l, r);
		}
	};

	template <
		class T_,
		class MiddleF_ = DefaultMiddle
		>
		inline auto
		binary_search(
			const T_& left,
			const T_& right,
			const auto& pred,
			const bool left_is,
			const MiddleF_& middle = {}
		) -> std::pair<T_, T_>
			requires requires(const T_ v) {
				{ pred(v) } -> std::convertible_to<bool>;
				{ middle(left, right) } -> std::convertible_to<T_>;
			}
	{
		T_ l = left;
		T_ r = right;
		while(r - l > 1) {
			const T_ mid = middle(l, r);
			if(pred(mid) ^ left_is) {
				r = mid;
			} else {
				l = mid;
			}
		}
		return {l, r};
	}
} // namespace stew::binary_search

// stew:test-begin (expand.py はここから stew:test-end までを取り除く)
#include <limits>
#include "utility.hpp"

namespace stew::binary_search {
	inline void test() {
		using utility::test_check;

		// x * x <= 50 を満たす最大の x と、満たさない最小の x
		{
			const auto [l, r] = binary_search(
				i64(0), i64(100), [](const i64 x) { return x * x <= 50; }, true
			);
			test_check(l == 7 && r == 8);
		}
		// 左側で偽になる述語
		{
			const auto [l, r] = binary_search(
				i64(0), i64(100), [](const i64 x) { return x * x > 50; }, false
			);
			test_check(l == 7 && r == 8);
		}
		// 負の範囲
		{
			const auto [l, r] = binary_search(
				i64(-100), i64(0), [](const i64 x) { return x <= -37; }, true
			);
			test_check(l == -37 && r == -36);
		}
		// 区間全体で真
		{
			const auto [l, r] = binary_search(
				i64(0), i64(10), [](const i64) { return true; }, true
			);
			test_check(l == 9 && r == 10);
		}
		// 両端が大きくても桁あふれしない
		{
			constexpr i64 big = std::numeric_limits<i64>::max();
			const auto [l, r] = binary_search(
				big - 100, big, [](const i64 x) { return x <= big - 37; }, true
			);
			test_check(l == big - 37 && r == big - 36);
		}
		{
			constexpr i64 small = std::numeric_limits<i64>::min();
			const auto [l, r] = binary_search(
				small, small + 100, [](const i64 x) { return x <= small + 37; }, true
			);
			test_check(l == small + 37 && r == small + 38);
		}

		// 中間値を指定する
		{
			i64 calls = 0;
			const auto [l, r] = binary_search(
				i64(0), i64(100), [](const i64 x) { return x <= 30; }, true,
				[&](const i64 a, const i64 b) { ++calls; return (a + b) / 2; }
			);
			test_check(l == 30 && r == 31 && calls > 0);
		}
	}
} // namespace stew::binary_search
// stew:test-end
