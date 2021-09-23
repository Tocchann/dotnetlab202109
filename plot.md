# 題名

WindowsにおけるUIスレッドの基礎

## 概要

Windows でいうところのUIスレッドがどんなものかを垣間見てみます。  
反応ないとさみしいので、久しぶりに MISAO も動かしてみようと思います。

## ゴールはどうする？

UIスレッドがどんなものかを垣間見る。

## 目次

- はじめに
- まとめ
- それでは本題

※ このセッションに限り #dotnetlab のコメントは MISAO で拾い上げて画面にながしています

## MISAO の紹介

- プロ生の主催者 @jz5 さん作の MISAO を使っています。
- Twitterしか拾い上げることができないのでご了承ください
- DL はこちら
  - https://github.com/jz5/misao-legacy

## はじめに

- 現時点で動いているものを正とする
  - 不特定多数を相手にするので今ある状況は常に正しい
- Windows GUI アプリの話題
  - NTサービスは除外
  - コンソールアプリは除外
- 以下の詳細は解説しない
  - Windows API
  - Window メッセージ
  - ユーザー定義関数
  - 文字コード
- 検証環境
  - Surface Book 2
  - Windows Insider Program 
    - Release Preview ON
    - Windows 11(22000.xxx)
- Windows は搭載メモリ、ストレージサイズで挙動が変わる
  - GPUの違いで挙動が変わることもある
- 古いOSでの検証はしていない
- 昔話は基本的に記憶と当時の書籍だより

## まとめ

### スレッドの特徴

- 一般にWindowsはプログラムごとにプロセス空間を用意
- プロセスには一つ以上のスレッドがある
- プロセス作成時に一緒に作られるスレッドをメインスレッドと呼ぶ
  - メインスレッド以外のスレッドをサブスレッドと呼ぶ
- メインスレッドが終了するとプロセス空間はOSによりクリーンナップ
- スレッドは作成時に指定したプロセス内でのみ動作
  - 一つのスレッドがプロセスをまたいで動作しない
  - プロセスを指定してスレッドを作成するAPI
    - CreateRemateThread
    - CreateRemoteThreadEx
- メインスレッドは必ずしもUIスレッドではない

## スレッドの種類

### メインスレッドとサブスレッド

- メインスレッド
  - プロセス作成時に一緒に作成されるスレッド
  - メインスレッドが終了するとプロセスも終了する
- サブスレッド
  - メインスレッドではないスレッド
  - サブスレッドが終了してもプロセスは終了しない
    - メインスレッドが終了時に動いていたら強制終了

### UIスレッドとワーカースレッド

- UIスレッド
  - メッセージ処理を行う必要のあるスレッド
- ワーカースレッド
  - メッセージ処理を行う必要のないスレッド
  - 非UIスレッドと呼ばれることもある

### UIスレッドで行われていること

- メッセージループをまわす
  - メッセージループとは？
    - メッセージポンプを継続的に呼び出すループのこと
  - メッセージポンプとは？
    - メッセージキューからメッセージを取得し適切なプロシージャにメッセージを送ること
    - メッセージを消費するともいう


### 今日出てくる関数(ユーザー定義関数)

### メイン関数(WinMain)

- 定義
```cpp
int APIENTRY wWinMain(
  _In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR lpCmdLine,
  _In_ int nCmdShow );
```
- アプリケーションが起動したときに呼ばれるユーザー定義関数。
- この関数が終了するとメインプロセスが終了しアプリケーションが終了する

### ウィンドウプロシージャ

- ウィンドウのふるまいを決めるユーザー定義関数
  - メッセージごとに実際の処理を記述する
  - 実際には、OSからメッセージが送られてくるためプログラムとしては受信関数となるが、Windowsにおいては、メッセージをプロシージャが受信するとは考えない

### メッセージ送信関数

- SendMessage
  - ウィンドウにメッセージを直接的に送信して同期的に処理してもらうAPI
  - 別スレッド(プロセスが異なる場合も同様)からの送信はメッセージキューに格納しておき、つぎにメッセージを取得しようとしたタイミングでAPI内部から直接的に送信する
  - 送ったメッセージはその場で処理され、処理が完了すると呼び出し元に戻ってくる
    - 一部例外あり(ReplayMessageを呼び出した場合など)
  - 類似API
    - SendNotifyMessage
    - SendMessageTimeout
    - SendMessageCallback

