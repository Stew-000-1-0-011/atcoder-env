#!/usr/bin/env bash
# 使い方: bash scripts/build_cpp.bash [名前...]   (省略時は foo)
#   work/<名前>.cpp をビルドして build/<名前>.exe を作る。
#   ビルド前に stew/make_default.hpp を展開して build/default.hpp を生成する。
set -eu

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/build"

# AtCoder の生成AI対策ルールにより、事前に生成AIで作成したコードを使うときは公開先の URL を提出に含める必要があるので、
# 展開したヘッダの先頭に書いておく (https://info.atcoder.jp/entry/llm-rules-ja)
{
	echo "// このライブラリは生成AIで作成したコードを含み、コンテスト開始前に以下で公開している"
	echo "// https://github.com/Stew-000-1-0-011/atcoder-env"
	python3 "$root/stew/expand.py" "$root/stew/make_default.hpp"
} > "$root/build/default.hpp"

if [ $# -eq 0 ]; then
	set -- foo
fi

for name in "$@"; do
	clang++ -std=c++26 -g -O0 -Wpedantic -Wall -Wextra -fsanitize=address \
		-I"$root" -I"$root/build" \
		"$root/work/$name.cpp" -o "$root/build/$name.exe"
done
