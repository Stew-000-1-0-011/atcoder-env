#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "integer_type.hpp"
#include "monoid.hpp"
#include "utility.hpp"

namespace stew::swag::impl {
	using monoid::Monoid;

	template <monoid::monoid T_>
	struct Swag final {
		using M = Monoid<T_>;

		std::vector<std::pair<T_, T_>> back_{{M::ide(), M::ide()}};
		std::vector<std::pair<T_, T_>> front_{{M::ide(), M::ide()}};
		std::vector<T_> buf{};

		constexpr auto size() const noexcept -> usize {
			return this->back_.size() + this->front_.size() - 2;
		}

		constexpr auto empty() const noexcept -> bool {
			return this->size() == 0;
		}

		constexpr auto back(this auto&& self) noexcept {
			if(self.back_.size() != 1) {
				return self.back_.back().first;
			} else {
				return self.front_[1].first;
			}
		}

		constexpr auto front(this auto&& self) noexcept {
			if(self.front_.size() != 1) {
				return self.front_.back().first;
			} else {
				return self.back_[1].first;
			}
		}

		constexpr void push_back(univ_ref<T_> auto&& x_) {
			T_ x{std::forward<decltype(x_)>(x_)};

			const auto& [_, acc_] = this->back_.back();
			T_ acc = M::op(acc_, x);
			this->back_.emplace_back(std::move(x), std::move(acc));
		}

		constexpr void push_front(univ_ref<T_> auto&& x_) {
			T_ x{std::forward<decltype(x_)>(x_)};

			const auto& [_, acc_] = this->front_.back();
			T_ acc = M::op(x, acc_);
			this->front_.emplace_back(std::move(x), std::move(acc));
		}

		constexpr auto pop_back() noexcept -> T_ {
			if(this->back_.size() == 1 && this->front_.size() == 1) {
				throw std::runtime_error{"panic! pop_back from empty swag."};
			}

			if(this->back_.size() == 1) {
				const usize k = this->front_.size() - 1;
				const usize moved = (k - 1) / 2;
				const usize stay = k - 1 - moved;

				// 先頭~約半分をbufに退避
				this->buf.clear();
				for(usize i = 0; i < stay; ++i) {
					auto x = std::move(this->front_.back().first);
					this->buf.emplace_back(std::move(x));
					this->front_.pop_back();
				}

				// 約半分~末尾-1をbackに追加
				for(usize i = 0; i < moved; ++i) {
					auto x = std::move(this->front_.back().first);
					this->push_back(std::move(x));
					this->front_.pop_back();
				}

				// 末尾をpop
				auto ret = std::move(this->front_.back().first);
				this->front_.pop_back();

				// bufの退避を戻す
				for(usize i = 0; i < stay; ++i) {
					this->push_front(std::move(this->buf.back()));
					this->buf.pop_back();
				}

				this->buf.clear();
				return ret;
			} else {
				auto ret = std::move(this->back_.back().first);
				this->back_.pop_back();
				return ret;
			}
		}

		constexpr auto pop_front() noexcept -> T_ {
			if(this->back_.size() == 1 && this->front_.size() == 1) {
				throw std::runtime_error{"panic! pop_front from empty swag."};
			}

			if(this->front_.size() == 1) {
				const usize k = this->back_.size() - 1;
				const usize moved = (k - 1) / 2;
				const usize stay = k - 1 - moved;

				// 末尾~約半分をbufに退避
				this->buf.clear();
				for(usize i = 0; i < stay; ++i) {
					auto x = std::move(this->back_.back().first);
					this->buf.emplace_back(std::move(x));
					this->back_.pop_back();
				}

				// 約半分~先頭-1をfrontに追加
				for(usize i = 0; i < moved; ++i) {
					auto x = std::move(this->back_.back().first);
					this->push_front(std::move(x));
					this->back_.pop_back();
				}

				// 先頭をpop
				auto ret = std::move(this->back_.back().first);
				this->back_.pop_back();

				// bufの退避を戻す
				for(usize i = 0; i < stay; ++i) {
					this->push_back(std::move(this->buf.back()));
					this->buf.pop_back();
				}

				this->buf.clear();
				return ret;
			} else {
				auto ret = std::move(this->front_.back().first);
				this->front_.pop_back();
				return ret;
			}
		}

		constexpr auto fold() const noexcept -> T_ {
			return M::op(this->front_.back().second, this->back_.back().second);
		}
	};
} // namespace stew::swag::impl

namespace stew::swag {
	using impl::Swag;
}