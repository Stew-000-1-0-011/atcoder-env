#pragma once

#include <concepts>

namespace stew::monoid {
	template <class T_> struct Monoid {
		static_assert(
			[] { return false; }(), "No Monoid defined for this type"
		);
	};

	template <class T_>
	concept monoid = requires(const T_ imut1, const T_ imut2) {
		{ Monoid<T_>::op(imut1, imut2) } -> std::same_as<T_>;
		{ Monoid<T_>::ide() } -> std::same_as<T_>;
	};
} // namespace stew::monoid