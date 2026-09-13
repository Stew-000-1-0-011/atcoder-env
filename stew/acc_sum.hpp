#pragma once

#include <bit>
#include <vector>

#include "integer_type.hpp"

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