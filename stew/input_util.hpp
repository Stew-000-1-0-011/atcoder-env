#pragma once

#include <concepts>
#include <iostream>
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
