#pragma once

#include <array>
#include <concepts>
#include <numeric>
#include <tuple>
#include <type_traits>
#include <vector>

#include "integer_type.hpp"
#include <utility>

namespace stew::integer_math {

	// T_ 同士の積が収まる型 (u32 以下 -> u64, u64 -> u128)
	// u128 は厳格モードでは std::unsigned_integral を満たさないので、使う側の U_ は制約していない
	template <std::unsigned_integral T_>
	using wide_t = std::conditional_t<(sizeof(T_) <= 4), u64, u128>;

	// n -> (log2(n)の切り捨て, 余り)
	// n >= 1 であること
	// O(log n)
	inline constexpr auto log2(const std::unsigned_integral auto n) {
		using T = std::remove_const_t<decltype(n)>;
		T result = 0;
		for(T i = n; i > 1; i >>= 1) {
			++result;
		}
		return std::tuple<T, T>{result, T(n - (T(1) << result))};
	}

	template <std::unsigned_integral T_, class U_ = wide_t<T_>>
	inline constexpr auto mod_pow(T_ a, T_ n, T_ mod) -> T_ {
		T_ result = 1;
		while(n > 0) {
			if(n & 1) {
				result = (U_(result) * a) % mod;
			}
			a = (U_(a) * a) % mod;
			n >>= 1;
		}
		return result;
	}

	template <std::unsigned_integral T_>
	inline constexpr auto is_prime_4759123140(T_ n) -> bool {
		constexpr std::array<T_, 3> bases{2, 7, 61};

		if(n == 2) {
			return true;
		}
		if(n < 2 || (n & 1) == 0) {
			return false;
		}
		T_ n1 = n - 1;
		T_ d = n - 1;

		T_ s = 0;
		for(; (d & 1) == 0; d >>= 1) s += 1;

		for(const auto& base : bases) {
			T_ a = base;
			if(a >= n) {
				a %= n;
				if(a == 0) continue;
			}

			T_ t = mod_pow(a, d, n);
			if(t == 1) continue;

			for(u32 r = 1; t != n1; ++r) {
				if(s <= r) return false;
				t = mod_pow(t, T_(2), n);
			}
		}
		return true;
	}

	template <std::unsigned_integral T_>
	inline constexpr auto is_prime_u64(T_ n) -> bool {
		constexpr std::array<T_, 7> bases{
			2, 325, 9375, 28178, 450775, 9780504, 1795265022
		};

		if(n == 2) {
			return true;
		}
		if(n < 2 || (n & 1) == 0) {
			return false;
		}
		T_ n1 = n - 1;
		T_ d = n - 1;

		T_ s = 0;
		for(; (d & 1) == 0; d >>= 1) s += 1;

		for(const auto& base : bases) {
			T_ a = base;
			if(a >= n) {
				a %= n;
				if(a == 0) continue;
			}

			T_ t = mod_pow(a, d, n);
			if(t == 1) continue;

			for(u32 r = 1; t != n1; ++r) {
				if(s <= r) return false;
				t = mod_pow(t, T_(2), n);
			}
		}
		return true;
	}

	// Brent's cycle detection による Pollard's Rho
	template <std::unsigned_integral T_, class U_ = wide_t<T_>>
	inline auto brent_rho(T_ n, auto&& rng) -> T_
		requires requires {
			{ rng() } -> std::convertible_to<T_>;
		}
	{
		if((n & 1) == 0) return 2;
		T_ y = rng() % (n - 1) + 1;
		T_ c = rng() % (n - 1) + 1;
		T_ m = 128;
		T_ g = 1, r = 1, q = 1;
		T_ x, ys;

		while(g == 1) {
			x = y;
			for(T_ i = 0; i < r; i++) {
				y = (U_(y) * y % n + c) % n;
			}
			T_ k = 0;
			while(k < r && g == 1) {
				ys = y;
				T_ limit = std::min(m, r - k);
				for(T_ i = 0; i < limit; i++) {
					y = (U_(y) * y % n + c) % n;
					T_ diff = (x > y ? x - y : y - x);
					q = U_(q) * diff % n;
				}
				g = std::gcd(q, n);
				k += limit;
			}
			r <<= 1;
		}

		if(g == n) {
			do {
				ys = (U_(ys) * ys % n + c) % n;
				T_ diff = (x > ys ? x - ys : ys - x);
				g = std::gcd(diff, n);
			} while(g == 1);
		}
		return g;
	}

