#　使い方
* index.htmlをブラウザでそのまま開く、または、適当なウェブサーバに一式を置く。
* index.htmlにアクセスする。

# キャプチャの仕方
## 最初から最後まで保存する
1. Source Model, Traget Model, Trace を設定する。
1. Start Recordingをクリックする。
1. Traceが終わるのを待つ。
1. Traceが終わると、gifの生成が始まる。
1. gifの生成が終わると、自動で気にダウンロードされる。

## 途中から保存/途中まで保存する。
1.  `Source Model`, `Traget Model`, `Trace` を設定する。
1. Executing Traceを実行する。
1. 好きなタイミングでStart Recordingをクリックする
1. 好きなタイミングでStop Recordingをクリックする
1. gifの生成が終わると、自動で気にダウンロードされる。

## 注意点
* バグはあるかもしれないです。
* 外部のライブラリを含んでます。
* 途中で保存しているときに、トレースと同時に処理が入ると重くなる場合は、`Steps per Frame`を1/60にすると、多少マシになるかも？