- PostMessage
  - ウィンドウをメッセージに間接的に送信して後で処理してもらうAPI
  - メッセージはメッセージキューと呼ばれる専用のバッファに蓄えられる
  - メッセージはHWNDをインスタンス化したスレッド上でメッセージループを介在して処理される
  - 類似API
    - PostThreadMessage

### メッセージ取得関数(受信ではない)

- GetMessage
  - メッセージキューを確認しメッセージがあればメッセージを取得
  - メッセージがない場合はメッセージが届くまで無限待機する
- PeekMessage
  - メッセージキューを確認しメッセージがあればメッセージを取得
  - メッセージがない場合も即座にリターンする
- GetQueueStatus
  - メッセージキューにメッセージがあるかを確認するAPI
  - メッセージがあってもなくてもすぐにリターンする
- MsgWaitForMultipleObjects
  - 複数の同期オブジェクト(待機ハンドル)を待機しつつメッセージも待機する待機関数
  - 同期オブジェクトが0個の場合でも利用可能
  - 類似API
    - MsgWaitForMultipleObjectsEx

### そのほかのAPI

- WaitMessage
  - メッセージキューが空ではなくなるまで待機する関数
- DispatchMessage
  - メッセージの宛先ウィンドウのプロシージャにメッセージを送るAPI
- TranslateMessage
  - メッセージがキーボード処理の場合、適切に変換するためのAPI(この結果再びメッセージが生成されることもある)
- IsDialogMessage
  - モードレスダイアログのダイアログメッセージを事前処理するためのAPI
  - 無くてもなんとなくなら動いてしまうので要注意(呼び出さないと挙動が一般的なダイアログと異なる場合が出る)

## UIスレッドとは？

- Windows の根幹をなすメッセージを適切に処理するループを持つスレッド
  - メッセージループ(メッセージポンプ)を持つスレッド
- ウィンドウがあるとは限らない
  - メッセージにはスレッドあてメッセージもある

### UIスレッドになるタイミングはいつ？

- メッセージキューが作成された時？
- メッセージが格納された時？
- メッセージが取得された時？

### メッセージキューが作成されるタイミング

- Windows 10 では、スレッドを作成した時点でメッセージキューが存在する
  - PostThreadMessage が失敗しない
- 全てのスレッドが潜在的にUIスレッドになりえる
  - メッセージポンプが動くとUIスレッドになる
- スレッドと共に作成される
- メッセージを処理しない場合でも処理できるように下準備だけされている(模様)

※特定環境での調査：PostThreadMessage にあるようにキューがない場合を考慮しておく必要がある

メッセージキューが作成されたらUIスレッドではない

### メッセージが格納されたことを検出できるのか？

- SendMessage/PostMessage されたなどの通知はない
- 能動的な識別はウィンドウを作るところくらいしかない。
- HWNDが無くてもメッセージが流れうる
  - 応答なしは、ウィンドウに対しての状態でありプロセスやスレッドの状態ではない

### メッセージを取得したとき？

- GetMessage/PeekMessage しつづければメッセージループは構築できる



### メッセージを処理しなければならないとき

1. そのスレッドでウィンドウを作った時==所属するウィンドウがある  
HWNDはSTA(別スレッドからSend/PostMessage しても作成先ウィンドウのスレッドで処理される)なので、そのスレッドがメッセージを処理しなければ応答なしになる
1. そのスレッドがメッセージを経由して動作する必要があるとき  
COMサーバー(OLEオートメーション)、非同期IOなどのメッセージドリブンなRPC実装


### メッセージキューの構造

- 取得優先順位付き特殊データ集合体
- 優先順位と構造
1. Sendキュー(QS_SENDMESSAGE)
1. Postキュー(QS_POSTMESSAGE)
1. 入力フラグ＆キュー(QS_INPUT)
1. ウィンドウ無効領域フラグ(QS_PAINT)
1. タイマーフラグ(QS_TIMER)

※ 同一スレッドの SendMessage はメッセージキューを経由せずそのままプロシージャが呼ばれる

## UIスレッドを織りなすもの

- メッセージの取得 API
  - PeekMessage
  - GetMessage
- メッセージの同期送信 API
  - SendMessage
  - SendMessageCallback
  - SendMessageTimeout
  - SendNotifyMessage
- メッセージの非同期送信API
  - PostMessage
  - PostThreadMessage(ウィンドウではなくスレッド宛て)

## メッセージループとは？

### メッセージループとは

WindowsのGUIアプリで各アプリの画面をつかさどる HWND が動作するために必要となるメッセージを受信するためのバッファと、そのバッファからメッセージを取り出しプロシージャにメッセージを処理させるための処理ループのこと

