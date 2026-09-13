#pragma once

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>

namespace stew::shakutori {
	// 内部可変性で外側にはimutableなものに`mutable`を使うのがちょっと不安なので、
	// 本来constであるべきものをconstにせず制約づけてる
	// 内部可変性のメンタルモデルがあんまりない...

	// noexceptも同様に無視してる

	// ビバ償却定数！

	template <class T_>
	concept slide_window = requires(T_& mut) {
		typename T_::Element;
		typename T_::Ret;
		requires requires(const T_::Element& e) {
			{ mut.extend_r(e) };
			{ mut.shrink_l(e) };
			{ mut.value() } -> std::convertible_to<typename T_::Ret>;
		};
	};

	template <class T_>
	concept tryable_slide_window =
		slide_window<T_> && requires(const T_::Element& e, T_& mut) {
			{ mut.shrink_r(e) };
		};

	// ほんとはstd::generatorで書くべきかも
	template <class Ret_, bool is_true_continue, class Idx_>
	inline constexpr auto shak_window(
		auto& prop,
		auto& range_to_val,
		auto& arr,
		const std::pair<Idx_, Idx_>& haop
	) noexcept -> std::vector<Ret_>
		requires(tryable_slide_window<std::remove_cvref_t<decltype(prop)>> &&
					std::convertible_to<
						typename std::remove_cvref_t<decltype(prop)>::Ret,
						bool
					>) &&
		(slide_window<std::remove_cvref_t<decltype(range_to_val)>> &&
			std::convertible_to<
				typename std::remove_cvref_t<decltype(range_to_val)>::Ret,
				Ret_
			>) &&
		(std::same_as<
			 std::remove_cvref_t<decltype(arr(std::declval<Idx_>()))>,
			 typename std::remove_cvref_t<decltype(prop)>::Element
		 > &&
			std::same_as<
				typename std::remove_cvref_t<decltype(prop)>::Element,
				typename std::remove_cvref_t<decltype(range_to_val)>::Element
			>)
	{
		const auto& [bi, en] = haop;
		std::vector<Ret_> ret{};
		Idx_ r = bi;

		for(Idx_ l = bi; l < en; ++l) {
			r = std::max<Idx_>(l, r);

			while(r < en) {
				prop.extend_r(arr(r));

				const bool prop_val = prop.value();
				if(prop_val == is_true_continue) {
					range_to_val.extend_r(arr(r));
					++r;
				} else {
					prop.shrink_r(arr(r));
					break;
				}
			}

			ret.emplace_back(range_to_val.value());
			if(l != r) {
				prop.shrink_l(arr(l));
				range_to_val.shrink_l(arr(l));
			}
		}

		return ret;
	}

	// std::generatorで書きたいかも
	template <bool is_true_continue, class Idx_>
	inline constexpr auto shak(
		auto& prop, const std::pair<Idx_, Idx_>& haop
	) noexcept -> std::vector<std::pair<Idx_, Idx_>>
		requires requires(const Idx_& l, const Idx_& r) {
			{ prop(l, r) } -> std::convertible_to<bool>;
		}
	{
		const auto& [bi, en] = haop;
		std::vector<std::pair<Idx_, Idx_>> ret{};
		Idx_ r = bi;

		for(Idx_ l = bi; l < en; ++l) {
			r = std::max<Idx_>(l, r);

			while(r < en) {
				const bool prop_val = prop(l, r + 1);
				if(prop_val == is_true_continue) {
					++r;
				} else {
					break;
				}
			}

			ret.emplace_back(l, r);
		}

		return ret;
	}
} // namespace stew::shakutori