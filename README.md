﻿# 紅桜エディタ

## はじめに

MS-DOS 時代、MIFES や VZ と共に一世を風靡した RED2 というエディタがありました。この RED2 ライクな操作性を
サクラエディタ(Sakura Editor)で実現するために派生させたものが「紅桜」です。（RED SAKURA で「紅桜」です。）  
RED2 の多くの機能は、サクラエディタの機能で実現可能ですので、キーバインドやメニューデフォルトの変更が主ですが、
一部、機能拡張も行なっています。

## 変更点

---
### 2014/12/01 - Ver.2014.12.1.21
* 「SVN Rev.3737: Fix: 1023文字を超える長さの補完をするとバッファオーバーランする(f394c03f6f60)」に相当する修正  
	未反映のチャンジセットである「SVN Rev.3161(8860d8feab21)」に依存していたため、旧ソース用に変更して適用。  

	http://sourceforge.net/p/sakura-editor/patchunicode/831/  
	長すぎる候補は、補完ファイル・補完プラグインから設定できます。ファイル内容から補完する場合は長さ制限が適用されています。  

* 「SVN Rev.3738: Fix: タイプ別設定のインポートで約122文字以上のファイル名を持つ色設定があるとバッファオーバーラン」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/832/  

* 「SVN Rev.3739: Fix: タイプ別設定の名前拡張子タイプ数が多いとファイルダイアログで異常終了する」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/834/  

* 「タイプ別設定」ダイアログを閉じると「色の設定」の「作成した色」が失われてしまう不具合を修正  
	「SVN Rev.3742(c078de6a149d): Fix: 作成した色の保持」に相当する変更。  
	未反映のチャンジセットである「SVN Rev.3450(1b5867d4a772): Keep: CPropCommonクラスとCPropTypesクラスの動的生成」に
	依存していたため、必要な変更を追加して適用。

---
### 2014/11/01 - Ver.2014.11.1.20
* 「SVN Rev.3664: Fix: GetDocumentationの修正」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/782/  

* 「SVN Rev.3672: Keep: 構造体のコピーでmemcpy_rawを使わない」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/797/  

* 「SVN Rev.3697: Fix: gccでお気に入りダイアログでフリーズする」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/791/  

* 「SVN Rev.3698: Fix: mingwでのコンパイル警告・バグの修正」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/792/  
	mingw環境での警告-Wallでの問題などを修正します。  
	・ふぁんくらぶ part16 >>476 windressで "\'" がunrecognized escape sequence  
	・未使用変数の削除  
	・アウトライン解析ルールファイルの改行を取り除く処理の修正  
	・プラグインフォルダ検索時の条件式修正  
	・タイプ別フォント選択時の等幅チェックの修正  

* 「SVN Rev.3700: Fix: 折り返し位置への挿入での再描画不足(r3069-)」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/802/  