	// n の素因数を vector<u64> に追加
	inline auto factorize(u64 n, auto&& rng) -> std::vector<std::pair<u64, u64>>
		requires requires {
			{ rng() } -> std::convertible_to<u64>;
		}
	{
		std::vector<std::pair<u64, u64>> ret{};
		if(n <= 1) return ret;  // 0 と 1 は空を返す
		if(is_prime_u64(n)) {
			ret.push_back({n, 1});
			return ret;
		}
		u64 d;
		do {
			d = brent_rho(n, rng);
		} while(d == n);
		const auto ret1 = factorize(d, rng);
		const auto ret2 = factorize(n / d, rng);
		u64 i1 = 0, i2 = 0;
		while(i1 < ret1.size() && i2 < ret2.size()) {
			if(ret1[i1].first == ret2[i2].first) {
				ret.push_back(
					{ret1[i1].first, ret1[i1].second + ret2[i2].second}
				);
				++i1;
				++i2;
			} else if(ret1[i1].first < ret2[i2].first) {
				ret.push_back(ret1[i1]);
				++i1;
			} else {
				ret.push_back(ret2[i2]);
				++i2;
			}
		}
		while(i1 < ret1.size()) {
			ret.push_back(ret1[i1]);
			++i1;
		}
		while(i2 < ret2.size()) {
			ret.push_back(ret2[i2]);
			++i2;
		}
		return ret;
	}
} // namespace stew::integer_math

// stew:test-begin (expand.py はここから stew:test-end までを取り除く)
#include <random>
#include "utility.hpp"

namespace stew::integer_math {
	inline void test() {
		using utility::test_check;

		// log2
		{
			const auto [e, rest] = log2(10u);
			test_check(e == 3 && rest == 2);
		}
		{
			const auto [e, rest] = log2(u64(1));
			test_check(e == 0 && rest == 0);
		}
		{
			const auto [e, rest] = log2(u64(1) << 40 | 5);
			test_check(e == 40 && rest == 5);
		}
		{
			const auto [e, rest] = log2(u8(255));
			test_check(e == 7 && rest == 127);
		}

		// mod_pow
		test_check(mod_pow<u64>(2, 10, 1000) == 24);
		test_check(mod_pow<u64>(5, 0, 7) == 1);
		test_check(mod_pow<u64>(3, 1'000'000'006, 1'000'000'007) == 1);
		// 法が 2^32 以上 (途中の積が 64bit を超える)
		test_check(
			mod_pow<u64>(3, 1'000'000'000'000'000'000, 999'999'999'999'999'989)
			== 531441
		);
		test_check(
			mod_pow<u64>(2, 64, 1'000'000'000'000'000'003) == 446744073709551562
		);

		// 素数判定: 小さい数は試し割りと一致する
		const auto naive_is_prime = [](const u64 n) {
			if(n < 2) return false;
			for(u64 d = 2; d * d <= n; ++d) {
				if(n % d == 0) return false;
			}
			return true;
		};
		for(u64 n = 0; n < 2000; ++n) {
			test_check(is_prime_4759123140<u64>(n) == naive_is_prime(n));
			test_check(is_prime_u64<u64>(n) == naive_is_prime(n));
		}
		// 2^32 より大きい数
		test_check(is_prime_4759123140<u64>(4'294'967'311));
		test_check(!is_prime_4759123140<u64>(4'294'967'297));  // 641 * 6700417
		test_check(is_prime_u64<u64>(4'294'967'311));
		test_check(!is_prime_u64<u64>(4'294'967'297));
		test_check(is_prime_u64<u64>(999'999'999'999'999'989));
		test_check(!is_prime_u64<u64>(999'999'999'999'999'991));
		test_check(is_prime_u64<u64>(1'000'000'000'000'000'003));

		// 素因数分解
		std::mt19937_64 rng{12345};
		using F = std::vector<std::pair<u64, u64>>;
		test_check(factorize(1, rng).empty());
		test_check(factorize(97, rng) == F{{97, 1}});
		test_check(factorize(360, rng) == F{{2, 3}, {3, 2}, {5, 1}});
		test_check(
			factorize(4'294'967'295, rng)
			== F{{3, 1}, {5, 1}, {17, 1}, {257, 1}, {65537, 1}}
		);
		test_check(factorize(0, rng).empty());
		test_check(factorize(999'999'999'999'999'989, rng) == F{{999'999'999'999'999'989, 1}});
		test_check(
			factorize(600'851'475'143, rng) == F{{71, 1}, {839, 1}, {1471, 1}, {6857, 1}}
		);
		test_check(factorize(1'000'000'000'000'000'000, rng) == F{{2, 18}, {5, 18}});
		test_check(
			factorize(998'244'353ull * 1'000'000'007ull, rng)
			== F{{998'244'353, 1}, {1'000'000'007, 1}}
		);
		test_check(factorize(4'294'967'311ull * 3, rng) == F{{3, 1}, {4'294'967'311, 1}});
	}
} // namespace stew::integer_math
// stew:test-end
