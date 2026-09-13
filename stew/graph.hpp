#pragma once

#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "integer_type.hpp"

#include "sparse_matrix.hpp"

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
