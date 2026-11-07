atcoder用の環境。

直下に ACL の `atcoder/` を置いてから使う。
```
git clone --depth 1 https://github.com/atcoder/ac-library.git
mv ac-library/atcoder .
rm -rf ac-library
```

## 構成
- `work/` : 解答 (`foo.cpp`)、入力生成 (`bar.cpp`)、出力整形 (`baz.cpp`)、デバッグ用入力 (`input.txt`)
- `stew/` : 自作ライブラリ。`expand.py` で `make_default.hpp` を1ファイルに展開できる
- `scripts/` : ビルド (`build_cpp.bash`) とランダムテスト (`find.py`)
- `build/` : 生成物 (`*.exe`, `default.hpp`)。git 管理外

## 使い方
```
bash scripts/build_cpp.bash            # foo をビルド
bash scripts/build_cpp.bash foo bar baz
python3 scripts/find.py 100
```
