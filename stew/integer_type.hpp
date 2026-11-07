#pragma once

#include <cstddef>
#include <cstdint>

namespace stew {
	namespace integer_type {
		using u8 = std::uint8_t;
		using u16 = std::uint16_t;
		using u32 = std::uint32_t;
		using u64 = std::uint64_t;
		using i8 = std::int8_t;
		using i16 = std::int16_t;
		using i32 = std::int32_t;
		using i64 = std::int64_t;
		using i128 = __int128;
		using u128 = unsigned __int128;
		using usize = std::size_t;
	} // namespace integer_type

	using namespace integer_type;
} // namespace stew

namespace stew::integer_type {
	inline void test() {
		static_assert(sizeof(u8) == 1 && sizeof(i8) == 1);
		static_assert(sizeof(u16) == 2 && sizeof(i16) == 2);
		static_assert(sizeof(u32) == 4 && sizeof(i32) == 4);
		static_assert(sizeof(u64) == 8 && sizeof(i64) == 8);
		static_assert(sizeof(i128) == 16 && sizeof(u128) == 16);
		static_assert(u128(-1) > 0);
		static_assert(i128(-1) < 0);
		static_assert(u64(-1) > 0);
		static_assert(sizeof(usize) == sizeof(void*));
	}
} // namespace stew::integer_type
