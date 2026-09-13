// ===== BEGIN "make_default.hpp"  (from ルート(入力ファイル)) =====
// ===== BEGIN "includes.hpp"  (from make_default.hpp) =====

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <concepts>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <numbers>
#include <optional>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
// ===== END   "includes.hpp" =====

// ===== BEGIN "integer_type.hpp"  (from make_default.hpp) =====

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
		using usize = std::size_t;
	} // namespace integer_type

	using namespace integer_type;
} // namespace stew
// ===== END   "integer_type.hpp" =====
// ===== BEGIN "utility.hpp"  (from make_default.hpp) =====

#include <concepts>
#include <type_traits>
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
// ===== END   "utility.hpp" =====

// ===== BEGIN "acc_sum.hpp"  (from make_default.hpp) =====

#include <bit>
#include <vector>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: acc_sum.hpp) ---

namespace stew::acc_sum {
	template <
		class T_,
		usize dim_,
		class Op_ = decltype([](const T_& l, const T_& r) noexcept -> T_ {
			return l + r;
		}),
		class Neg_ = decltype([](const T_& x) noexcept -> T_ { return -x; }),
		class Ide_ = decltype([]() noexcept -> T_ { return T_{}; })
	>
	inline void do_acc(
		std::vector<T_>& acc,
		const std::array<usize, dim_>& shape,
		const Op_& op = {},
		const Neg_& neg = {},
		const Ide_& ide = {}
	) {
		std::array<usize, dim_> stride{};
		stride.back() = 1;
		usize plusone = 1;
		for(usize id = dim_ - 1; id-- > 0;) {
			stride[id] = stride[id + 1] * shape[id + 1];
			plusone += stride[id];
		}

		const auto helper = [&](this const auto& self,
								const usize idx,
								const usize lev) noexcept -> void {
			if(lev == dim_) {
				T_ a = ide();

				for(u64 ib = 0; ib < (u64(1) << dim_) - 1; ++ib) {
					usize diff = 0;
					for(usize id = 0; id < dim_; ++id) {
						diff += ((ib >> id) & 1) ? stride[id] : 0;
					}
					a =
						op(a,
							(std::popcount(ib) % 2 == dim_ % 2
									? neg(acc[idx + diff])
									: acc[idx + diff]));
				}

				acc[idx + plusone] = op(acc[idx + plusone], a);
			} else {
				for(usize is = 0; is < shape[lev] - 1; ++is) {
					self(idx + is * stride[lev], lev + 1);
				}
			}
		};
		helper(0, 0);
	}

	template <
		class T_,
		usize dim_,
		class Op_ = decltype([](const T_& l, const T_& r) noexcept -> T_ {
			return l + r;
		}),
		class Neg_ = decltype([](const T_& x) noexcept -> T_ { return -x; }),
		class Ide_ = decltype([]() noexcept -> T_ { return T_{}; })
	>
	inline auto query_acc(
		std::vector<T_>& acc,
		const std::array<usize, dim_>& shape,
		const std::array<usize, dim_>& open_l,
		const std::array<usize, dim_>& close_r,
		const Op_& op = {},
		const Neg_& neg = {},
		const Ide_& ide = {}
	) noexcept -> T_ {
		std::array<usize, dim_> stride{};
		stride.back() = 1;
		for(usize id = dim_ - 1; id-- > 0;) {
			stride[id] = stride[id + 1] * shape[id + 1];
		}

		T_ ans = ide();
		for(u64 ib = 0; ib < (u64(1) << dim_); ++ib) {
			usize idx = 0;
			for(usize id = 0; id < dim_; ++id) {
				idx +=
					(((ib >> id) & 1) ? close_r[id] : open_l[id]) * stride[id];
			}
			ans =
				op(ans,
					std::popcount(ib) % 2 == dim_ % 2 ? acc[idx]
													  : neg(acc[idx]));
		}

		return ans;
	}
} // namespace stew::acc_sum
// ===== END   "acc_sum.hpp" =====
// ===== BEGIN "binary_search.hpp"  (from make_default.hpp) =====

#include <concepts>
#include <utility>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: binary_search.hpp) ---

