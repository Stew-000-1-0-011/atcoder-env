#pragma once

#include <concepts>
#include "utility.hpp"

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

namespace stew::monoid {
	namespace test_detail {
		struct Sum {
			long long v;
		};
	} // namespace test_detail

	template <> struct Monoid<test_detail::Sum> {
		static auto op(const test_detail::Sum& l, const test_detail::Sum& r)
			-> test_detail::Sum {
			return {l.v + r.v};
		}
		static auto ide() -> test_detail::Sum { return {0}; }
	};

	inline void test() {
		using utility::test_check;
		using test_detail::Sum;

		static_assert(monoid<Sum>);
		test_check(Monoid<Sum>::op(Sum{2}, Sum{3}).v == 5);
		test_check(Monoid<Sum>::op(Monoid<Sum>::ide(), Sum{7}).v == 7);
	}
} // namespace stew::monoid