基本的にはHWNDにメッセージを届ける仕組みであり、マルチスレッド・マルチプロセスで動く Windows では、プロセス間・スレッド間であってもメッセージを送信できるためそれを確実に受信するための仕組みでもある。

メッセージループを利用してメッセージを処理する仕組みは初期のWindowsから実装されているもので、現在もほぼ当時と変わりのない形で利用可能になっている。

実際Win16として一定の成功を収めた Windows 3.1 の
Windowsのウィンドウ(HWND)はメッセージを処理するメッセージプロシージャで動作を定義しており、メッセージプロシージャにメッセージを届けるためのループがメッセージループとなる。

初期のWindowsからある仕組みで現在のWindowsでもメッセージを処理する形は変わっていない。

Windows のGUIアプリはどのような実装であっても必ず１つ以上のHWNDを持つため、何らかの形でメッセージループが実装されている。


### パターン

- GetMessage 版
  - メッセージが到着するまで待機するループ構造
  - ATLのデフォルト実装
  - WPFの実装(.NET Framework 4.8)
- PeekMessage 版
  - メッセージがあれば処理し、無ければほかのことをするビジーループ型
  - MFCの実装スタイル
  - WinForms の実装(.NET Framework 4.8)
  - メッセージが来ていない場合はメッセージがくるまで待機するAPIを併用
- MsgWaitForMultipleObjects 併用版
  - 複数の同期ハンドルを待機しつつメッセージも待機する複合ループ構造
  - 同期ハンドルは最大 MAXIMUM_WAIT_OBJECTS‎‎-1 個まで(-1はメッセージ待機分)



### GetMessage 版

```cpp
MSG msg;
while( GetMessage( &msg, nullptr, 0, 0 ) )
{
  TranslateMessage( &msg );
  DispatchMessage( &msg );
}
```

### PeekMessage 版

```cpp
MSG msg;
do{
  if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
  {
    if( msg.msg == WM_QUIT )
    {
      break;
    }
    TranslateMessage( &msg );
    DispatchMessage( &msg );
  }
  else
  {
    // このタイミングでメッセージがない時に処理したいものを実行する
    WaitMessage();
  }
}while( msg.msg != WM_QUIT );
```

### MsgWaitForMultipleObjects 版(Ex版も同様なので省略)

```cpp
for(;;)
{
  auto waitResult = MsgWaitForMultipleObjects( waitCounts, waitHandles, FALSE, INFINITE, QS_ALLEVENTS );
  // メソッド呼び出しエラー(パラメータ間違ってるとかそういうパターンが大半)
  if( waitResult == WAIT_FAILED )
  {
    auto lastError = GetLastError();
    // 適切なエラー処理を行ってループを続行するか、続行不能なのでループを抜ける
    break;
  }
  // waitHandles のいずれかがシグナル状態になった
  else if( WAIT_OBJECT_0 <= waitResult && waitResult < WAIT_OBJECT_0+waitcounts )
  {
    // waitHandles[waitResult-WAIT_OBJECT_0]のハンドルがシグナル状態になったので何か処理
  }
  // waitHandles のいずれかが待機不可能状態になった
  else if( WAIT_ABANDONED_0 <= waitResult && waitResult < WAIT_ABANDONED_0+waitCounts )
  {
    // waitHandles[waitResult-WAIT_ABANDONED_0]のハンドルが待機失敗したのでなにか処理(Multexの場合に発生する)
  }
  else if( waitResult == WAIT_OBJECT_0+waitCounts )
  {
    // ここをAfxPumpMessage にして、終了処理を適切にフォローアップすれば、MFC の CWinThread::Run() も置き換え可能
    MSG msg;
    while( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) && msg.msg != WM_QUIT )
    {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
    }
    // WM_QUIT はメッセージループの終了を意味するので終わる
    if( msg.msg == WM_QUIT )
    {
      break;
    }
  }
}
```
### GetMessage の中身っておそらくこんな感じ

```cpp
WINUSERAPI
BOOL WINAPI GetMessageImitation(
    _Out_ LPMSG lpMsg,
    _In_opt_ HWND hWnd,
    _In_ UINT wMsgFilterMin,
    _In_ UINT wMsgFilterMax )
{
  for(;;)
  {
    if( PeekMessage( lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE ))
    {
      return ( lpMsg->message != WM_QUIT ) ? TRUE : FALSE ;
    }
    else if( !WaitMessage() )
    {
      return -1;
    }
  }
}
```
