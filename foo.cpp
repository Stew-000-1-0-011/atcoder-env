#include <atcoder/segtree.hpp>

#include "default2.hpp"
#include <algorithm>
#include <compare>
#include <limits>
#include <optional>

using namespace stew::integer_type;
using stew::input;
using stew::inputs;
using stew::Z;
using stew::utility::debug;

struct S {
	std::optional<std::string> str{};
	i64 correct{std::numeric_limits<i64>::max()};
	i64 from_qid{-1};
	i64 num{};

	friend auto operator*(const S& l, const S& r) noexcept -> S {
		return S {
			std::nullopt
			, l.num + r.num
		};
	}

	static auto zero() noexcept -> S {
		return S{};
	}

	static auto disable(const S& x) -> S {
		return S {
			{}
			, x.correct
			, x.from_qid
			, 0
		};
	}

	static auto enable(const S& x) -> S {
		return S {
			{}
			, x.correct
			, x.from_qid
			, 1
		};
	}

	friend auto operator<=>(const S&, const S&) noexcept -> std::strong_ordering = default;
};

int main() {
	const auto [n, m, k] = inputs<i64, i64, i64>();
	const auto t = input<std::string>();
	std::vector<std::string> s(n);
	for(auto& si : s) si = input<std::string>();
	const auto q = input<i64>();
	std::vector<std::pair<i64, i64>> ij(q);
	for(auto& [i, j] : ij) {
		std::tie(i, j) = inputs<Z<i64>, Z<i64>>();
	}

	std::vector<S> base(n + q);
	std::vector<i64> before(n);
	for(i64 i = 0; i < n; ++i) {
		std::string str(k, 'X');
		i64 correct = 0;
		for(i64 ik = 0; ik < k; ++ik) {
			str[ik] = (t[ik] == s[i][ik] ? 'O' : 'X');
			if(t[ik] == s[i][ik]) correct++;
		}
		base[i] = S {
			std::move(str)
			, correct
			, i
			, 1
		};
		before[i] = i;
	}
	std::vector<std::pair<i64, i64>> change(q);
	for(i64 iq = 0; iq < q; ++iq) {
		const auto [i, j] = ij[iq];
		const S& last = base[before[i]];
		std::string str = *(last.str);
		str[j] = (str[j] == 'O' ? 'X' : 'O');
		const i64 correct = last.correct + (str[j] == 'O' ? -1 : 1);
		base[n + iq] = S {
			std::move(str)
			, correct
			, n + iq
			, 0
		};
		change[iq] = {before[i], n + iq};
		before[i] = n + iq;
	}

	std::ranges::sort(base);
	std::vector<i64> sorted_idxs(n + q);
	for(i64 i = 0; i < n + q; ++i) {
		sorted_idxs[base[i].from_qid] = i;
	}

	using Segt = atcoder::segtree <
		S
		, [](const S& l, const S& r) noexcept -> S {
			return l * r;
		}
		, []() -> S {
			return S::zero();
		}
	>;

	Segt segt(std::move(base));
	
	for(i64 iq = 0; iq < q; ++iq) {
		const auto [bef, aft] = change[iq];
		segt.set(sorted_idxs[bef], S::disable(segt.get(sorted_idxs[bef])));
		segt.set(sorted_idxs[aft], S::enable(segt.get(sorted_idxs[aft])));
		const auto r = segt.max_right(0, [&](const S& x) -> bool {
			return x.num <= m;
		});
		const i64 datsuraku_correct = r == n + q ? 0 : segt.get(r).correct;
		const auto true_r = segt.max_right(0, [&](const S& x) -> bool {
			return x.correct > datsuraku_correct;
		});
		
		if(sorted_idxs[n + iq] < true_r) {
			std::println("Yes");
		}
		else {
			std::println("No");
		}
	}
}
