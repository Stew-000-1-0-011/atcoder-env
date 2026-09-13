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