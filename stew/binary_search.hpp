#pragma once

#include <concepts>
#include <utility>

#include "integer_type.hpp"
#include "utility.hpp"

namespace stew::binary_search {
	// 既定の中間値。整数以外では呼べないので、binary_search の制約で弾かれる。
	// (制約付きジェネリックラムダの decltype を既定値にすると clang がクラッシュするため、名前付きの型にしている)
	struct DefaultMiddle {
		template <std::integral Int_>
		constexpr auto operator()(const Int_& l, const Int_& r) const {
			return (l + r) / 2;
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