namespace stew::binary_search {
	template <
		class T_,
		class MiddleF_ = decltype([]<class Int_>
				requires std::integral<Int_>
			(const Int_& l, const Int_& r) { return (l + r) / 2; })
		>
		inline auto
		binary_search(
			const T_& left,
			const T_& right,
			const auto& pred,
			const bool left_is,
			const MiddleF_& middle = {}
		) -> std::pair<T_, T_>
			requires requires(const T_ v) {
				{ pred(v) } -> std::convertible_to<bool>;
				{ middle(left, right) } -> std::convertible_to<T_>;
			}
	{
		T_ l = left;
		T_ r = right;
		while(r - l > 1) {
			const T_ mid = middle(l, r);
			if(pred(mid) ^ left_is) {
				r = mid;
			} else {
				l = mid;
			}
		}
		return {l, r};
	}
} // namespace stew::binary_search
// ===== END   "binary_search.hpp" =====
// ===== BEGIN "graph.hpp"  (from make_default.hpp) =====

#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: graph.hpp) ---

// ===== BEGIN "sparse_matrix.hpp"  (from graph.hpp) =====

#include <algorithm>
#include <ranges>
#include <utility>
#include <vector>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: sparse_matrix.hpp) ---

namespace stew::sparse_matrix {

	template <class E_ = void>
	struct Csr final {
		std::vector<E_> data;
		std::vector<usize> acc_row;
		std::vector<usize> col;

