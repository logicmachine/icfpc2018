## 使い方
### 事前準備
* [score_calculator](https://github.com/logicmachine/icfpc2018/tree/master/tsakai/score_calculator) でテストした結果のnbtファイルと、score_calculatorでテストした結果のcsvファイルが含まれているディレクトリを用意する。（複数可能、nbtの欠損があっても問題ない）
* dfltTracesF (公式ページで提供されているnbtファイル)を用意する。
 * .nbtに欠損があった場合に使われます。
* full.confのprivateIDにチームのprivateIDを書く。
* 必要なら、full.confの設定を変更する。(nbtファイルを格納するフォルダとzipファイルの名前を設定できる)

### 実行
* 結果のzipファイルと、sha256をファイル名に持つtxtが生成される。

```
$ python merge.py
$ bash gen_zip.sh
$ ls -l *.txt *.zip
-rw-rw-r-- 1 tsakai tsakai 18152539  7月 23 15:53 submission_1.zip
-rw-rw-r-- 1 tsakai tsakai        0  7月 23 15:53 submission_1_has_e255f8b04ddc3344fbfbbcce139510950348fc19afd9381cf4910f265656acd5.txt
```

