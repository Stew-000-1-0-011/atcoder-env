atcoder用の環境。

clone したら最初に一度だけ実行する。ACL (`atcoder/`) の取得と、`templates/` のひな形から `work/` の作成を行う。
```
bash scripts/setup.bash
```

## 構成
- `work/` : 解答 (`foo.cpp`)、入力生成 (`bar.cpp`)、出力整形 (`baz.cpp`)、デバッグ用入力 (`input.txt`)。git 管理外
- `templates/` : `work/` のひな形。`work/` の編集はここに反映されない
- `stew/` : 自作ライブラリ。`expand.py` で `make_default.hpp` を1ファイルに展開できる。各ヘッダの名前空間に `test()` があり、`test.cpp` から呼ぶ
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
