#!/usr/bin/env bash
# 使い方: bash scripts/test.bash [名前...]   (省略時はすべて)
#   stew/test.cpp をビルドし、各ヘッダの test() を1つずつ別プロセスで実行する。
#   (クラッシュしても他のテストの結果が見られるように分けている)
set -u

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$root/build"

clang++ -std=c++26 -g -O0 -Wpedantic -Wall -Wextra -fsanitize=address,undefined \
	-I"$root/stew" \
	"$root/stew/test.cpp" -o "$root/build/test.exe" || exit 1

if [ $# -eq 0 ]; then
	mapfile -t names < <("$root/build/test.exe" --list | tr -d '\r')
else
	names=("$@")
fi

ng=0
for name in "${names[@]}"; do
	ASAN_OPTIONS=exitcode=99 UBSAN_OPTIONS=halt_on_error=1:exitcode=99 "$root/build/test.exe" "$name"
	status=$?
	if [ $status -gt 1 ]; then
		echo "CRASH $name (終了コード $status)"
	fi
	if [ $status -ne 0 ]; then
		ng=$((ng + 1))
	fi
done

echo "失敗: $ng / ${#names[@]}"
[ $ng -eq 0 ]
