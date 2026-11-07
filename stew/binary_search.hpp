#pragma once

#include <concepts>
#include <utility>

#include "integer_type.hpp"

namespace stew::binary_search {
	template <
		class T_,
		class MiddleF_ = decltype([]<class Int_>
				requires std::integral<Int_>
			(const Int_& l, const Int_& r) { return (l + r) / 2; })
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
		// 現状、binary_search を呼ぶと clang 21 がクラッシュする (g++ 15 では通る)。
		// テンプレート引数 MiddleF_ の既定値が「制約付きのジェネリックラムダ」の decltype であることが原因で、
		// ここで呼び出すと default.hpp を使うすべてのビルドが壊れるため、呼び出しのテストは置いていない。
		// 直したら次を有効にする:
		//   binary_search(i64(0), i64(100), [](i64 x) { return x * x <= 50; }, true) == {7, 8}
		//   binary_search(i64(0), i64(100), [](i64 x) { return x * x > 50; }, false) == {7, 8}
		//   binary_search(i64(-100), i64(0), [](i64 x) { return x <= -37; }, true) == {-37, -36}
		//   binary_search(i64(0), i64(10), [](i64) { return true; }, true) == {9, 10}
	}
} // namespace stew::binary_search
