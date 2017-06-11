# UEDirectInputPadPlugin
UE4.10(Windows)以降で、DirectInputのゲームパッド(以下DIPad)を使用できるようするプラグインです。  

# 特徴
- 複数のゲームパッドに対応
- UE4のゲームパッドイベントと統合

## 複数のゲームパッド対応
複数のゲームパッドがPCに認識されてる場合、見つかった順番にPlayerIndexを割り当てます。  
最大でPlayerIndex(8つ)分認識します。

XInputパッドがすでに存在していた場合は、XInputパッドを優先します。  
つまり、XInputパッドの次からPlayerIndexが割り振られます。

## UE4のゲームパッドイベントと統合
UE4が元からもっている「ゲームパッドAボタン」「ゲームパッド左サムスティックX」などのイベントとして入力を受け取れます。
それらを使う限り、入力元のゲームパッドがDIPadなのかXInputパッドなのかを気にする必要はありません。

ただし、DIPadはパッドごとにボタン配置が違いますので、XInputパッドの配置と合わせる(キーキャリブレーション)必要があります。キーキャリブレーションをサポートする機能もあります。

もちろん、DIPad専用のイベントもあります。こちらの場合、最大で6つ軸、1つのPOV、32個のボタンイベントを扱うことができます。

# 使用方法
UE4のプラグインとして入れて有効にするだけで使うことができます。
プラグインのフォルダ名は、「DirectInputPadPlugin」としてください。  
VisualStudioは入れて、ソースからビルドしてください。バイナリ(DLL)はつけていません。

DIPadとしてのイベントは、BPエディタでDIGamePadで検索！

BP公開APIをさっと見たい人は、「DirectInputPadJoystick.h」を見て下さい。

詳しい使い方は、[こちらの記事](http://katze.hatenablog.jp/entry/2015/11/27/191351)を参照してください。

# 使用上の注意
- キーリピート非対応
- フォースフィードバック非対応
- エディタ上でスタンドアローン実行すると、実行開始してから数フレームの間、各種APIが正しい値を返せないことがあります
 - UE4の仕様なので避けることができませんでした

# ToDo
- 動的な抜き差し

# Misc
VS2017/2015でビルドできるのを確認しています。

# サンプルプロジェクト
https://github.com/katze7514/UEDirectInputPadPluginSample

# ライセンス
The MIT License (MIT)

# 参考
https://github.com/Ikarus76/UEJoystickPlugin/

# 連絡先
Twitter : [katze_7514](http://twitter.com/katze_7514)  
blog    : [GameProgrammer's Night](http://katze.hatenablog.jp/)
