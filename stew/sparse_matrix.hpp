#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <ranges>
#include <utility>
#include <vector>

#include "integer_type.hpp"
#include "utility.hpp"

namespace stew::sparse_matrix {
	namespace impl {
		// Csr の行を順に返すイテレータ。
		// Csr へのポインタと行番号だけを持つので、begin() / end() の呼び出しをまたいでも有効。
		template <class Csr_>
		struct RowIterator final {
			using difference_type = std::ptrdiff_t;

			Csr_* csr;
			usize index;

			auto operator*() const {
				if constexpr(std::is_const_v<Csr_>) {
					return this->csr->c_row(this->index);
				} else {
					return this->csr->row(this->index);
				}
			}

			auto operator++() -> RowIterator& {
				++this->index;
				return *this;
			}

			auto operator++(int) -> RowIterator {
				auto old = *this;
				++this->index;
				return old;
			}

			friend auto operator==(const RowIterator&, const RowIterator&)
				-> bool = default;
		};
	} // namespace impl

	template <class E_ = void>
	struct Csr final {
		std::vector<E_> data;
		std::vector<usize> acc_row;
		std::vector<usize> col;

		auto rows() const -> usize {
			return this->acc_row.empty() ? 0 : this->acc_row.size() - 1;
		}

		auto row(const usize index) {
			auto f = [this](const usize i) -> std::pair<usize, E_&> {
				return {this->col[i], this->data[i]};
			};

			if(index >= this->rows()) {
				return std::views::iota(usize(0), usize(0)) |
					std::views::transform(std::move(f));
			} else {
				return std::views::iota(
						   this->acc_row[index], this->acc_row[index + 1]
					   ) |
					std::views::transform(std::move(f));
			}
		}

		auto c_row(const usize index) const {
			auto f = [this](const usize i) -> std::pair<usize, const E_&> {
				return {this->col[i], this->data[i]};
			};

			if(index >= this->rows()) {
				return std::views::iota(usize(0), usize(0)) |
					std::views::transform(std::move(f));
			} else {
				return std::views::iota(
						   this->acc_row[index], this->acc_row[index + 1]
					   ) |
					std::views::transform(std::move(f));
			}
		}

		auto view() {
			return std::views::iota(usize(0), this->rows()) |
				std::views::transform([this](const usize i) {
					return this->row(i);
				});
		}

		auto c_view() const {
			return std::views::iota(usize(0), this->rows()) |
				std::views::transform([this](const usize i) {
					return this->c_row(i);
				});
		}

		using iterator = impl::RowIterator<Csr>;
		using const_iterator = impl::RowIterator<const Csr>;

		auto begin() -> iterator { return {this, 0}; }
		auto end() -> iterator { return {this, this->rows()}; }
		auto begin() const -> const_iterator { return {this, 0}; }
		auto end() const -> const_iterator { return {this, this->rows()}; }
		auto cbegin() const -> const_iterator { return {this, 0}; }
		auto cend() const -> const_iterator { return {this, this->rows()}; }
	};

	template <>
	struct Csr<> final {
		std::vector<usize> acc_row;
		std::vector<usize> col;

		auto rows() const -> usize {
			return this->acc_row.empty() ? 0 : this->acc_row.size() - 1;
		}

		auto row(const usize index) {
			auto f = [this](const usize i) -> usize { return this->col[i]; };

			if(index >= this->rows()) {
				return std::views::iota(usize(0), usize(0)) |
					std::views::transform(std::move(f));
			} else {
				return std::views::iota(
						   this->acc_row[index], this->acc_row[index + 1]
					   ) |
					std::views::transform(std::move(f));
			}
		}

		auto c_row(const usize index) const {
			auto f = [this](const usize i) -> usize { return this->col[i]; };

			if(index >= this->rows()) {
				return std::views::iota(usize(0), usize(0)) |
					std::views::transform(std::move(f));
			} else {
				return std::views::iota(
						   this->acc_row[index], this->acc_row[index + 1]
					   ) |
					std::views::transform(std::move(f));
			}
		}

		auto view() {
			return std::views::iota(usize(0), this->rows()) |
				std::views::transform([this](const usize i) {
					return this->row(i);
				});
		}

		auto c_view() const {
			return std::views::iota(usize(0), this->rows()) |
				std::views::transform([this](const usize i) {
					return this->c_row(i);
				});
		}

		using iterator = impl::RowIterator<Csr>;
		using const_iterator = impl::RowIterator<const Csr>;

		auto begin() -> iterator { return {this, 0}; }
		auto end() -> iterator { return {this, this->rows()}; }
		auto begin() const -> const_iterator { return {this, 0}; }
		auto end() const -> const_iterator { return {this, this->rows()}; }
		auto cbegin() const -> const_iterator { return {this, 0}; }
		auto cend() const -> const_iterator { return {this, this->rows()}; }
	};

	template <class E_ = void>
	struct Coo final {
		std::vector<std::pair<std::pair<usize, usize>, E_>> buffer;
		bool is_sorted = false;

		static auto make(const usize buffer_size) {
			auto buffer = std::vector<std::pair<std::pair<usize, usize>, E_>>{};
			buffer.reserve(buffer_size);
			return Coo<E_>{.buffer = std::move(buffer), .is_sorted = false};
		}

		void add(const std::pair<usize, usize>& index, E_&& elem) {
			this->is_sorted = false;
			this->buffer.emplace_back(
				std::pair<std::pair<usize, usize>, E_>{index, std::move(elem)}
			);
		}

