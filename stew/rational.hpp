#pragma once

#include <compare>
#include <numeric>

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
