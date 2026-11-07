#!/usr/bin/env python3
"""
C++ファイルの #include "foo.hpp"(ダブルクオート)を再帰的に展開する。

ルール:
  - #include "..."  -> 中身を展開する(ダブルクオートのみ対象)
  - #include <...>   -> 展開せずそのまま残す
  - #pragma once     -> 削除する(重複展開は展開器側で防止するため不要)
  - 同じファイルは一度だけ展開する(循環include・重複展開を防止)
  - "..." の include が見つからない場合はエラーで停止する
  - 展開箇所には「どのファイルから展開されたか」がわかるマーカーを付ける
  - // stew:test-begin の行から // stew:test-end の行までは取り除く(テスト用のコード)。
    --keep-tests を付けると残す

使い方:
  python3 expand_includes.py main.cpp
  python3 expand_includes.py main.cpp -I include -I third_party/foo
  python3 expand_includes.py main.cpp -o expanded.cpp
"""

import argparse
import re
import sys
from pathlib import Path

# 行頭の空白を許容しつつ #include "..." を拾う(<...> はここでは拾わない)
QUOTE_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"\s*$')

# #pragma once を拾う(行頭の空白・トークン間の空白を許容)
PRAGMA_ONCE_RE = re.compile(r'^\s*#\s*pragma\s+once\s*$')

# テスト用のコードの範囲を示すコメント
TEST_BEGIN_RE = re.compile(r'^\s*//\s*stew:test-begin\b')
TEST_END_RE = re.compile(r'^\s*//\s*stew:test-end\b')


class TestBlockError(Exception):
	"""stew:test-begin / stew:test-end の対応が取れていないことを表す。"""


class IncludeNotFoundError(Exception):
	"""ダブルクオートの include が解決できなかったことを表す。"""

	def __init__(self, inc_name: str, referrer: Path, include_dirs: list[Path]):
		self.inc_name = inc_name
		self.referrer = referrer
		self.include_dirs = include_dirs
		searched = [str(referrer.parent)] + [str(d) for d in include_dirs]
		super().__init__(
			f'include が見つかりません: "{inc_name}"\n'
			f'  参照元: {referrer}\n'
			f'  探索したディレクトリ:\n'
			+ "\n".join(f'    - {s}' for s in searched)
		)


def resolve_include(inc_name: str, current_dir: Path, include_dirs: list[Path]) -> Path | None:
	"""include対象のファイルパスを解決する。見つからなければ None。"""
	# まず #include を書いたファイルと同じディレクトリを探す(C++の慣習)
	candidate = (current_dir / inc_name)
	if candidate.is_file():
		return candidate.resolve()
	# 次に -I で指定された検索パスを順に探す
	for d in include_dirs:
		candidate = (d / inc_name)
		if candidate.is_file():
			return candidate.resolve()
	return None


def expand_file(
	path: Path,
	include_dirs: list[Path],
	expanded: set[Path],
	display_name: str,
	from_desc: str,
	keep_tests: bool = False,
) -> list[str]:
	"""
	path のファイルを読み、include を再帰展開した行のリストを返す。

	expanded: すでに展開済みのファイル集合(一度のみ展開を保証)
	display_name: マーカーに表示する include 名(元の "foo.hpp" 表記)
	from_desc: どこから展開されたかの説明文
	"""
	resolved = path.resolve()
	lines: list[str] = []

	# 展開開始マーカー
	lines.append(f'// ===== BEGIN "{display_name}"  (from {from_desc}) =====')

	try:
		content = resolved.read_text(encoding="utf-8")
	except OSError as e:
		lines.append(f'// !! 読み込み失敗: {resolved} ({e})')
		lines.append(f'// ===== END   "{display_name}" =====')
		return lines

	expanded.add(resolved)

	in_test = False
	for lineno, raw_line in enumerate(content.splitlines(), start=1):
		# テスト用のコードは取り除く
		if not keep_tests:
			if TEST_BEGIN_RE.match(raw_line):
				if in_test:
					raise TestBlockError(f'{resolved}:{lineno}: stew:test-begin が入れ子になっています')
				in_test = True
				continue
			if TEST_END_RE.match(raw_line):
				if not in_test:
					raise TestBlockError(f'{resolved}:{lineno}: 対応する stew:test-begin がありません')
				in_test = False
				continue
			if in_test:
				continue

		# #pragma once は削除(そのまま次の行へ)
		if PRAGMA_ONCE_RE.match(raw_line):
			continue

		m = QUOTE_INCLUDE_RE.match(raw_line)
		if not m:
			# 通常行(<...> の include もここでそのまま残る)
			lines.append(raw_line)
			continue

		inc_name = m.group(1)
		target = resolve_include(inc_name, resolved.parent, include_dirs)

		if target is None:
			# 見つからない場合はエラーで停止する
			raise IncludeNotFoundError(inc_name, resolved, include_dirs)
		elif target in expanded:
			# 一度のみ展開: すでに展開済みなら中身は入れず注記だけ
			lines.append(f'// --- skip "{inc_name}" '
						 f'(既に展開済み / 参照元: {resolved.name}) ---')
		else:
			# 再帰的に展開
			lines.extend(
				expand_file(
					target,
					include_dirs,
					expanded,
					display_name=inc_name,
					from_desc=resolved.name,
					keep_tests=keep_tests,
				)
			)

	if in_test:
		raise TestBlockError(f'{resolved}: stew:test-begin に対応する stew:test-end がありません')

	# 展開終了マーカー
	lines.append(f'// ===== END   "{display_name}" =====')
	return lines


def main() -> int:
	parser = argparse.ArgumentParser(
		description='C++の #include "..." を再帰的に一度だけ展開する。'
	)
	parser.add_argument("source", help="入力するC++ファイル")
	parser.add_argument(
		"-I", "--include-dir", action="append", default=[],
		metavar="DIR", help="include検索パス(複数指定可)",
	)
	parser.add_argument(
		"-o", "--output", metavar="FILE",
		help="出力ファイル(省略時は標準出力)",
	)
	parser.add_argument(
		"--keep-tests", action="store_true",
		help="stew:test-begin から stew:test-end までのテスト用コードを残す",
	)
	args = parser.parse_args()

	source = Path(args.source)
	if not source.is_file():
		print(f"エラー: ファイルが存在しません: {source}", file=sys.stderr)
		return 1

	include_dirs = [Path(d) for d in args.include_dir]
	expanded: set[Path] = set()

	try:
		lines = expand_file(
			source,
			include_dirs,
			expanded,
			display_name=source.name,
			from_desc="ルート(入力ファイル)",
			keep_tests=args.keep_tests,
		)
	except (IncludeNotFoundError, TestBlockError) as e:
		print(f"エラー: {e}", file=sys.stderr)
		return 1

	result = "\n".join(lines) + "\n"

	# 出力は常に UTF-8 (改行は LF) で書く。
	# 標準出力をそのまま使うと、Windows ではコンソールの文字コード (cp932 など) で書かれて文字化けする。
	data = result.encode("utf-8")
	if args.output:
		Path(args.output).write_bytes(data)
		print(f"書き出しました: {args.output}", file=sys.stderr)
	else:
		sys.stdout.flush()
		sys.stdout.buffer.write(data)
		sys.stdout.buffer.flush()

	return 0


if __name__ == "__main__":
	raise SystemExit(main())