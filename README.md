atcoder用の環境。

clone したら最初に一度だけ実行する。ACL (`atcoder/`) の取得と、`templates/` のひな形から `work/` の作成を行う。
```
bash scripts/setup.bash
```

## 構成
- `work/` : 解答 (`foo.cpp`)、入力生成 (`bar.cpp`)、出力整形 (`baz.cpp`)、デバッグ用入力 (`input.txt`)。git 管理外
- `templates/` : `work/` のひな形。`work/` の編集はここに反映されない
- `stew/` : 自作ライブラリ。`expand.py` で `make_default.hpp` を1ファイルに展開できる。各ヘッダの名前空間に `test()` があり、`test.cpp` から呼ぶ。テストは `// stew:test-begin` から `// stew:test-end` の間に置き、`expand.py` はそこを取り除く (`--keep-tests` で残す)。各ヘッダの名前空間をまとめて使うときは `stew::stew_compat` を使う
- `scripts/` : ビルド (`build_cpp.bash`)、ライブラリのテスト (`test.bash`)、ランダムテスト (`find.py`)
- `build/` : 生成物 (`*.exe`, `default.hpp`)。git 管理外

## 使い方
```
bash scripts/build_cpp.bash            # foo をビルド
bash scripts/build_cpp.bash foo bar baz
python3 scripts/find.py 100
bash scripts/test.bash                 # stew のテストをすべて実行
bash scripts/test.bash swag rational   # 一部だけ実行
```

## 生成AIルールについて
`stew/` などには生成AIで作成したコードが含まれる。[AtCoder の生成AI対策ルール](https://info.atcoder.jp/entry/llm-rules-ja)により、ABC・ARC・AGC の開催中に使うには、コンテスト開始前にこのリポジトリへ push して公開しておき、提出に URL を含める必要がある。URL は `scripts/build_cpp.bash` が生成する `build/default.hpp` の先頭に入るので、`stew/expand.py` で展開した提出コードにもそのまま残る。
