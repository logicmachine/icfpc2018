# 7/22(日)

* 7:40 起床。問題を読みなおして、朝ごはん
* 10:12 真面目に方針を練る
    * 全体を直方体近似で表現したい
        * 各直方体は全て連結している
        * 面倒を排除するため、排他的に設置する
        * 直方体同士の連結情報はグラフで持っておく
        * 直方体には、塗る/塗らないの2値が振られている
    * Botをたくさん動かしたい！
        * 8-bot 1チーム編成
        * 直方体を一つづつFill/Emptyにしていく
        * 一定数を割ったら、ワラワラと
    * 直方体近似の方法
        * Greedyでよくね??
            * ただし、同じ体積で消せるなら、端の方が高得点
            * Voxelの累積和持っておけば、O(1)で含まれるBoxelの個数は計算できる
                * xor を計算しようとしたら、Boxの体積 - 含まれるvoxel体積でOK
                * しかし、愚直にscanするとO(N^4)かかる...
                    * まぁ乱択 + 局所改善すればいいよね
            * よく考えたら、Voxelのサイズを30以下にする必要はない
                * 30より大きくても、分割して作ればOKなので
        * 今は、余分にでかく作ることは考えないものとする(池田くん案)
            * すると、単純に1x1のVoxelをmergeする操作を考える
            * Merge対象の領域はGreedyでOK
            * Mergeする場所もGreedyでOKでは…?
                * 1万回くらい焼なませば、割と広い領域取れそう
                * こっちの場合は、30x30x30 以下にする必要がある
            * しかし、詰まないように依存関係を定めるにはどうしたらいい?
                * 3次元でbfsする
                * ただし、mergeしたオブジェクトは 8頂点の部分で最もBFS木の深さが浅い点を優先する
                    * そうなると、obj_id -> 深さの対応関係を持っておくのがいいかな(これだけでは辛そう)
            * アルゴリズム
                1. ランダムにVoxel内の1点を選択する
                1. もしそれがmerge済みでないなら、以下を1万回くらい繰り返して、Rangeを作成する
                    * xyz座標±1 伸ばす
                    * xyz座標−1 シフトする
                    * 制約条件として「全てがBoxの内部に含まれている」がある
                    * その場で動的に作成していくと、少し辛い
            * 立方体を敷き詰める時に詰むのを避ける一般的なテクを考えた
                * 禁止エリアという概念を作ろう
                    * Voxelオブジェクトを絶対作る必要がある場所
                    * Boxを積み上げていった時、xyz全部順方向のBoxは置けないように制約を張る
                    * 辛そう…
                    * よく考えると、
                * 厚み1は絶対に逃げ道がある！！
            * 「a x a x a のごく小さい領域を1つのオブジェクトとみなして、その中に含まれる連結なVoxelを1つのbotに任せる、」みたいなのが欲しい
                * そうすると、オイラー路判定（かつ、次数は絶対2以下）をする必要がある
                    * 全てが連結で、次数2以下 or 次数1が2 or 0
        * ゴールが見えたかも！？
    * とにかく、Botが順方向に協調しない限りはどうしようもない
        * まずは、協調Botを作ろう
    * 中島くんとの差分を見つけてしまう
        * よく考えたら、y = 0を全て根に繋げていないから、ダメでは…
        * y = 0 のVecを全てQueueに突っ込めばOK

# 直方体構築アルゴリズム

1. y = 0 のboxelを1つ選び、y = -1にして Queueに突っ込む
1. queue が空でない間、以下を反復させる
    1. queueから、始点オブジェクトを取得
    1. 以下のようにして、直方体を成長させる
        * xyz 1伸ばす(ただし、探索済み領域と被らない)
        * xyz 1減らす
        * 1辺どれかの長さが30以下になるように
    1. 体積が V_t 以上だったら、直方体をFIXして、厚みを全方向に1増やして禁止領域を登録する
    1. 6方向にしか隣接関係を持たないと仮定して、隣接直方体は持っておく
1. 厚みが1でない場合は、6平面分の平面オブジェクトを生やす

# 埋める順序を決定するアルゴリズム

1. bfsツリー構築
    * マージがある場合は、マージ優先
1. 原点から空白地帯にBFSをやる
    * 周囲に自分よりもBFSの深さが深くないやつを列挙する。
    * 適当に消していく

# データ構造

* 各nanobotはActionStateを持っている
    * Move(to)
    * 