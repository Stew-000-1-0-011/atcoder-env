#pragma once

#include <queue>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "integer_type.hpp"

#include "sparse_matrix.hpp"
#include <algorithm>
#include "utility.hpp"

namespace stew::graph {
	namespace impl {
		// 構造化束縛で2つに分けられる型 (pair や 要素2つの tuple)
		template <class T_>
		concept pair_like = requires {
			requires std::tuple_size<std::remove_cvref_t<T_>>::value == 2;
		};

		// adj(v) が返す range の要素 (参照) の型
		template <class T_>
		using adj_reference_t = std::ranges::range_reference_t<
			decltype(std::declval<const T_&>()(std::declval<const usize&>()))
		>;
	} // namespace impl

	// adj(v) が v の隣接頂点を並べた range を返す
	template <class T_>
	concept adjant = requires(const T_ imut, const usize v) {
		requires std::ranges::input_range<decltype(imut(v))>;
	};

	// adj(v) の要素が (隣接頂点, 重み) に分けられる
	template <class T_>
	concept weighted_adjant =
		adjant<T_> && impl::pair_like<impl::adj_reference_t<T_>>;

	// adj(v) の要素が書き換えられる (const でない左辺値参照で返る)
	template <class T_>
	concept mut_adjant = adjant<T_> &&
		std::is_lvalue_reference_v<impl::adj_reference_t<T_>> &&
		!std::is_const_v<std::remove_reference_t<impl::adj_reference_t<T_>>>;

	template <class T_>
	concept mut_weighted_adjant = weighted_adjant<T_> && mut_adjant<T_>;

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
	inline constexpr auto tarjan(const usize n, auto&& adj) -> sparse_matrix::Csr<>
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

namespace stew::graph {
	inline void test() {
		using utility::test_check;

		// concept
		{
			const std::vector<std::vector<usize>> g1{{0}};
			std::vector<std::vector<usize>> g2{{0}};
			const std::vector<std::vector<std::pair<usize, i64>>> g3{{{0, 1}}};
			std::vector<std::vector<std::pair<usize, i64>>> g4{{{0, 1}}};
			auto csr = sparse_matrix::Coo<i64>::make(0);
			auto csr2 = std::move(csr).to_csr(1);

			const auto a1 = [&](const usize v) -> const auto& { return g1[v]; };
			const auto a2 = [&](const usize v) -> auto& { return g2[v]; };
			const auto a3 = [&](const usize v) -> const auto& { return g3[v]; };
			const auto a4 = [&](const usize v) -> auto& { return g4[v]; };
			const auto a5 = [&](const usize v) { return csr2.row(v); };
			const auto a6 = [](const usize v) { return v; };

			static_assert(adjant<decltype(a1)> && !weighted_adjant<decltype(a1)>);
			static_assert(!mut_adjant<decltype(a1)> && mut_adjant<decltype(a2)>);
			static_assert(weighted_adjant<decltype(a3)> && !mut_weighted_adjant<decltype(a3)>);
			static_assert(mut_weighted_adjant<decltype(a4)>);
			static_assert(weighted_adjant<decltype(a5)>);
			static_assert(!adjant<decltype(a6)>);
		}

		// dijkstra: 0 -> 2 -> 1 -> 3 が最短
		{
			const std::vector<std::vector<std::pair<usize, i64>>> g{
				{{1, 4}, {2, 1}},
				{{3, 1}},
				{{1, 2}},
				{},
			};
			const auto dist = dijkstra<usize, i64>(
				0, 3, [&](const usize v) -> const auto& { return g[v]; }
			);
			test_check(dist.at(0).first == 0);
			test_check(dist.at(2) == std::pair<i64, usize>{1, 0});
			test_check(dist.at(1) == std::pair<i64, usize>{3, 2});
			test_check(dist.at(3) == std::pair<i64, usize>{4, 1});
		}

		// tarjan: {0, 1, 2} -> {3, 4}
		{
			const std::vector<std::vector<usize>> g{{1}, {2}, {0, 3}, {4}, {3}};
			auto scc = tarjan(5, [&](const usize v) -> const auto& { return g[v]; });
			const auto group = [&](const usize i) {
				std::vector<usize> v{};
				for(const auto x : scc.row(i)) v.push_back(x);
				std::ranges::sort(v);
				return v;
			};
			test_check(scc.acc_row.size() == 3);
			// 逆トポロジカル順なので、行き先側の成分が先に来る
			test_check(group(0) == std::vector<usize>{3, 4});
			test_check(group(1) == std::vector<usize>{0, 1, 2});
		}

		// tarjan: 辺がない
		{
			const std::vector<std::vector<usize>> g(3);
			auto scc = tarjan(3, [&](const usize v) -> const auto& { return g[v]; });
			test_check(scc.acc_row.size() == 4);
		}
	}
} // namespace stew::graph
