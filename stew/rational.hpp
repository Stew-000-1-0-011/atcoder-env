#pragma once

#include <compare>
#include <numeric>
#include "integer_type.hpp"
#include "utility.hpp"

namespace stew::rational {
	template <class T_> struct Rational final {
		T_ nume{};
		T_ deno{1};

		constexpr auto simp() noexcept -> Rational {
			const auto g = std::gcd(this->nume, this->deno);
			return Rational{this->nume / g, this->deno / g};
		}

		constexpr auto operator+=(const Rational& rhs) noexcept -> Rational& {
			this->nume = nume * rhs.deno + rhs.nume * deno;
			this->deno *= rhs.deno;
			return *this;
		}

		constexpr auto operator-=(const Rational& rhs) noexcept -> Rational& {
			this->nume = nume * rhs.deno - rhs.nume * deno;
			this->deno *= rhs.deno;
			return *this;
		}

		constexpr auto operator*=(const Rational& rhs) noexcept -> Rational& {
			this->nume *= rhs.nume;
			this->deno *= rhs.deno;
			return *this;
		}

		constexpr auto operator/=(const Rational& rhs) noexcept -> Rational& {
			this->nume *= rhs.deno;
			this->deno *= rhs.nume;
			return *this;
		}

		friend auto operator+(const Rational& lhs, const Rational& rhs) noexcept
			-> Rational {
			return Rational{lhs} += rhs;
		}

		friend auto operator-(const Rational& lhs, const Rational& rhs) noexcept
			-> Rational {
			return Rational{lhs} -= rhs;
		}

		friend auto operator*(const Rational& lhs, const Rational& rhs) noexcept
			-> Rational {
			return Rational{lhs} *= rhs;
		}

		friend auto operator/(const Rational& lhs, const Rational& rhs) noexcept
			-> Rational {
			return Rational{lhs} /= rhs;
		}

		friend auto operator-(const Rational& rhs) noexcept -> Rational {
			return Rational{-rhs.nume, rhs.deno};
		}

		friend auto operator<=>(
			const Rational& lhs, const Rational& rhs
		) noexcept -> std::strong_ordering {
			return lhs.nume * rhs.deno <=> rhs.nume * lhs.deno;
		}

		friend auto
		operator==(const Rational& lhs, const Rational& rhs) noexcept -> bool {
			return lhs.nume * rhs.deno == rhs.nume * lhs.deno;
		}
	};
} // namespace stew::rational

namespace stew::rational {
	inline void test() {
		using utility::test_check;
		using R = Rational<i64>;

		test_check(R{1, 2} + R{1, 3} == R{5, 6});
		test_check(R{1, 2} - R{1, 3} == R{1, 6});
		test_check(R{2, 3} * R{3, 4} == R{1, 2});
		test_check(R{1, 2} / R{1, 4} == R{2, 1});
		test_check(-R{1, 2} == R{-1, 2});
		test_check(R{2, 4} == R{1, 2});
		test_check(R{1, 3} < R{1, 2});
		test_check(R{-1, 2} < R{1, 3});
		{
			const R s = R{6, 8}.simp();
			test_check(s.nume == 3 && s.deno == 4);
		}
		{
			const R s = R{0, 5}.simp();
			test_check(s.nume == 0 && s.deno == 1);
		}

		// 割り算で分母が負になる場合 (1 / (-2) = -1/2)
		const R neg = R{1, 1} / R{-2, 1};
		test_check(neg == R{-1, 2});
		test_check(neg < R{0, 1});
		test_check(neg < R{1, 1});
	}
} // namespace stew::rational
