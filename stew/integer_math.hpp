#pragma once

#include <array>
#include <concepts>
#include <numeric>
#include <tuple>
#include <vector>

#include "integer_type.hpp"

namespace stew::integer_math {

	// n -> (log2(n)の切り捨て, 余り)
	// O(log n)
	inline constexpr auto log2(const std::unsigned_integral auto n)
		-> std::tuple<decltype(n), decltype(n)> {
		using T = decltype(n);
		T result = 0;
		for(T i = n; i > 1; i >>= 1) {
			++result;
		}
		return {result, n - (T(1) << result)};
	}

	template <std::unsigned_integral T_, std::unsigned_integral U_ = T_>
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
	template <std::unsigned_integral T_, std::unsigned_integral U_ = T_>
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
		if(n == 1) return ret;
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