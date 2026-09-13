#!/usr/bin/env python3
"""bar.exe の出力を foo.exe / foo2.exe に渡し、
さらにそれらを baz.exe に通した結果が食い違うケースを探す。

パイプライン:
    bar.exe                      -> bar_out
    bar_out -> foo.exe           -> foo_out
    bar_out -> foo2.exe          -> foo2_out
    foo_out  -> baz.exe          -> r1
    foo2_out -> baz.exe          -> r2
    r1 != r2 のときに (bar_out, foo_out, foo2_out) を返してループを抜ける
"""

import subprocess
import sys


def run(exe, stdin=None):
    """exe を起動し、stdin(bytes) を渡して stdout(bytes) を返す。"""
    proc = subprocess.run(
        [exe],
        input=stdin,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,  # 異常終了コードでも例外にしない（必要なら True に）
    )
    return proc.stdout


def find_divergence(iterations):
    """最大 iterations 回試行し、差分が出た組を返す。なければ None。"""
    for i in range(iterations):
        bar_out = run("bar.exe")

        foo_out = run("foo.exe", bar_out)
        foo2_out = run("foo2.exe", bar_out)

        r1 = run("baz.exe", foo_out)
        r2 = run("baz.exe", foo2_out)

        if r1 != r2:
            print(f"[!] 差分を検出しました（試行 {i + 1} 回目）", file=sys.stderr)
            return bar_out, foo_out, foo2_out

    print(f"[ ] {iterations} 回試行しましたが差分はありませんでした", file=sys.stderr)
    return None


def main():
    # 試行回数はコマンドライン引数で指定（省略時は 100 回）
    iterations = int(sys.argv[1]) if len(sys.argv) > 1 else 100

    result = find_divergence(iterations)
    if result is None:
        sys.exit(1)

    bar_out, foo_out, foo2_out = result
    # 必要に応じてファイルに保存しておくと再現に便利
    with open("divergence_bar.bin", "wb") as f:
        f.write(bar_out)
    with open("divergence_foo.bin", "wb") as f:
        f.write(foo_out)
    with open("divergence_foo2.bin", "wb") as f:
        f.write(foo2_out)

    print("bar.exe out :", bar_out)
    print("foo.exe out :", foo_out)
    print("foo2.exe out:", foo2_out)


if __name__ == "__main__":
    main()