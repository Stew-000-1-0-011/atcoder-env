#!/usr/bin/env bash
# 使い方: bash scripts/build_cpp.bash [名前...]   (省略時は foo)
#   work/<名前>.cpp をビルドして build/<名前>.exe を作る。
#   ビルド前に stew/make_default.hpp を展開して build/default.hpp を生成する。
#
#   #include "default.hpp" しているファイルは、プリコンパイル済みヘッダ (build/pch.hpp.pch) を使ってビルドする。
#   PCH は default.hpp と ACL をまとめたもので、ヘッダ・ビルドオプション・コンパイラが変わったときだけ作り直す。
#   NO_PCH=1 を付けると PCH を使わない。ACL を差し替えたときは build/pch.hpp.pch を消す。
set -eu

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build="$root/build"
mkdir -p "$build"

flags=(-std=c++26 -g -O0 -Wpedantic -Wall -Wextra -fsanitize=address -I"$root" -I"$build")

# $1 の内容を標準入力の内容にする。変わらないときは書き換えない (更新時刻を保ち、PCH を作り直さずに済ませる)
write_if_changed() {
	local tmp="$1.tmp"
	cat > "$tmp"
	if cmp -s "$tmp" "$1"; then
		rm -f "$tmp"
	else
		mv -f "$tmp" "$1"
	fi
}

# AtCoder の生成AI対策ルールにより、事前に生成AIで作成したコードを使うときは公開先の URL を提出に含める必要があるので、
# 展開したヘッダの先頭に書いておく (https://info.atcoder.jp/entry/llm-rules-ja)
# #pragma once は、PCH に入れたヘッダを解答の #include で読み直さないためのもの (提出用に展開すると expand.py が取り除く)
{
	echo "#pragma once"
	echo "// このライブラリは生成AIで作成したコードを含み、コンテスト開始前に以下で公開している"
	echo "// https://github.com/Stew-000-1-0-011/atcoder-env"
	python3 "$root/stew/expand.py" "$root/stew/make_default.hpp"
} | write_if_changed "$build/default.hpp"

pch="$build/pch.hpp.pch"
use_pch=1
if [ "${NO_PCH:-0}" = "1" ]; then
	use_pch=0
fi

if [ $use_pch -eq 1 ]; then
	{
		echo "#pragma once"
		if [ -d "$root/atcoder" ]; then
			echo "#include <atcoder/all>"
		fi
		echo '#include "default.hpp"'
	} | write_if_changed "$build/pch.hpp"

	# PCH はビルドオプションとコンパイラが同じときしか使えないので、それも記録しておく
	{
		clang++ --version | head -n 1
		printf '%s\n' "${flags[@]}"
	} | write_if_changed "$build/pch.flags"

	if [ ! -f "$pch" ] || [ "$build/default.hpp" -nt "$pch" ] ||
		[ "$build/pch.hpp" -nt "$pch" ] || [ "$build/pch.flags" -nt "$pch" ]; then
		echo "プリコンパイル済みヘッダを作ります" >&2
		rm -f "$pch"
		clang++ "${flags[@]}" -x c++-header "$build/pch.hpp" -o "$pch"
	fi
fi

if [ $# -eq 0 ]; then
	set -- foo
fi

for name in "$@"; do
	src="$root/work/$name.cpp"
	extra=()
	if [ $use_pch -eq 1 ] && grep -q '^[[:space:]]*#[[:space:]]*include[[:space:]]*"default.hpp"' "$src"; then
		extra=(-include-pch "$pch")
	fi
	clang++ "${flags[@]}" "${extra[@]}" "$src" -o "$build/$name.exe"
done
