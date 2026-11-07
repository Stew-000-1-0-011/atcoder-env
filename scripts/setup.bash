#!/usr/bin/env bash
# git clone 後に一度実行する: bash scripts/setup.bash
#   - ACL を取得して直下に atcoder/ を置く
#   - templates/ のひな形を work/ にコピーする
# 既にあるものは上書きしないので、何度実行してもよい。
set -eu

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [ ! -d "$root/atcoder" ]; then
	tmp="$(mktemp -d)"
	git clone --depth 1 https://github.com/atcoder/ac-library.git "$tmp/ac-library"
	mv "$tmp/ac-library/atcoder" "$root/atcoder"
	rm -rf "$tmp"
	echo "作成: atcoder/"
fi

mkdir -p "$root/work"
for src in "$root"/templates/*; do
	name="$(basename "$src")"
	if [ ! -e "$root/work/$name" ]; then
		cp "$src" "$root/work/$name"
		echo "作成: work/$name"
	fi
done
