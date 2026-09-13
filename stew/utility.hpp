#pragma once

#include <concepts>
#include <type_traits>
namespace stew::utility {
	template <class T_, class U_>
	concept univ_ref = std::same_as<std::remove_cvref_t<T_>, U_>;

	inline void debug() {
		volatile int x = 0;
		x = x + 1;
	}
}

namespace stew {
	using utility::univ_ref;
}