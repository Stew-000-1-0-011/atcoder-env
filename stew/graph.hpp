#pragma once

#include <deque>
#include <iterator>
#include <limits>
#include <queue>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "integer_type.hpp"

#include "sparse_matrix.hpp"
#include <algorithm>

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

	// 頂点 0 .. n-1 のグラフで、start からの最短距離を求める。
	// 戻り値の [v] は (距離, 最短経路で v の直前の頂点)。
	//   - start は (0, usize(-1))
	//   - 届かない頂点は (Cost_ の最大値, usize(-1))
	// goal を取り出した時点で打ち切るので、goal 以外の頂点の距離は確定していないことがある。
	// すべての頂点の距離が必要なら、goal に n 以上の値 (usize(-1) など) を渡す。
	template <class Cost_ = i64>
	inline auto dijkstra(
		const usize n, const usize start, const usize goal, auto&& adj
	) -> std::vector<std::pair<Cost_, usize>>
	requires weighted_adjant<std::remove_cvref_t<decltype(adj)>>
	{
		constexpr Cost_ inf = std::numeric_limits<Cost_>::max();
		std::vector<std::pair<Cost_, usize>> dist(n, {inf, usize(-1)});
		std::priority_queue<
			std::pair<Cost_, usize>,
			std::vector<std::pair<Cost_, usize>>,
			std::greater<std::pair<Cost_, usize>>
		>
			que{};
		dist[start] = {Cost_(0), usize(-1)};
		que.push({Cost_(0), start});

		while(!que.empty()) {
			const auto [cost, v] = que.top();
			que.pop();

			if(dist[v].first < cost)
				continue;

			for(const auto& [next, w] : adj(v)) {
				const Cost_ new_cost = cost + w;
				if(new_cost < dist[next].first) {
					dist[next] = {new_cost, v};
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

		// 再帰の代わりに、DFS の途中の状態 (頂点と、隣接頂点をどこまで見たか) を積む。
		// adj(v) が view を返す場合、イテレータが view 自身を指すことがあるので、
		// 要素の場所が動かない deque に置き、view とイテレータを同じフレームに持つ。
		using Adj = std::remove_reference_t<decltype(adj)>;
		using Range = decltype(adj(usize{}));
		struct Frame {
			usize v;
			Range range;
			std::ranges::iterator_t<std::remove_reference_t<Range>> it;
			std::ranges::sentinel_t<std::remove_reference_t<Range>> end;

			Frame(const usize v_, Adj& adj_)
				: v(v_), range(adj_(v_)), it(std::ranges::begin(range)),
				  end(std::ranges::end(range)) {}
		};
		std::deque<Frame> frames{};

		const auto visit = [&](const usize v) {
			low[v] = ord[v] = now_ord;
			now_ord++;
			stack.emplace_back(v);
			on_stack[v] = true;
			frames.emplace_back(v, adj);
		};

		for(usize root = 0; root < n; ++root) {
			if(ord[root] != usize(-1)) continue;
			visit(root);

			while(!frames.empty()) {
				Frame& f = frames.back();
				const usize v = f.v;

				if(f.it != f.end) {
					const usize next = *f.it;
					++f.it;
					if(ord[next] == usize(-1)) {
						visit(next);  // 戻ってきたときに low[v] を更新する (下の else)
					}
					else if(on_stack[next]) {  // on_stackでない場合、既に訪問しきっている -> その頂点からこちらへ戻れない
						low[v] = std::min(low[v], ord[next]);
					}
					continue;
				}

				// v の隣接頂点を見終えた
				if(low[v] == ord[v]) {  // そのSCCのルートならば
					while(true) {
						const usize w = stack.back(); stack.pop_back();
						on_stack[w] = false;
						coo.add({group_count, w});
						if(w == v) break;
					}
					group_count++;
				}
				frames.pop_back();
				if(!frames.empty()) {
					const usize parent = frames.back().v;
					low[parent] = std::min(low[parent], low[v]);
				}
			}
		}

		coo.is_sorted = true;

		return std::move(coo).to_csr(group_count);
	}
} // namespace stew::graph

// stew:test-begin (expand.py はここから stew:test-end までを取り除く)
#include <random>
#include "utility.hpp"

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
			const auto dist = dijkstra(
				4, 0, 3, [&](const usize v) -> const auto& { return g[v]; }
			);
			using P = std::pair<i64, usize>;
			test_check(dist.size() == 4);
			test_check(dist[0] == P{0, usize(-1)});
			test_check(dist[2] == P{1, 0});
			test_check(dist[1] == P{3, 2});
			test_check(dist[3] == P{4, 1});
		}

		// dijkstra: 届かない頂点と、Cost_ の指定
		{
			const std::vector<std::vector<std::pair<usize, i32>>> g{{{1, 5}}, {}, {{0, 1}}};
			const auto dist = dijkstra<i32>(
				3, 0, usize(-1), [&](const usize v) -> const auto& { return g[v]; }
			);
			test_check(dist[1] == std::pair<i32, usize>{5, 0});
			test_check(dist[2] == std::pair<i32, usize>{std::numeric_limits<i32>::max(), usize(-1)});
		}

		// dijkstra: ランダムなグラフで、ベルマンフォードと比べる (全頂点を求める)
		{
			std::mt19937 rng{11};
			for(int iter = 0; iter < 200; ++iter) {
				const usize n = 1 + rng() % 10;
				std::vector<std::vector<std::pair<usize, i64>>> g(n);
				std::vector<std::tuple<usize, usize, i64>> edges{};
				const usize m = rng() % (n * 3 + 1);
				for(usize e = 0; e < m; ++e) {
					const usize a = rng() % n;
					const usize b = rng() % n;
					const i64 w = rng() % 10;
					g[a].push_back({b, w});
					edges.push_back({a, b, w});
				}
				const usize start = rng() % n;

				constexpr i64 inf = std::numeric_limits<i64>::max();
				std::vector<i64> bf(n, inf);
				bf[start] = 0;
				for(usize k = 0; k < n; ++k) {
					for(const auto& [a, b, w] : edges) {
						if(bf[a] != inf && bf[a] + w < bf[b]) bf[b] = bf[a] + w;
					}
				}

				const auto dist = dijkstra(
					n, start, usize(-1), [&](const usize v) -> const auto& { return g[v]; }
				);
				for(usize v = 0; v < n; ++v) {
					test_check(dist[v].first == bf[v]);
					// 直前の頂点からの辺で距離が説明できる
					if(v != start && bf[v] != inf) {
						const usize p = dist[v].second;
						bool ok = false;
						for(const auto& [to, w] : g[p]) ok |= (to == v && dist[p].first + w == dist[v].first);
						test_check(ok);
					}
				}
			}
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

		// tarjan: 長い一本道 (再帰だとスタックが深くなる)
		{
			const usize n = 200'000;
			std::vector<std::vector<usize>> path(n);
			for(usize i = 0; i + 1 < n; ++i) path[i].push_back(i + 1);
			auto scc = tarjan(n, [&](const usize v) -> const auto& { return path[v]; });
			test_check(scc.rows() == n);
			// 逆トポロジカル順なので、最後の頂点が最初の成分になる
			test_check(scc.col.front() == n - 1 && scc.col.back() == 0);

			// 一周する閉路は1つの成分になる
			path[n - 1].push_back(0);
			auto cyc = tarjan(n, [&](const usize v) -> const auto& { return path[v]; });
			test_check(cyc.rows() == 1 && cyc.col.size() == n);
		}

		// tarjan: view を返す隣接 (Csr の行)
		{
			auto coo = sparse_matrix::Coo<>::make(5);
			coo.add({0, 1});
			coo.add({1, 0});
			coo.add({1, 2});
			auto g = std::move(coo).to_csr(3);
			auto scc = tarjan(3, [&](const usize v) { return g.row(v); });
			test_check(scc.rows() == 2);
		}

		// tarjan: ランダムなグラフで、推移閉包から求めた強連結成分と比べる
		{
			std::mt19937 rng{7};
			for(int iter = 0; iter < 200; ++iter) {
				const usize n = 1 + rng() % 12;
				std::vector<std::vector<usize>> g(n);
				std::vector<std::vector<char>> reach(n, std::vector<char>(n, 0));
				for(usize i = 0; i < n; ++i) reach[i][i] = 1;
				const usize m = rng() % (n * 3 + 1);
				for(usize e = 0; e < m; ++e) {
					const usize a = rng() % n;
					const usize b = rng() % n;
					g[a].push_back(b);
					reach[a][b] = 1;
				}
				for(usize k = 0; k < n; ++k)
					for(usize i = 0; i < n; ++i)
						for(usize j = 0; j < n; ++j)
							if(reach[i][k] && reach[k][j]) reach[i][j] = 1;

				auto scc = tarjan(n, [&](const usize v) -> const auto& { return g[v]; });
				std::vector<usize> group(n, usize(-1));
				for(usize c = 0; c < scc.rows(); ++c) {
					for(const auto v : scc.row(c)) group[v] = c;
				}
				for(usize i = 0; i < n; ++i) {
					test_check(group[i] != usize(-1));
					for(usize j = 0; j < n; ++j) {
						const bool same = reach[i][j] && reach[j][i];
						test_check(same == (group[i] == group[j]));
						// 逆トポロジカル順: i から j へ行けるなら、j の成分が先 (番号が小さいか同じ)
						if(reach[i][j]) test_check(group[j] <= group[i]);
					}
				}
			}
		}

		// tarjan: 辺がない
		{
			const std::vector<std::vector<usize>> g(3);
			auto scc = tarjan(3, [&](const usize v) -> const auto& { return g[v]; });
			test_check(scc.acc_row.size() == 4);
		}
	}
} // namespace stew::graph
// stew:test-end
