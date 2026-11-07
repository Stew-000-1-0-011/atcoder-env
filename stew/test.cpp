// stew 以下の各ヘッダの test() を実行する。
//   test.exe          : すべて実行
//   test.exe <名前>   : 指定したものだけ実行
//   test.exe --list   : 名前の一覧を表示
#include <cstring>
#include <print>

#include "make_default.hpp"
#include "monoid.hpp"
#include "shakutori.hpp"
#include "swag.hpp"

struct Entry {
	const char* name;
	void (*run)();
};

constexpr Entry entries[] = {
	{"integer_type", stew::integer_type::test},
	{"utility", stew::utility::test},
	{"acc_sum", stew::acc_sum::test},
	{"binary_search", stew::binary_search::test},
	{"graph", stew::graph::test},
	{"input_util", stew::input_util::test},
	{"integer_math", stew::integer_math::test},
	{"monoid", stew::monoid::test},
	{"rational", stew::rational::test},
	{"shakutori", stew::shakutori::test},
	{"sparse_matrix", stew::sparse_matrix::test},
	{"swag", stew::swag::test},
	{"union_find", stew::union_find::test},
};

int main(int argc, char** argv) {
	if(argc >= 2 && std::strcmp(argv[1], "--list") == 0) {
		for(const auto& e : entries) std::println("{}", e.name);
		return 0;
	}

	bool found = false;
	for(const auto& e : entries) {
		if(argc >= 2 && std::strcmp(argv[1], e.name) != 0) continue;
		found = true;
		const auto before = stew::utility::test_failures();
		e.run();
		const auto failed = stew::utility::test_failures() - before;
		std::println("{} {} ({} 件失敗)", failed == 0 ? "OK  " : "NG  ", e.name, failed);
	}
	if(!found) {
		std::println(stderr, "テストが見つかりません: {}", argv[1]);
		return 2;
	}
	return stew::utility::test_failures() == 0 ? 0 : 1;
}
