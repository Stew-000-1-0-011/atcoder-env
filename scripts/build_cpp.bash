#!/usr/bin/env bash
# 使い方: bash scripts/build_cpp.bash [名前...]   (省略時は foo)
#   work/<名前>.cpp をビルドして build/<名前>.exe を作る。
#   ビルド前に stew/make_default.hpp を展開して build/default2.hpp を生成する。
set -eu

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/build"

python3 "$root/stew/expand.py" "$root/stew/make_default.hpp" -o "$root/build/default2.hpp"

if [ $# -eq 0 ]; then
	set -- foo
fi

for name in "$@"; do
	clang++ -std=c++23 -g -O0 -Wpedantic -Wall -Wextra -fsanitize=address \
		-I"$root" -I"$root/build" \
		"$root/work/$name.cpp" -o "$root/build/$name.exe"
done
