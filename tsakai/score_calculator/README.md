## 使い方
* 後でちゃんと書く
* win10 + Firefox/Chrome + python3.7 で動かした。

## 
* `pip selenium`
* Chromeの場合は、`score_calculator.py` と同じフォルダにchrome driverを入れる。
 * リンク: http://chromedriver.chromium.org/downloads
*  firefoxの場合は、score_calculator.pyと同じフォルダに gecko driverを入れる。
 * https://github.com/mozilla/geckodriver/releases

## 設定項目

|設定項目|||
|:-|:-|:-|
|browser|使用するブラウザを指定|Chrome / Firefox|
|visualizer||True/False|
|problem_dir|問題ファイルが入っているディレクトリを指定||
|nbt_dir||nbtファイルが入っているディレクトリを指定|
|FA_solve|FAxxxを使うかどうかを指定|True/False|
|FA_begin|FAxxxの開始するケース番号を指定|数字 (e.g. 1, 2, ...)|
|FA_end|FAxxxの終了するケース番号を指定|数字 (e.g. 1, 2, ...)|
|FD_solve|FDxxxを使うかどうかを指定|True/False|
|FD_begin|FDxxxの開始するケース番号を指定|数字 (e.g. 1, 2, ...)|
|FD_end|FDxxxの終了するケース番号を指定|数字 (e.g. 1, 2, ...)|
|FR_solve|FRxxxを使うかどうかを指定|True/False|
|FR_begin |FRxxxの開始するケース番号を指定| 数字 (e.g. 1, 2, ...) |
|FR_end|FRxxxの終了するケース番号を指定|数字 (e.g. 1, 2, ...)|


## ソースコード内の設定項目
* Chromeを使う場合は、kIsChrome = True。Firefoxを使う場合はkIsChrome = False。
* self.vis: True / Falseを設定する。Trueの場合は、visualizer付で実行される。Falseの場合はvisualizerなしで実行される。
* kProblemDir: 問題ファイルを置いているディレクトリ
* kNBTDir: 結果ファイルを置いているディレクトリ
* rangies: それぞれ、FA, FD, FRの中で、解答を実行する問題の範囲。 = [[1, 20], [1, 167], [1, 20]]
* for q in range(1, 2):  FA, FD, FRの中で、解くものを設定する。 range(1, 2)の場合は、FDのみが解かれる。
 