* 「SVN Rev.3714: Fix: 行頭禁則と行末禁則の初期値」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/807/  
	- 行頭禁則  
	重複している文字を変更しました(U+FFE0 -> U+00A2)。  
	- 行末禁則  
	重複している文字を変更しました(U+FFE1 -> U+00A3）。  
	円記号を追加しました(U+00A5)。  

* 「SVN Rev.3717: Fix: アクティブ化時にキャプション更新されないことがある(r2929-)」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/815/  
	[r2929]の変更を元に戻します。  

* 「SVN Rev.3724」の変更を適用  
	Fix: 強調キーワード長の設定・コピーがおかしいのを修正  
	Fix: 強調キーワードの文字列コピーのバッファオーバーランの可能性の修正  

* 「SVN Rev.3727: Fix: 対括弧強調の状態で選択すると対括弧が強調されたままになる」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/814/  

* 「SVN Rev.3728: Fix: マクロで引数無しのSearchNext()/SearchPrev()でエラー」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/818/  

	紅桜では未適用であった「SVN Rev.3160: New: ダイアログ表示/結果適用のマクロを追加」に依存していたため、
	必要な変更を追加して適用。  

* 「SVN Rev.3730: Fix: 文字幅に合わせてスペースを詰めるのヘルプID修正」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/820/  
	IDが「ラインモード貼り付けを可能にする」と重複していたため変更します。  

* 「SVN Rev.3731: Fix: ウィンドウの位置と大きさの位置に-(マイナス記号)を直接入力できない」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/822/  
	マイナスを直接入力できず、UpDownコントロールで一度マイナスにする必要があったのを修正します  

* 「SVN Rev.3732: Fix: トレイメニューのホットキーをなしにするとなんでも反応する」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/823/  
	>サクラエディタが「非アクティブ」のとき、F1キーを押すとサクラエディタのメニューが出てきます。  
	F1キーに関しては再現しませんでしたが、私の環境だと「なし」にすると、
	マウスパッドの拡大縮小をするとトレイメニューが反応します。  
	「なし」のときは登録しないようにし、これを反応しないようにします。  

* 「SVN Rev.3733: Fix: マクロパスのブロック範囲外でのアクセスの修正」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/824/  

* 「SVN Rev.3735: Fix: タグジャンプの!_TAG_S_SEARCH_NEXTが正常に処理されない」の変更を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/827/  
	tagsファイルに次のタグファイルの場所を指定する "!_TAG_S_SEARCH_NEXT" の処理がおかしくて
	次のタグが検索されないことがあるバグを修正します。  

---
### 2014/10/01 - Ver.2014.10.1.19
* 「ダイレクトタグジャンプリスト」、「アウトライン解析」の表示に編集用フォントを使用するように変更  
	高 DPI 環境において、文字が小さくて読みにくいため、編集画面で使用しているフォントとフォントサイズを、
	以下に適用するように変更。
	- 「ダイレクトタグジャンプリスト」のリストビュー
	- 「アウトライン解析」のリストビュー
	- 「アウトライン解析」のツリービュー

* タイプ別設定: C/C++: デフォルト値に「関数を強調表示するための正規表現キーワード」を追加。  
	正規表現は SakuraEditorWiki の以下のページのものを使用。  
	http://sakura.qp.land.to/?%C0%B5%B5%AC%C9%BD%B8%BD%2F%C5%EA%B9%C6#w02a741e

* デフォルトの表示色を変更  
	コメント: 00d000 -> 00c000 (１段暗い緑に)  
	シングルクォーテーション文字列: 48d0c0 -> ff3030 (ダブルクォーテーション文字列と同じ赤に)  
	正規表現キーワード1: デフォルト有効に変更し、40ffff -> 98fb98 (シアン系の色からpalegreenに)  

* 「SVN Rev.3626」の修正を適用  
	Keep: CFileLoadの変換でEOLも含めるように  
	Fix: ファイル読み込み進捗で改行コード長が抜けていた  

	http://sourceforge.net/p/sakura-editor/patchunicode/736/  
	pUnicodeBuffer->AppendString(EOL)でメモリが再確保されることがあるので、cLineBufferの段階でEOLを含めるようにします。  
	M_nReadLengthに改行コード長が抜けていたバグが修正されてました。(ファイル読み込みなどで100%にならない)  
	・余分な代入を削除  

* 「SVN Rev.3638: Fix: CTipWndクラスのメンバ変数の初期化」の修正を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/779/  
	m_hFontをコンストラクタで初期化するようにしました。Create() 呼び出し時に、Debug ビルドのときは
	m_hFont = 0xcdcdcdcd なので、DeleteObject() を呼び出しますが、Release ビルド時は m_hFont = 0 になるので
	大丈夫みたいです。

* 「SVN Rev.3640: Fix: 現在位置の単語選択」不具合修正を適用  
	http://sourceforge.net/p/sakura-editor/patchunicode/781/  
	[r3417]以降、現在位置の単語選択が再実行できない不具合を修正しました。

---
### 2014/08/01 - Ver.2014.8.1.18
* 共通設定: ウィンドウ -> タイトルバー のデフォルト値に「排他制御の状態」を表示するパラメータを追加  
	共通設定 -> ファイル -> ファイルの排他制御 にて、ファイルの排他制御を有効にした場合、タイトルバーに
	"(排他: 上書き禁止)" もしくは "(排他: 読み書き禁止)" と表示する。

* タイプ別設定: Markdown を追加  
	ファイルの拡張子は ".md"。初期状態で Markdown 用の正規表現キーワードがいくつか登録されています。

* 「カーソル行背景色」有効時に、マウスで矩形選択を行うと、カーソル行背景色が残ってしまう 不具合を修正  
	「SVN Rev.3618: Fix: マウスによる矩形選択でカーソル行背景色の描画不正」に相当する修正。

* 「SVN Rev.3623: Fix: 印刷プレビューで設定変更するとメモリーリーク」不具合修正を適用  

* 「SVN Rev. 3624: Fix: 整数オーバーフローの修正」に相当する不具合修正  
	未反映のチャンジセットである「SVN Rev.3177: New: 文字幅取得マクロの追加」に依存していたため、この変更を一部マージして適用。
	以下のページに簡単な説明はあるが、不具合の再現方法が不明であるため、修正確認はできていない。  

	http://sourceforge.net/p/sakura-editor/patchunicode/755/  
	大きいファイルでフォントサイズを大きくしている場合などに、落ちたり、描画がおかしかったりするのを修正します。

---
### 2014/06/01 - Ver.2014.6.1.17

* ExpandParameter の条件に「ファイルの排他制御状態」を評価する 'X' を追加  
	排他制御のモードを切り替えてファイルをオープンする機能を実現するための予備実装。
	以下の指定で使用可能だが、共有メモリの内容で判定しているため、このままではあまり役に立つ機能ではない。  
	${X?$:(排他: 上書き禁止)$:(排他: 読み書き禁止)$}

* 「Explorer 起動」時にカレントディレクトリの explorer を実行してしまう 不具合を修正  
	「Explorer 起動」コマンドの実行時に、パスを指定せずに explorer.exe を起動していたため、同じ名前の実行ファイルが
	カレントパス（編集中のファイルパス）などに存在していた場合には、こちらが起動していた。
	この場合でも Windows 標準の explorer が起動するように、Windows ディレクトリのパスを指定して explorer.exe を起動するように変更した。

* 正規表現検索に \ のみ指定すると、エラーダイアログが２度表示されてしまう 不具合を修正  
	「SVN Rev.3609, 3610: Fix: 不正な正規表現検索で不要なメッセージが出る」に相当する修正。
	検索ダイアログを開き、「正規表現」にチェックを入れた状態で「条件」に「\」と入力し、検索を実行すると、「unmatched separator」、
	「検索条件を指定してください。」と２度、エラーダイアログが表示されていたため、「unmatched separator」のダイアログのみ表示されるように
	変更されている。
	また、置換ダイアログも同じ動作だったため、同様に修正されている。

* 検索、置換、Grepダイアログ、および、メインメニュー再設定時のハンドルリーク不具合を修正  
	「SVN Rev.3602: Fix: フォント、メニューのハンドルリークの修正」に相当する修正。以下のハンドルリークが修正されている。
	- 検索、置換、Grepダイアログを開いて閉じた際のフォントハンドルリーク
	- メインメニュー再設定時のハンドルリーク

	また、upatchid:764 には「設定のフォントラベルのハンドルリーク修正」も行ったとの記述があり、以下の操作に関する処理が修正されているが、
	修正の前後で差異は見受けられなかった。
	- 共通設定 -> 支援 タブ -> キーワードヘルプ -> フォント変更
	- 共通設定 -> タブバー タブ -> タブの概観 -> フォント変更

---
### 2014/04/01 - Ver.2014.4.1.16

* Command: 「Explorer 起動」機能を追加  
	キー割り当ての「その他」種別に、「Explorer 起動」を追加。
	この機能は、編集中ファイルの格納されたフォルダウィンドウを起動する。
	デフォルトで Shift + Ctrl + F キーにバインド。

* 行番号の最少幅を 2 桁から 4 桁に変更  
	ファイルの総行数によって、横方向の表示位置が変わらないように。
	また、1000 万行で表示がロールオーバーするように変更。
	（このあたりが動作確認できる上限であるため。）

* 「指定行へジャンプ」ダイアログの機能改善  
	- テキストボックスの幅を広げて、入力可能桁数を増加させた
	- テキストボックスに数値のみ入力できるように変更
	- スピンコントロールの上限値を 1000 万行に
	- 0 行目を指定した場合は、不正値が入力された扱いに変更
	- 不正値が入力された場合に、先頭行にジャンプしてしまう不具合を修正

* Java 強調キーワードを別セットで定義するように変更  
	import と package、基本型とラッパークラスを別セットで登録。

* C/C++ のソースコードを編集中にアウトライン解析を行うと異常終了する場合があった不具合を修正  
	サクラエディタ向けの以下の修正パッチを適用。  
	http://sourceforge.net/p/sakura-editor/patchunicode/786/  
	上記パッチにより、以下の不具合が修正されている。
	- wcsncat() 使用方法の誤りによるバッファーオーバーラン
	- 文字列長の更新漏れによるバッファーオーバーラン

* C/C++ の不完全なソースコードを編集中にアウトライン解析を行うと異常終了する不具合を修正  
	以下のような記述途中の不完全なソースコードを解析すると、不正番地アクセスにより異常終了してしまう。

        template <> unsigned long testFunc<int (int arg)
        {
            printf("testFunc() : %d\n", arg);
        }

	関数名の解析が正常に終了していない場合には、関数名の登録処理を迂回するように修正した。

* コントロールプロセス、および、新しいエディタプロセスが起動するまでのタイムアウトを 10 秒から 60 秒に変更  
	PC の負荷が高い状態（ウィルスチェック中など）でタグジャンプを行って新しいウィンドウを開くと
	「プロセスの起動に失敗しました。」というダイアログが表示される場合がある。
	対策として、プロセス起動完了までのタイムアウトを延ばして暫定対策とする。
	また、初回起動時に生成されるコントロールプロセスの起動待ちタイムアウトも 10 秒から 60 秒に変更した。

	関連:  
		Request/455: コントロールプロセスの起動に失敗した旨のメッセージが表示された
		場合、［OK］ボタンを押下しなくてもサクラエディタを起動して欲しい。
		(http://sakura.qp.land.to/?Request%2F455)

* デバッグビルド版の実行ファイルが起動できない不具合を修正  
	サクラエディタの以下のチェンジセットを反映。
	- SVN Rev.3133: Fix: シングルトンのアサートに引っかかる

* 実行ファイルのプロパティ「製品バージョン」の表記を、カンマ区切りからピリオドに変更  
	「SVN Rev.3608: Fix: 一部のバージョン表記がカンマ区切りで表示される」に相当する変更。

---
### 2014/03/01 - Ver.2014.03.01.15

* C/C++ プリプロセッサコメント (#if 1 ～ #else ～ #endif) のカラーリングに対応  
	\#if 1 ～ #else ～ #endif 中の #else ～ #endif 部分をプリプロセッサコメントと判定してカラーリングを行うように変更。
	ただし、プリプロセッサのネストは未対応です。

* カラーリング処理の速度改善  
	「Rev.ba0bb4663620 C/C++ プリプロセッサコメント（#if 0）のコメントネスト対応」以降、暫定実装であったカラーリングの
	終了判定を見なおして、編集時の動作速度を改善。

* 「各種モードの取り消し」機能の実行時に「検索マークの切り替え（クリア）」も行うように変更  
	ESC キー押下時に、検索結果のハイライト表示をクリアするため。

* キーバインドのデフォルトを変更  
	* Shift + Ctrl + A : TortoiseHg annotate 起動

* About ダイアログ  
	- サクラエディタのバージョン表記、コピーライトの年を修正
	- ビルドに使用した Visual Sutdio のバージョン表記を追加

* Build: Windows XP 向けのバイナリ生成用に VC2008 のプロジェクトファイルを更新  

---
### 2014/02/01 - Ver.2014.02.01.14

* TortoiseHg の annotate ウィンドウを起動する機能を追加  
	キー割り当ての「その他」種別に、「TortoiseHg annotate 起動」機能を追加。
	この機能は、外部コマンド実行で "thg.exe annotate \$F --line \$y" を指定した場合と同じ動作を行います。
	TortoiseHg Ver.2.10 以降のバージョンを使用してください。(Ver.2.10.2 で動作を確認しています。）

* サクラエディタ Ver.2.1.1.0 - SVN Rev.3593 までの不具合修正を一部適用  
	容易にマージ可能だったものに関して、いくつか適用しています。影響がありそうな修正は、以下の通りです。
	- SVN Rev.3283: タブバーの「閉じるボタン」が ON の場合に、タブバーのフォント指定が効かない
	- SVN Rev.3528: カーソル形状が MS-DOS 風の場合に、挿入/上書きを切り替えるとカーソル表示がおかしくなる
	- SVN Rev.3545: タブを閉じた後、直前にアクティブだったタブを表示するように変更

---
### 2014/01/01 - Ver.2014.01.01.13

* ビルド環境を「Visual C++ 2008 Express」から 「Visual Studio Express 2013 for Windows Desktop」に変更  
	プラットフォーム ツールセットは v120 を使用しているため、このバージョン以降の紅桜は Windows Vista 
	以降の OS でのみ動作します。（Windows XP では動作しません。）

* アプリケーションにマニフェストを埋め込むように変更  
	マニフェストファイルがない状態でも、Windows XP 以降の UI スタイルになるように。

* ビルドオプションを「サイズ優先」から「速度優先」に変更

---
### 2013/11/01 - Ver.2013.11.01.12

* ダイレクトタグジャンプ: ファイル名によるタグジャンプに対応  
	タグファイル生成時に「ファイル先頭行のエントリ」を出力するように変更。
	(ctags のオプションに "--extra=+f" を追加)
	「ダイレクトタグジャンプ」ダイアログの「種類」に file を追加。

* 共通設定: タグファイル作成時の「サブフォルダも対象にする」チェックボックスのデフォルトを有効に変更  

* 共通設定: タブバー「閉じるボタン」のデフォルトを有効に変更  

---
### 2013/08/01 - Ver.2013.08.01.11

* キーバインドのデフォルトを変更  
	- F12 : ブックマークの一覧
	- Ctrl + PgDn : 次のウィンドウ
	- Ctrl + PgUp : 前のウィンドウ
	- Alt + N : 次のウィンドウ
	- Alt + P : 前のウィンドウ
	- Shift + Ctrl + C : ファイルの最後に移動
	- Shift + Ctrl + R : ファイルの先頭に移動
	- Shift + Ctrl + T : 行末まで削除(改行単位)
	- Shift + Ctrl + V : 行頭まで削除(改行単位)  
	RED2 の Ctrl キーのうち、割り当てできていなかった機能を Shift + Ctrl に割り当てました。

* 表示色のデフォルトを変更  
	URL: Blue -> Deep Periwinkle に変更。  
	(黒地の青は見づらかったので、薄い青に変更しました。)

* アプリケーションのアイコンを変更  
	アプリケーションのアイコンを Windows Vista Generic Document ベースのアイコンに変更しました。  
	（Windows8 では関連付けしたファイルがすべてアプリケーションのアイコンになってしまうため、
	紅桜の情けないアイコンが大量に表示されていました。）  
	ついでに Grep ウィンドウのアイコンも Vista のアイコンに変更しました。

* サクラエディタ SVN Rev.3115 までの変更を反映  
	サクラエディタ Ver.2.1.0.0 - SVN Rev.3241 までのチェンジセットのうち、容易にマージ可能だったものに関しては、
	いくつか適用しています。
	未反映のチャンジセットに関しては 0.0.0.9 と同様です。

---
### 2013/07/01 - Ver.2013.07.01.10

* タグファイル作成 ダイアログ: タグ作成ファルダの初期値に VCS リポジトリルートを設定するように変更  

* タグファイル作成 ダイアログ: "C/C++ prototype, external and forward declarations を追加" チェックボックスを追加  
	チェックボックスを ON にすると、C 言語のプロトタイプ宣言、外部宣言、前置宣言もタグに追加されます。  
	(ctags のオプションに "--c-kinds=+px" を追加)

* サクラエディタ Ver.2.0.8.1 - SVN Rev.3019 までの変更を反映  
	未反映のチャンジセットに関しては 0.0.0.9 と同様です。

---
### 2013/06/01 - Ver 0.0.0.9

* Grep 時にバイナリファイルを除外するように変更  
	ファイルの先頭から 1KB に 0x00 (1byte) が含まれていた場合には、バイナリファイルと判断して Grep の対象から除外する。  
	制限事項:  
	この判定方法では UTF16 形式のファイルがバイナリファイルとして判断され、除外されてしまう。

* C/C++ 強調キーワードを追加  
	* 不足していたキーワードや、よく使うキーワードを追加
	* 定数キーワードを別グループで登録  
	サクラエディタ SVN Rev.2999 の代わりに、個人的に使用するキーワードを追加しました。

* タイプ別設定: アセンブラ: GNU Assembler 対応  
	* Assembler ファイルの拡張子に ".s" を追加
	* GNU Assembler、C/C++互換プリプロセッサのキーワードを追加して、カラーリングを有効に

* リポジトリ clone 直後の状態でビルドできない不具合を修正  
	svnrev.h が存在しない場合にビルドに失敗していたため、参照しないよう修正しました。

* サクラエディタ SVN Rev.3009 までの変更を反映  
	未反映チャンジセットの増分は以下の通りです。
	- SVN Rev.2868: 「TAB矢印表示の仕様変更」と「TAB矢印の太字が反映されない」不具合の修正
	- SVN Rev.2999: Imp: C/C++キーワード更新

---
### 2013/05/01 - Ver 0.0.0.8

* Grep 時に ".bzr", ".git", ".hg", ".svn" フォルダを除外するように変更  

* Grep 時に VCS リポジトリルートを検索対象ファルダの初期値に設定する機能を追加  
	Grep ダイアログの「フォルダの初期値をカレントフォルダにする」がチェックされている場合に、
	編集中のファイルが格納されたディレクトリを起点に、親ディレクトリ方向に ".bzr", ".git", ".hg", ".svn"
	ディレクトリを検索して、これらのディレクトリが発見されたフォルダを、検索対象の初期値として設定する機能を追加。
	VCS リポジトリが発見できなかった場合には、今までどおり、編集中のファイルが格納されたディレクトリを設定する。

* C/C++ プリプロセッサコメント（#if 0）の判定において、同一行内で完結しているコメントアウト部分は無視するように変更  
	以下のように、コメント内に #if 0 が記述されている場合には無視できるように対応。

        \#if 0
            \#if 0
                \#if 0
                \#endif /* #if 0 */
            \#endif // #if 0
        \#endif

* \#1: 「キー割り当て」のインポートを行うと暴走してしまう 不具合を修正  
	サクラエディタの以下の変更によるデグレード。  
	- Rev.4d6bb991fa10 Keep: CShareData\_IOクラス変更(CommonSetting\_KeiBind関連)

	サクラエディタ Ver.2.0.7.1 の方では修正されているようなので、紅桜で取り込めていない
	以下の変更で修正されていたものと思われる。
	- SVN Rev.2505: Fix: 古いキー割り当て設定をImportすると誤動作する
	- SVN Rev.2511: Imp: キーコードからコマンドを検索する時のループを除去
	- SVN Rev.2530: Fix: Importしたキーの名前が内部のバッファサイズより長いとメモリ破壊する

	紅桜のキー定義ファイルはサクラエディタと互換性がない（キー定義数が異なる）ため、
	キー定義数の異なるファイルはインポートできないように変更した。
	付随して、旧形式（？）のキー定義ファイルのインポート処理も削除した。

* 共通設定: GREP フォルダの初期値をカレントフォルダにする: 有効に変更  

* 共通設定: カーソル位置の文字列をデフォルトの検索文字列にする: 無効に変更  

* タイプ別設定: 「入力補完 強調キーワード」のデフォルトを有効に変更  

* 「タグジャンプ」ダイアログの縦幅を拡張  
	ダイアログのリサイズ対応（サクラエディタ Rev.2754）により、ウィンドウのスタイルが変更になった影響で
	「サイズ変更グリップ」と「キャンセルボタン」が重なって描画されていたため。

* About ダイアログ: バージョン表示に「Mercurial タグ + 相対リビジョン」を使用するように変更  

* サクラエディタ Ver.2.0.7.1 - SVN Rev.2836 までの変更を反映  
	未反映のチャンジセットに関しては 0.0.0.7 と同様です。

---
### 2013/04/01 - Ver 0.0.0.7

* サクラエディタ Ver.2.0.6.0 - SVN Rev.2743 までの変更を反映  
	未反映チャンジセットの増分は以下の通りです。  
	- SVN Rev.2643: Keep: Windows2000以降サポート  
	紅桜は WindowsXP 以降をサポートのため。

---
### 2013/03/01 - Ver 0.0.0.6

* サクラエディタ SVN Rev.2614 までの変更を反映  
	未反映のチャンジセットに関しては 0.0.0.4 と同様です。

---
### 2013/02/15 - Ver 0.0.0.5

* サクラエディタ SVN Rev.2563 までの変更を反映  
	SVN Rev.2544: 「強調キーワードによる補完機能」を使用したかったため更新しました。  
	未反映のチャンジセットに関しては 0.0.0.4 と同様です。

---
### 2013/02/01 - Ver 0.0.0.4

* C/C++ プリプロセッサによるコメントのネスト対応にて #ifndef の判定が漏れていた不具合を修正  

* サクラエディタ SVN Rev.2535 までの変更を反映  
	ただし、以下の変更に関しては、変更が衝突していたため未反映です。
	* SVN Rev.2505: Fix: 古いキー割り当て設定をImportすると誤動作する
	* SVN Rev.2511: Imp: キーコードからコマンドを検索する時のループを除去
	* SVN Rev.2530: Fix: Importしたキーの名前が内部のバッファサイズより長いとメモリ破壊する

---
### 2013/01/01 - Ver 0.0.0.3

* アプリケーションのアイコンを変更  

* C/C++ プリプロセッサによるコメントのネスト対応  
	\#if 0 ～ #endif がネストした状態でも正しくカラーリングできるよう改良。
	ただし、暫定実装の部分が残っているため、カラーリング処理の速度は低下しているはずです。

* タイプ別設定: 「行の間隔」のデフォルトを 1 -> 0 に変更  

* １行の最大桁数を 10240 -> 1024 に変更  

* サクラエディタ SVN Rev.2513 までの変更を反映  
	ただし、以下の変更に関しては、変更が衝突していたため未反映です。
	* SVN Rev.2505: Fix: 古いキー割り当て設定をImportすると誤動作する
	* SVN Rev.2511: Imp: キーコードからコマンドを検索する時のループを除去

---
### 2012/12/01 - Ver 0.0.0.2

* ルーラーを 8 文字単位で表示するように変更  

* 検索 ダイアログ:「見つからないときにメッセージを表示」チェックボックスを削除  

* ジャンプ ダイアログ: PL/SQL 関連の機能を削除  

* バージョン情報 ダイアログ: コントロールの配置を調整  

* 検索や GREP 時の「英大文字と小文字を区別する」のデフォルトを「有効」に変更  

* 共通設定: 「ファイルの排他制御」のデフォルト値を「上書きを禁止する」から「しない」に変更  

* 共通設定: GREP 結果のリアルタイム表示を「有効」に変更  

* 共通設定: ウィンドウ非アクティブ時のタイトルバー表示を、ウィンドウアクティブ時と同一に  

* タイプ別設定: C/C++, Java のスマートインデントのデフォルトを「無効」に変更  

* デフォルトの表示色を RED2 ライクな配色に変更  
	* ルーラーの背景色をグリーンに
	* TAB を暗いブルーに
	* 改行記号をイエローに
	* 行番号をグレーに

* サクラエディタ SVN Rev.2498 までの変更を反映  

---
### 2012/11/01 - Ver 0.0.0.1 (サクラエディタ Ver.2.0.5.0 からの変更点)

* 背景色のデフォルトを「黒地」ベースに変更  

* キーバインドのデフォルトを RED2 ライクなバインドに変更  
	ただし、Windows 標準機能に関する CTRL + Z, X, C, V など、サクラエディタのキーバインドを継承している部分も多いです。
	参考までに、RED2 とキーバインドが異なる CTRL キーのバインドは以下のとおりです。

	|キー       |RED2                      |紅桜             |
	|-----------|--------------------------|-----------------|
	|CTRL + C   |ファイルの最後にジャンプ  |コピー           |
	|CTRL + F   |１語先（右）に進む        |検索ダイアログ   |
	|CTRL + G   |カーソル上の文字を削除    |Grep ダイアログ  |
	|CTRL + H   |カーソル直前の文字を削除  |未割り当て       |
	|CTRL + J   |大文字小文字変換          |指定行へジャンプ |
	|CTRL + M   |改行入力                  |ブックマーク     |
	|CTRL + O   |改行・タブ表示切替        |アウトライン表示 |
	|CTRL + R   |ファイル先頭にジャンプ    |置換ダイアログ   |
	|CTRL + S   |カーソル左                |上書き保存       |
	|CTRL + V   |カーソルから左の文字を削除|貼り付け         |
	|CTRL + X   |カーソル下                |切り取り         |
	|CTRL + Z   |拡張コントロールキー      |元に戻す         |
	|CTRL + E   |カーソル上                |ウィンドウ一覧   |
	|CTRL + D   |カーソル右                |単語削除         |

	※ CTRL + I は未割り当て状態ですが、 TAB が入力されます

	RED2 の F1 ～ F4 メニューに相当する機能は、カスタムメニューで実現しています。コントロールキーのバインドされて
	いないファイル先頭へのジャンプは F2 - T, ファイル末尾へのジャンプは F2 - B で代用してください。

* 「行頭に移動(折り返し単位)」のデフォルト動作を、空白を無視して移動するように変更  
	サクラエディタは「行頭に移動(折り返し単位)」でカーソルを移動する場合に、行頭のホワイトスペース位置に移動して
	いましたが、RED2 と同様、行頭に移動するように変更しました。（紅桜では SHIFT + ← にバインド）

* 「改行」機能を追加  
	RED2 では CTRL + M にバインドされていた機能ですが、紅桜の CTRL + M はサクラエディタと同様「マーク」機能のままです。
	RED2 と同じキーバインドを実現したい場合に使用してください。

* 「単語取り込み」機能を追加  
	実装はしたものの、デフォルトキーにはバインドされていません。

* 「現在位置～単語末尾の文字で検索」機能を追加  
	RED2 で CTRL + L にバインドされていた「カーソル位置文字列を検索バッファに取り込む」機能。RED2 から機能を拡張して
	おり、検索バッファに取り込むと同時に検索を行います。RED2 と同様、連続実行することで、後続の単語を追加して
	再検索を行います。デフォルトでは ALT + L にバインド。

* 「現在位置の単語で検索」機能を追加  
	上記の「現在位置～単語末尾の文字で検索」と同様の機能ですが、サクラエディタの単語取り込みと同様、カーソルより
	前方向に単語区切りを探して単語を取り込みます。連続実行することで、後続の単語を追加して再検索を行います。
	デフォルトでは CTRL + L にバインド。

* ダブルクリックに「現在位置の単語で検索」機能をバインド  
	上記の機能をダブルクリックにもバインドしていますので、マウスのみで単語検索を行うことができます。

* 単語取り込み時の文字種別に「括弧」を追加  
	括弧は ( ) { } [ ] の 6 文字で、これらの括弧は常に単語の区切りとして認識されます。

* C/C++ ソース編集時に、プリプロセッサを用いたコメントアウトが正しくカラーリングできない問題に対応  
	サクラエディタでは、ブロックコメントのデフォルトに #if 0 と #endif を定義して、プリプロセッサ コメントの
	カラーリングを実現していたため、途中に #else や #elif などが記述されていても、 #endif までコメントとして
	カラーリングされていました。
	対策として、 #if 0 をコメント開始、#endif、#else、#elif をコメント終了として判定を行う処理を追加して、
	カラーリングを行うように変更しています。

* 色指定に「C/C++ プリプロセッサコメント」項目を追加  
	コメントとは別の色を設定可能です。デフォルト色はグレー。

* 挿入モード時の DOS タイプカーソルの高さを、半分から ÷ 2.6 に変更（少し低くした）  
	半分の高さだと "-" の上にカーソルがある場合に文字があるのか無いのか、わからなくなってしまうため。

* 「検索／置換」の対象文字列が見つからなかった場合のダイアログ表示を削除  
	いちいちダイアログが開くのは煩わしかったので削除しました。

* キーボードの「Pause」キーを使用可能に変更  
	ノート PC など「Home」キーが存在しない機種で、代替キーとして使用できるように。デフォルトのキーバインドは「Home」
	と同様、ウィンドウの切り替えに設定されています。

* ウィンドウの「最大化」機能を追加  
	WMed32 と同様、デフォルトでは F12 にバインド。

* C/C++ 強調キーワードのデフォルト定義を３種類に分割  
	プリプロセッサ、データタイプを別々に定義。

* タイプ別設定のうち、個人的に使用頻度の低い設定を削除  
	HTML, SQL, COBOL, AWK, DOS Batch, Pascal, TEX, Perl, VB, Rich Text を削除

* その他、各種デフォルト値の変更  


---
## ソースコード

### ライセンス

サクラエディタの最近の変更部分については「zlib/libpng ライセンス」を適用しているようですので、紅桜の変更部分も
これに倣い、「 zlib / libpng ライセンス」を適用します。

### ソースコード リポジトリ

本家サクラエディタの Subversion リポジトリ内の trunk2 ディレクトリを HgSubversion にて Mercurial リポジトリに
変換したものをベースにしています。
ソースコード リポジトリは bitbucket にて公開しています。

https://bitbucket.org/suzzsegv/benizakura/

## ビルド方法

Visual Studio Express 2013 for Windows Desktop を用いてビルドしています。sakura/BeniZakura\_vs2013.sln をダブルクリックして
IDE を起動後、ターゲットとして Release\_Unicode を選択、ビルドメニューからビルドを行ってください。


