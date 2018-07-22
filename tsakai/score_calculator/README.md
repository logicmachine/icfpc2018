## 使い方
* 後でちゃんと書く
* Firefoxだと動かない？

## 
* `pip selenium`
* Chromeの場合は、`score_calculator.py` と同じフォルダにchrome driverを入れる。
 * リンク: http://chromedriver.chromium.org/downloads
 * Chromeの場合は、`self.driver = webdriver.Chrome("./chromedriver.exe")`のコメントアウトを外し、`driver = webdriver.Firefox()`をコメントアウトする
* <s> firefoxの場合は、score_calculator.pyと同じフォルダに gecko driverを入れる。</s>
 * <s>https://github.com/mozilla/geckodriver/releases</s>
 * <s>firefoxの場合は、`driver = webdriver.Firefox()`のコメントアウトを外し、`self.driver = webdriver.Chrome("./chromedriver.exe")`をコメントアウトする</s>


 ## ソースコード内の設定項目

* self.vis: True / Falseを設定する。Trueの場合は、visualizer付で実行される。Falseの場合はvisualizerなしで実行される。
* kProblemDir: 問題ファイルを置いているディレクトリ
* kNBTDir: 結果ファイルを置いているディレクトリ
* rangies: それぞれ、FA, FD, FRの中で、解答を実行する問題の範囲。 = [[1, 20], [1, 167], [1, 20]]
* for q in range(1, 2):  FA, FD, FRの中で、解くものを設定する。 range(1, 2)の場合は、FDのみが解かれる。
 