		void sort() {
			std::ranges::sort(this->buffer, {}, [](const auto& x) { return x.first; });
			this->is_sorted = true;
		}

		auto to_csr(const usize row_size) && -> Csr<E_>
		// [[expect: indices are sorted by row and then by column]]
		{
			if(not this->is_sorted) {
				this->sort();
			}

			std::vector<E_> data{};
			std::vector<usize> acc_row{};
			std::vector<usize> col{};

			data.reserve(this->buffer.size());
			acc_row.reserve(row_size + 1);
			col.reserve(this->buffer.size());

			for(auto& [index, elem] : this->buffer) {
				data.emplace_back(std::move(elem));
				col.emplace_back(index.second);
			}

			auto it = this->buffer.begin();
			for(usize i = 0; i < row_size; ++i) {
				acc_row.push_back(it - this->buffer.begin());
				while(it != this->buffer.end() && it->first.first == i) {
					++it;
				}
			}
			acc_row.push_back(this->buffer.size());

			return Csr<E_>{
				.data = std::move(data),
				.acc_row = std::move(acc_row),
				.col = std::move(col)
			};
		}
	};

	template <>
	struct Coo<> final {
		std::vector<std::pair<usize, usize>> buffer;
		bool is_sorted = false;

		static auto make(const usize buffer_size) {
			auto buffer = std::vector<std::pair<usize, usize>>{};
			buffer.reserve(buffer_size);
			return Coo<>{.buffer = std::move(buffer), .is_sorted = false};
		}

		void add(const std::pair<usize, usize>& index) {
			this->is_sorted = false;
			this->buffer.push_back(index);
		}

		void sort() {
			std::ranges::sort(this->buffer);
			this->is_sorted = true;
		}

		auto to_csr(const usize row_size) && -> Csr<> {
			if(not this->is_sorted) {
				this->sort();
			}

			std::vector<usize> acc_row{};
			std::vector<usize> col{};

			acc_row.reserve(row_size + 1);
			col.reserve(this->buffer.size());

			for(const auto& index : this->buffer) {
				col.push_back(index.second);
			}

			auto it = this->buffer.begin();
			for(usize i = 0; i < row_size; ++i) {
				acc_row.push_back(it - this->buffer.begin());
				while(it != this->buffer.end() && it->first == i) {
					++it;
				}
			}
			acc_row.push_back(this->buffer.size());

			return Csr<>{.acc_row = std::move(acc_row), .col = std::move(col)};
		}
	};
} // namespace stew::sparse_matrix

namespace stew::sparse_matrix {
	inline void test() {
		using utility::test_check;

		const auto to_vec = [](auto&& r) {
			std::vector<usize> v{};
			for(const auto c : r) v.push_back(c);
			return v;
		};

		// 要素なし
		{
			auto coo = Coo<>::make(3);
			coo.add({1, 2});
			coo.add({0, 1});
			coo.add({1, 0});
			auto csr = std::move(coo).to_csr(3);
			test_check(csr.acc_row == std::vector<usize>{0, 1, 3, 3});
			test_check(csr.col == std::vector<usize>{1, 0, 2});
			test_check(to_vec(csr.row(0)) == std::vector<usize>{1});
			test_check(to_vec(csr.row(1)) == std::vector<usize>{0, 2});
			test_check(to_vec(csr.row(2)).empty());
			test_check(to_vec(csr.row(5)).empty());  // 範囲外の行は空
			test_check(to_vec(std::as_const(csr).c_row(1)) == std::vector<usize>{0, 2});

			// range-for で全行を回す
			usize rows = 0;
			usize sum = 0;
			for(auto r : csr) {
				++rows;
				for(const auto c : r) sum += c;
			}
			test_check(rows == 3 && sum == 3);
		}

		// 要素あり
		{
			auto coo = Coo<i64>::make(3);
			coo.add({1, 0}, 10);
			coo.add({0, 2}, 20);
			coo.add({0, 1}, 30);
			auto csr = std::move(coo).to_csr(2);
			test_check(csr.acc_row == std::vector<usize>{0, 2, 3});
			test_check(csr.col == std::vector<usize>{1, 2, 0});
			test_check(csr.data == std::vector<i64>{30, 20, 10});
			for(auto p : csr.row(0)) p.second += 1;  // 要素は参照で返る
			test_check(csr.data == std::vector<i64>{31, 21, 10});
		}

		// const な Csr を range-for で回す
		{
			auto coo = Coo<>::make(2);
			coo.add({0, 1});
			coo.add({2, 0});
			const auto csr = std::move(coo).to_csr(3);
			std::vector<usize> sizes{};
			for(const auto r : csr) sizes.push_back(to_vec(r).size());
			test_check(sizes == std::vector<usize>{1, 0, 1});
			test_check(csr.rows() == 3);
		}

		// 既定構築 (行なし)
		{
			const Csr<> empty{};
			test_check(empty.rows() == 0);
			test_check(to_vec(empty.c_row(0)).empty());
			usize n = 0;
			for(const auto r : empty) n += to_vec(r).size() + 1;
			test_check(n == 0);
		}

		// ムーブ専用の要素
		{
			auto coo = Coo<std::unique_ptr<int>>::make(2);
			coo.add({1, 0}, std::make_unique<int>(5));
			coo.add({0, 0}, std::make_unique<int>(7));
			auto csr = std::move(coo).to_csr(2);
			test_check(*csr.data[0] == 7 && *csr.data[1] == 5);
		}
	}
} // namespace stew::sparse_matrix