		auto row(const usize index) {
			auto f = [this](const usize i) -> std::pair<usize, E_&> {
				return {this->col[i], this->data[i]};
			};

			if(index >= this->acc_row.size() - 1) {
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

			if(index >= this->acc_row.size() - 1) {
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
			return std::views::iota(0u, this->acc_row.size() - 1) |
				std::views::transform([this](const usize i) {
					return this->row(i);
				});
		}

		auto c_view() const {
			return std::views::iota(0u, this->acc_row.size() - 1) |
				std::views::transform([this](const usize i) {
					return this->c_row(i);
				});
		}

		auto begin() {
			return this->view().begin();
		}

		auto end() {
			return this->view().end();
		}

		auto cbegin() const {
			return this->c_view().begin();
		}

		auto cend() const {
			return this->c_view().end();
		}
	};

	template <>
	struct Csr<> final {
		std::vector<usize> acc_row;
		std::vector<usize> col;

		auto row(const usize index) {
			auto f = [this](const usize i) -> usize { return this->col[i]; };

			if(index >= this->acc_row.size() - 1) {
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

			if(index >= this->acc_row.size() - 1) {
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
			return std::views::iota(0u, this->acc_row.size() - 1) |
				std::views::transform([this](const usize i) {
					return this->row(i);
				});
		}

		auto c_view() const {
			return std::views::iota(0u, this->acc_row.size() - 1) |
				std::views::transform([this](const usize i) {
					return this->c_row(i);
				});
		}

		auto begin() {
			return this->view().begin();
		}

		auto end() {
			return this->view().end();
		}

		auto cbegin() const {
			return this->c_view().begin();
		}

		auto cend() const {
			return this->c_view().end();
		}
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
			std::ranges::sort(this->buffer);
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

			for(const auto& [index, elem] : this->buffer) {
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
// ===== END   "sparse_matrix.hpp" =====

namespace stew::graph {
	template<class T_>
	concept weighted_adjant = requires(const T_ imut, const usize v) {
		{[](auto&& r) {
			for(const auto& [next, w] : r) {

			}
		}(imut(v))};
	};

	template<class T_>
	concept adjant = requires(const T_ imut, const usize v) {
		{[](auto&& r) {
			for(const auto& next : r) {

			}
		}(imut(v))};
	};

	template<class T_>
	concept mut_weighted_adjant = requires(const T_ imut, const usize v) {
		{[](auto&& r) {
			for(auto& [next, w] : r) {

			}
		}(imut(v))};
	};

	template<class T_>
	concept mut_adjant = requires(const T_ imut, const usize v) {
		{[](auto&& r) {
			for(auto& next : r) {

			}
		}(imut(v))};
	};

	template <class Vertex_, class Cost_ = Vertex_>
	inline constexpr auto dijkstra(
		const Vertex_ start, const Vertex_ goal, auto&& adj
	) -> std::unordered_map<Vertex_, std::pair<Cost_, Vertex_>>
	requires weighted_adjant<std::remove_cvref_t<decltype(adj)>>
	{
		std::unordered_map<Vertex_, std::pair<Cost_, Vertex_>> dist{};
		std::priority_queue<
			std::pair<Cost_, Vertex_>,
			std::vector<std::pair<Cost_, Vertex_>>,
			std::greater<std::pair<Cost_, Vertex_>>
		>
			que{};
		dist[start] = {Cost_(0), Vertex_(-1)};
		que.push({Cost_(0), start});

		while(!que.empty()) {
			const auto [cost, v] = que.top();
			que.pop();

			if(dist[v].first < cost)
				continue;

			for(const auto& [next, w] : adj(v)) {
				const auto new_cost = cost + w;
				if(dist.find(next) == dist.end() ||
					new_cost < dist[next].first) {
					dist[next] = std::pair{new_cost, v};
					que.push({new_cost, next});
				}
			}

			if(v == goal) {
				break;
			}
		}

		return dist;
	}

	// tarjan法。SCCを逆トポロジカルソートで返す
	template<class E_ = void>
	inline constexpr auto tarjan(const usize n, auto&& adj) -> sparse_matrix::Csr<E_>
	requires adjant<std::remove_cvref_t<decltype(adj)>> {
		auto coo = sparse_matrix::Coo<>::make(n);
		std::vector<usize> stack{}; stack.reserve(n);
		std::vector<usize> low(n);
		std::vector<usize> ord(n, u64(-1));
		std::vector<char> on_stack(n, false);
		usize now_ord = 0;
		usize group_count = 0;
		
		auto dfs = [&](this auto&& self, const usize v) -> void {
			low[v] = ord[v] = now_ord;
			now_ord++;
			stack.emplace_back(v);
			on_stack[v] = true;

			for(const auto& next : adj(v)) {
				if(ord[next] == usize(-1)) {
					self(next);
					low[v] = std::min(low[v], low[next]);
				}
				else if(on_stack[next]) {  // on_stackでない場合、既に訪問しきっている -> その頂点からこちらへ戻れない
					low[v] = std::min(low[v], ord[next]);
				}
			}

			if(low[v] == ord[v]) {  // そのSCCのルートならば
				while(true) {
					const usize w = stack.back(); stack.pop_back();
					on_stack[w] = false;
					coo.add({group_count, w});
					if(w == v) break;
				}
				group_count++;
			}
		};

		for(usize i = 0; i < n; ++i) {
			if(ord[i] == usize(-1)) dfs(i);
		}

		coo.is_sorted = true;

		return std::move(coo).to_csr(group_count);
	}
} // namespace stew::graph
// ===== END   "graph.hpp" =====
// ===== BEGIN "input_util.hpp"  (from make_default.hpp) =====

#include <concepts>
#include <iostream>
namespace stew::input_util {
	struct InputMarker {};

	template <class T_> struct Z final : InputMarker {
		using T = T_;
		static auto after_input(T_&& v) noexcept -> T_ { return v - 1; }
	};

	template <class T_> inline auto input() noexcept {
		if constexpr(std::derived_from<T_, InputMarker>) {
			typename T_::T v;
			std::cin >> v;
			return T_::after_input(std::move(v));
		} else {
			T_ v;
			std::cin >> v;
			return v;
		}
	}

	template <class... Args_> inline auto inputs() noexcept {
		return std::tuple{input<Args_>()...};
	}
} // namespace stew::input_util
// ===== END   "input_util.hpp" =====
// ===== BEGIN "integer_math.hpp"  (from make_default.hpp) =====

#include <array>
#include <concepts>
#include <numeric>
#include <tuple>
#include <vector>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: integer_math.hpp) ---

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
// ===== END   "integer_math.hpp" =====
// ===== BEGIN "rational.hpp"  (from make_default.hpp) =====

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
// ===== END   "rational.hpp" =====
// --- skip "sparse_matrix.hpp" (既に展開済み / 参照元: make_default.hpp) ---
// ===== BEGIN "union_find.hpp"  (from make_default.hpp) =====

#include <cstddef>

#include <optional>
#include <utility>
#include <vector>

// --- skip "integer_type.hpp" (既に展開済み / 参照元: union_find.hpp) ---

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
// ===== END   "union_find.hpp" =====

namespace stew {
	using namespace integer_type;
	using namespace utility;
	
	using namespace acc_sum;
	using namespace binary_search;
	using namespace graph;
	using namespace input_util;
	using namespace integer_math;
	using namespace rational;
	using namespace sparse_matrix;
	using namespace union_find;
}
// ===== END   "make_default.hpp" =====
