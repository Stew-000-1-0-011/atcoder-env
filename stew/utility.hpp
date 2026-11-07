#pragma once

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <cstdio>
#include <print>
#include <source_location>
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

namespace stew::utility {
	// テストの失敗数
	inline auto test_failures() noexcept -> std::size_t& {
		static std::size_t count = 0;
		return count;
	}

	// cond が偽なら失敗として数え、呼び出し位置を表示する
	inline void test_check(
		const bool cond,
		const std::source_location loc = std::source_location::current()
	) {
		if(!cond) {
			++test_failures();
			std::println(
				stderr, "FAILED: {}:{} ({})",
				loc.file_name(), loc.line(), loc.function_name()
			);
		}
	}

	inline void test() {
		static_assert(univ_ref<int, int>);
		static_assert(univ_ref<const int&, int>);
		static_assert(univ_ref<int&&, int>);
		static_assert(!univ_ref<long, int>);
		debug();
	}
} // namespace stew::utility
