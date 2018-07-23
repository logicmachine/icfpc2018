## 使い方
### 事前準備
* [score_calculator](https://github.com/logicmachine/icfpc2018/tree/master/tsakai/score_calculator) でテストした結果のnbtファイルと、score_calculatorでテストした結果のcsvファイルが含まれているディレクトリを用意する。（複数可能、nbtの欠損があっても問題ない）
* dfltTracesF (公式ページで提供されているnbtファイル)を用意する。
 * .nbtに欠損があった場合に使われます。
* full.confのprivateIDにチームのprivateIDを書く。
* 必要なら、full.confの設定を変更する。(nbtファイルを格納するフォルダとzipファイルの名前を設定できる)

### 例
* `ikeda_result`と`tmp`に実行結果がおいてある例です。

```
$ ls -al
合計 76
drwxrwxr-x 5 tsakai tsakai  4096  7月 23 15:59 .
drwxrwxr-x 8 tsakai tsakai  4096  7月 23 14:47 ..
drwxrwxrwx 2 root   root   36864  7月 23 14:48 dfltTracesF
-rw-rw-r-- 1 tsakai tsakai   287  7月 23 15:43 full.conf
-rwxrwxr-x 1 tsakai tsakai  1199  7月 23 15:52 gen_zip.sh
drwxrwxrwx 2 root   root    4096  7月 23 15:15 ikeda_result
-rw-rw-r-- 1 tsakai tsakai  2200  7月 23 15:28 merge.py
-rw-rw-r-- 1 tsakai tsakai 10303  7月 23 15:29 result_merged.csv
drwxrwxrwx 2 root   root    4096  7月 23 15:15 tmp

```

### 実行
* 結果のzipファイルと、sha256をファイル名に持つtxtが生成される。

```
$ python merge.py
$ bash gen_zip.sh
$ ls -l *.txt *.zip
-rw-rw-r-- 1 tsakai tsakai 18152539  7月 23 15:53 submission_1.zip
-rw-rw-r-- 1 tsakai tsakai        0  7月 23 15:53 submission_1_has_e255f8b04ddc3344fbfbbcce139510950348fc19afd9381cf4910f265656acd5.txt
```

