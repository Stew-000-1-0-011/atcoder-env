#pragma once

#include <cstddef>

#include <optional>
#include <utility>
#include <vector>

#include "integer_type.hpp"

namespace stew::union_find {

	template <class T_ = void>
	struct UnionFind final {
		std::vector<usize> root;
		std::vector<usize> size;
		std::vector<T_> data;

		constexpr UnionFind(std::vector<T_>&& data) noexcept
			: root(data.size()), size(data.size(), 1), data(std::move(data)) {
			for(usize i = 0; i < this->data.size(); ++i) {
				this->root[i] = i;
			}
		}

		constexpr auto get(const usize x) const noexcept -> const T_& {
			return this->data[this->find(x)];
		}

		constexpr auto get(const usize x) noexcept -> T_& {
			return this->data[this->find(x)];
		}

		constexpr auto find(const usize x) noexcept -> usize {
			if(this->root[x] == x) {
				return x;
			} else {
				return this->root[x] = this->find(this->root[x]);
			}
		}

		constexpr auto unite(
			usize x, usize y, auto&& f, bool call_when_same
		) noexcept -> std::optional<usize>
			requires requires(T_ t) {
				{ f(t, t) } -> std::same_as<T_>;
			}
		{
			x = this->find(x);
			y = this->find(y);
			if(x == y) {
				if(call_when_same) {
					this->data[x] = f(this->data[x], this->data[y]);
				}
				return std::nullopt;
			}
			if(this->size[x] < this->size[y]) std::swap(x, y);
			this->root[y] = x;
			this->size[x] += this->size[y];
			this->data[x] = f(this->data[x], this->data[y]);
			return x;
		}
	};

	template <>
	struct UnionFind<void> final {
		std::vector<usize> root;
		std::vector<usize> size;

		constexpr UnionFind(const usize n) noexcept : root(n), size(n, 1) {
			for(usize i = 0; i < n; ++i) {
				this->root[i] = i;
			}
		}

		constexpr auto find(const usize x) noexcept -> usize {
			if(this->root[x] == x) {
				return x;
			} else {
				return this->root[x] = this->find(this->root[x]);
			}
		}

		constexpr auto unite(usize x, usize y) noexcept
			-> std::optional<usize> {
			x = this->find(x);
			y = this->find(y);
			if(x == y) {
				return std::nullopt;
			}
			if(this->size[x] < this->size[y]) std::swap(x, y);
			this->root[y] = x;
			this->size[x] += this->size[y];
			return x;
		}
	};
} // namespace stew