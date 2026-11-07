#pragma once

#include <cstddef>

#include <optional>
#include <utility>
#include <vector>

#include "integer_type.hpp"
#include "utility.hpp"

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
} // namespace stew::union_find

namespace stew::union_find {
	inline void test() {
		using utility::test_check;

		{
			UnionFind<> uf(5);
			test_check(uf.unite(0, 1).has_value());
			test_check(uf.unite(3, 4).has_value());
			test_check(!uf.unite(1, 0).has_value());
			test_check(uf.find(0) == uf.find(1));
			test_check(uf.find(3) == uf.find(4));
			test_check(uf.find(0) != uf.find(3));
			test_check(uf.find(2) == 2);
			test_check(uf.unite(1, 4).has_value());
			test_check(uf.find(0) == uf.find(3));
			test_check(uf.size[uf.find(0)] == 4);
		}

		{
			const auto plus = [](const i64 a, const i64 b) { return a + b; };
			UnionFind<i64> uf(std::vector<i64>{1, 2, 3});
			uf.unite(0, 1, plus, false);
			test_check(uf.get(0) == 3 && uf.get(1) == 3);
			uf.unite(0, 1, plus, false);  // 同じ集合なので何もしない
			test_check(uf.get(0) == 3);
			uf.unite(1, 0, plus, true);  // 同じ集合でも f を呼ぶ
			test_check(uf.get(0) == 6);
			test_check(uf.get(2) == 3);
		}

	}
} // namespace stew::union_find
