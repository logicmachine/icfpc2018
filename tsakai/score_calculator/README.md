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
* settings.confの設定

|設定項目|||
|:-|:-|:-|
|browser|使用するブラウザを指定|Chrome / Firefox|
|visualizer|TrueならExecute Trace、FalseならExecute Trace (Full, No Visualizer) を使う|True/False|
|problem_dir|問題ファイルが入っているディレクトリを指定||
|nbt_dir|nbtファイルが入っているディレクトリを指定| |
|FA_solve|FAxxxを使うかどうかを指定|True/False|
|FA_begin|FAxxxの開始するケース番号を指定。[FA_begin, FA_end]までのケースが実行される|数字で指定 (e.g. 1, 2, ...)|
|FA_end|FAxxxの終了するケース番号を指定|数字で指定 (e.g. 1, 2, ...)|
|FD_solve|FDxxxを使うかどうかを指定|True/False|
|FD_begin|FDxxxの開始するケース番号を指定。[FD_begin, FD_end]までのケースが実行される|数字で指定 (e.g. 1, 2, ...)|
|FD_end|FDxxxの終了するケース番号を指定|数字で指定 (e.g. 1, 2, ...)|
|FR_solve|FRxxxを使うかどうかを指定|True/False|
|FR_begin |FRxxxの開始するケース番号を指定。[FR_begin, FR_end]までのケースが実行される| 数字で指定 (e.g. 1, 2, ...) |
|FR_end|FRxxxの終了するケース番号を指定|数字で指定 (e.g. 1, 2, ...)|



 
