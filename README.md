# SEGA Collegeポートフォリオ 

セガカレッジ「プログラマ3Days就業体験」への応募作品として、以下の2作品を掲載しています。

## 提出作品

### 1. デスクトップマスコット

**C# / Unity**

デスクトップ上に3Dキャラクターを表示し、ユーザーの操作や時間経過に応じてキャラクターが動作するデスクトップマスコットアプリです。

主な実装：
- マウスによるキャラクターのドラッグ移動
- ランダムなモーション制御
- キャラクターの向き制御

[Desktop Mascotの詳細を見る](./desktop-mascot/)

---

### 2. ネットワークシミュレータns-3用のWi-Fiシミュレーション

**C++ / ns-3**

複数の端末が同時にWi-Fiを利用する環境を再現し、端末数や通信設定による性能の違いを評価するシミュレーションプログラムです。

主な実装：
- Wi-Fiシミュレーション環境の構築
- UDPアップリンク通信の生成
- CWなどのシミュレーション条件の設定
- スループット・パケット損失率・平均遅延の算出
- CSV形式での結果出力

[Wi-Fiシミュレーションの詳細を見る](./wifi-simulation/)

## リポジトリ構成

```text
sega-college-portfolio/
├── desktop-mascot/
│   ├── README.md
│   └── src/
│       ├── DragModelWithMouse.cs
│       ├── FaceCamera.cs
│       ├── JumpPosition.cs
│       └── RandomAnimatorController.cs
│
└── wifi-simulation/
    ├── README.md
    └── src/
        └── wifi-cw.cc
