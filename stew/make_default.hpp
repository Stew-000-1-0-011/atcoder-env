#pragma once

#include "includes.hpp"

#include "integer_type.hpp"
#include "utility.hpp"

#include "acc_sum.hpp"
#include "binary_search.hpp"
#include "graph.hpp"
#include "input_util.hpp"
#include "integer_math.hpp"
#include "rational.hpp"
#include "sparse_matrix.hpp"
#include "union_find.hpp"

// 各ヘッダの名前空間をまとめて使うための名前空間。
// stew 自体には取り込まないので、using namespace stew; しても log2 や binary_search などが std の名前と並ばない。
// (stew に直接あるのは、整数型の別名と各ヘッダの名前空間の名前、utility::univ_ref だけ)
namespace stew::stew_compat {
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