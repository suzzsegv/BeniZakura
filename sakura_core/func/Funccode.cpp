/*!	@file
	@brief 機能分類定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, MIK, Stonee, asa-o, Misaka, genta, hor
	Copyright (C) 2002, aroka, minfu, MIK, ai, genta
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, isearch
	Copyright (C) 2005, genta, MIK
	Copyright (C) 2006, aroka, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

//	Sept. 14, 2000 Jepro note: functions & commands list
//	キーワード：コマンド一覧順序
//	ここに登録されているコマンドが共通設定の機能種別に表示され、キー割り当てにも設定できるようになる
//	このファイルは「コマンド一覧」のメニューの順番や表示にも使われている
//	sakura_rc.rcファイルの下のほうにあるString Tableも参照のこと

#include "StdAfx.h"
#include "func/Funccode.h"
#include "config/maxdata.h" //MAX_MRU
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "doc/CEditDoc.h"
#include "_main/CAppMode.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "macro/CSMacroMgr.h"
#include "window/CEditWnd.h"
#include "docplus/CDiffManager.h"
#include "CMarkMgr.h"	// CAutoMarkMgr

//using namespace nsFuncCode;

const TCHAR* nsFuncCode::ppszFuncKind[] = {
//	"--未定義--",	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更
//	Oct. 16, 2000 JEPRO 表示の順番をメニューバーのそれに合わせるように少し入れ替えた(下の個別のものも全部)
	_T("ファイル操作系"),
	_T("編集系"),
	_T("カーソル移動系"),
	_T("選択系"),		//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	_T("矩形選択系"),	//Oct. 17, 2000 JEPRO 「選択系」に一緒にすると多くなりすぎるので「矩形選択系」も独立させた
	_T("クリップボード系"),
	_T("挿入系"),
	_T("変換系"),
	_T("検索系"),
	_T("モード切り替え系"),
	_T("設定系"),
	_T("マクロ系"),
	//	Oct. 15, 2001 genta カスタムメニューの文字列をは動的に変更可能にするためここからは外す．
//	_T("カスタムメニュー"),	//Oct. 21, 2000 JEPRO 「その他」から独立分離化
	_T("ウィンドウ系"),
	_T("支援"),
	_T("その他")
};
const int nsFuncCode::nFuncKindNum = _countof(nsFuncCode::ppszFuncKind);


/* ファイル操作系 */
const EFunctionCode pnFuncList_File[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	F_FILENEW			,	//新規作成
	F_FILENEW_NEWWINDOW	,	//新規ウインドウを開く
	F_FILEOPEN			,	//開く
	F_FILEOPEN_DROPDOWN	,	//開く(ドロップダウン)
	F_FILESAVE			,	//上書き保存
	F_FILESAVEAS_DIALOG	,	//名前を付けて保存
	F_FILESAVEALL		,	//全て上書き保存	// Jan. 24, 2005 genta
	F_FILECLOSE			,	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	F_FILECLOSE_OPEN	,	//閉じて開く
	F_WINCLOSE			,	//ウィンドウを閉じる	//Oct.17,2000 コマンド本家は「ウィンドウ系」	//Feb. 18, 2001	JEPRO 下から移動した
	F_FILESAVECLOSE		,	//保存して閉じる Feb. 28, 2004 genta
	F_FILE_REOPEN		,	//開き直す	//Dec. 4, 2002 genta
	F_FILE_REOPEN_SJIS		,//SJISで開き直す
	F_FILE_REOPEN_JIS		,//JISで開き直す
	F_FILE_REOPEN_EUC		,//EUCで開き直す
	F_FILE_REOPEN_LATIN1	,//Latin1で開き直す	// 2010/3/20 Uchi
	F_FILE_REOPEN_UNICODE	,//Unicodeで開き直す
	F_FILE_REOPEN_UNICODEBE	,//UnicodeBEで開き直す
	F_FILE_REOPEN_UTF8		,//UTF-8で開き直す
	F_FILE_REOPEN_CESU8		,//CESU-8で開き直す
	F_FILE_REOPEN_UTF7		,//UTF-7で開き直す
	F_FILE_EXCLUSIVE_LOCK,
	F_FILE_SHARE_LOCK,
	F_FILE_UNLOCK,
	F_PRINT				,	//印刷
	F_PRINT_PREVIEW		,	//印刷プレビュー
	F_PRINT_PAGESETUP	,	//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	F_OPEN_HfromtoC		,	//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
//	F_OPEN_HHPP			,	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	F_OPEN_CCPP			,	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	F_ACTIVATE_SQLPLUS			,	/* Oracle SQL*Plusをアクティブ表示 */	//Sept. 20, 2000 「コンパイル」JEPRO アクティブ表示を上に移動した
	F_PLSQL_COMPILE_ON_SQLPLUS	,	/* Oracle SQL*Plusで実行 */	//Sept. 20, 2000 jepro 説明の「コンパイル」を「実行」に統一
	F_BROWSE			,	//ブラウズ
	F_VIEWMODE			,	//ビューモード
	F_PROPERTY_FILE		,	/* ファイルのプロパティ */
	F_EXITALLEDITORS	,	//編集の全終了	// 2007.02.13 ryoji F_WIN_CLOSEALL→F_EXITALLEDITORS
	F_EXITALL				//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
};
const int nFincList_File_Num = _countof( pnFuncList_File );	//Oct. 16, 2000 JEPRO 配列名変更(FuncList5→FuncList_File)


/* 編集系 */
const EFunctionCode pnFuncList_Edit[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	F_UNDO				,	//元に戻す(Undo)
	F_REDO				,	//やり直し(Redo)
	F_DELETE			,	//削除
	F_DELETE_BACK		,	//カーソル前を削除
	F_WordDeleteToStart	,	//単語の左端まで削除
	F_WordDeleteToEnd	,	//単語の右端まで削除
	F_WordCopy			,	//単語コピー
	F_WordCut			,	//単語切り取り
	F_WordDelete		,	//単語削除
	F_LineCutToStart	,	//行頭まで切り取り(改行単位)
	F_LineCutToEnd		,	//行末まで切り取り(改行単位)
	F_LineDeleteToStart	,	//行頭まで削除(改行単位)
	F_LineDeleteToEnd	,	//行末まで削除(改行単位)
	F_CUT_LINE			,	//行切り取り(折り返し単位)
	F_DELETE_LINE		,	//行削除(折り返し単位)
	F_DUPLICATELINE		,	//行の二重化(折り返し単位)
	F_INDENT_TAB		,	//TABインデント
	F_UNINDENT_TAB		,	//逆TABインデント
	F_INDENT_SPACE		,	//SPACEインデント
	F_UNINDENT_SPACE	,	//逆SPACEインデント
	F_LTRIM				,	//左(先頭)の空白を削除	2001.12.03 hor
	F_RTRIM				,	//右(末尾)の空白を削除	2001.12.03 hor
	F_SORT_ASC			,	//選択行の昇順ソート	2001.12.06 hor
	F_SORT_DESC			,	//選択行の降順ソート	2001.12.06 hor
	F_MERGE				,	//選択行のマージ		2001.12.06 hor
	F_RECONVERT			,	//再変換 				2002.04.09 minfu
	F_INSERT_NEWLINE		//改行
//		F_WORDSREFERENCE		//単語リファレンス
};
const int nFincList_Edit_Num = _countof( pnFuncList_Edit );	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)


/* カーソル移動系 */
const EFunctionCode pnFuncList_Move[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	F_UP				,	//カーソル上移動
	F_DOWN				,	//カーソル下移動
	F_LEFT				,	//カーソル左移動
	F_RIGHT				,	//カーソル右移動
	F_UP2				,
	F_DOWN2				,
	F_WORDLEFT			,	//単語の左端に移動
	F_WORDRIGHT			,	//単語の右端に移動
	F_GOLINETOP			,	//行頭に移動(折り返し単位)
	F_GOLINEEND			,	//行末に移動(折り返し単位)
//	F_ROLLDOWN			,	//スクロールダウン
//	F_ROLLUP			,	//スクロールアップ
	F_HalfPageUp		,	//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown		,	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp			,	//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown			,	//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP			,	//ファイルの先頭に移動
	F_GOFILEEND			,	//ファイルの最後に移動
	F_CURLINECENTER		,	//カーソル行をウィンドウ中央へ
	F_JUMP_DIALOG		,	//指定行ヘジャンプ	//Sept. 17, 2000 JEPRO コマンド本家は「検索系」
	F_JUMP_SRCHSTARTPOS	,	//検索開始位置へ戻る	// 02/06/26 ai コマンド本家は｢検索系｣
	F_JUMPHIST_PREV		,	//移動履歴: 前へ
	F_JUMPHIST_NEXT		,	//移動履歴: 次へ
	F_JUMPHIST_SET		,	//現在位置を移動履歴に登録
	F_WndScrollDown		,	//テキストを１行下へスクロール	// 2001/06/20 asa-o
	F_WndScrollUp		,	//テキストを１行上へスクロール	// 2001/06/20 asa-o
	F_GONEXTPARAGRAPH	,	//次の段落へ
	F_GOPREVPARAGRAPH	,	//前の段落へ
	F_AUTOSCROLL		,	//オートスクロール
};
const int nFincList_Move_Num = _countof( pnFuncList_Move );	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)


/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
const EFunctionCode pnFuncList_Select[] = {
	F_SELECTWORD			,	//現在位置の単語選択
	F_SELECTALL				,	//すべて選択
	F_SELECTLINE			,	//1行選択	// 2007.10.06 nasukoji
	F_BEGIN_SEL				,	//範囲選択開始
	F_UP_SEL				,	//(範囲選択)カーソル上移動
	F_DOWN_SEL				,	//(範囲選択)カーソル下移動
	F_LEFT_SEL				,	//(範囲選択)カーソル左移動
	F_RIGHT_SEL				,	//(範囲選択)カーソル右移動
	F_UP2_SEL				,
	F_DOWN2_SEL				,
	F_WORDLEFT_SEL			,	//(範囲選択)単語の左端に移動
	F_WORDRIGHT_SEL			,	//(範囲選択)単語の右端に移動
	F_GOLINETOP_SEL			,	//(範囲選択)行頭に移動(折り返し単位)
	F_GOLINEEND_SEL			,	//(範囲選択)行末に移動(折り返し単位)
//	F_ROLLDOWN_SEL			,	//(範囲選択)スクロールダウン
//	F_ROLLUP_SEL			,	//(範囲選択)スクロールアップ
	F_HalfPageUp_Sel		,	//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown_Sel		,	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp_Sel			,	//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown_Sel			,	//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP_SEL			,	//(範囲選択)ファイルの先頭に移動
	F_GOFILEEND_SEL			,	//(範囲選択)ファイルの最後に移動
	F_GONEXTPARAGRAPH_SEL	,	//(範囲選択)次の段落へ
	F_GOPREVPARAGRAPH_SEL		//(範囲選択)前の段落へ
};
const int nFincList_Select_Num = _countof( pnFuncList_Select );


/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
const EFunctionCode pnFuncList_Box[] = {
//	F_BOXSELALL			,	//矩形ですべて選択
	F_BEGIN_BOX			,	//矩形範囲選択開始
	F_UP_BOX			,	//(矩形選択)カーソル上移動
	F_DOWN_BOX			,	//(矩形選択)カーソル下移動
	F_LEFT_BOX			,	//(矩形選択)カーソル左移動
	F_RIGHT_BOX			,	//(矩形選択)カーソル右移動
	F_UP2_BOX			,
	F_DOWN2_BOX			,
	F_WORDLEFT_BOX		,	//(矩形選択)単語の左端に移動
	F_WORDRIGHT_BOX		,	//(矩形選択)単語の右端に移動
	F_GOLOGICALLINETOP_BOX	,	//(矩形選択)行頭に移動(改行単位)
	F_GOLINETOP_BOX		,	//(矩形選択)行頭に移動(折り返し単位)
	F_GOLINEEND_BOX		,	//(矩形選択)行末に移動(折り返し単位)
	F_HalfPageUp_BOX	,	//(矩形選択)半ページアップ
	F_HalfPageDown_BOX	,	//(矩形選択)半ページダウン
	F_1PageUp_BOX		,	//(矩形選択)１ページアップ
	F_1PageDown_BOX		,	//(矩形選択)１ページダウン
	F_GOFILETOP_BOX		,	//(矩形選択)ファイルの先頭に移動
	F_GOFILEEND_BOX			//(矩形選択)ファイルの最後に移動
};
const int nFincList_Box_Num = _countof( pnFuncList_Box );


/* クリップボード系 */
const EFunctionCode pnFuncList_Clip[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	F_CUT						,	//切り取り(選択範囲をクリップボードにコピーして削除)
	F_COPY						,	//コピー(選択範囲をクリップボードにコピー)
	F_COPY_ADDCRLF				,	//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	F_COPY_CRLF					,	//CRLF改行でコピー
	F_PASTE						,	//貼り付け(クリップボードから貼り付け)
	F_PASTEBOX					,	//矩形貼り付け(クリップボードから矩形貼り付け)
//	F_INSTEXT_W					,	//テキストを貼り付け		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
//	F_ADDTAIL_W					,	//最後にテキストを追加		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
	F_COPYLINES					,	//選択範囲内全行コピー
	F_COPYLINESASPASSAGE		,	//選択範囲内全行引用符付きコピー
	F_COPYLINESWITHLINENUMBER	,	//選択範囲内全行行番号付きコピー
	F_COPY_COLOR_HTML			,	//選択範囲内色付きHTMLコピー
	F_COPY_COLOR_HTML_LINENUMBER,	//選択範囲内行番号色付きHTMLコピー
	F_COPYFNAME					,	//このファイル名をクリップボードにコピー //2002/2/3 aroka
	F_COPYPATH					,	//このファイルのパス名をクリップボードにコピー
	F_COPYTAG					,	//このファイルのパス名とカーソル位置をコピー	//Sept. 14, 2000 JEPRO メニューに合わせて下に移動
	F_CREATEKEYBINDLIST				//キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO IDM_TESTのままではうまくいかないのでFに変えて登録	//Dec. 25, 2000 復活
};
const int nFincList_Clip_Num = _countof( pnFuncList_Clip );	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)


/* 挿入系 */
const EFunctionCode pnFuncList_Insert[] = {
	F_INS_DATE				,	// 日付挿入
	F_INS_TIME				,	// 時刻挿入
	F_CTRL_CODE_DIALOG			//コントロールコードの入力
};
const int nFincList_Insert_Num = _countof( pnFuncList_Insert );


/* 変換系 */
const EFunctionCode pnFuncList_Convert[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	F_TOLOWER				,	//小文字
	F_TOUPPER				,	//大文字
	F_TOHANKAKU				,	/* 全角→半角 */
	// From Here 2007.01.24 maru 並び順変更
	F_TOZENKAKUKATA			,	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	F_TOZENKAKUHIRA			,	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	F_TOZENEI				,	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	F_TOHANEI				,	/* 全角英数→半角英数 */
	F_TOHANKATA				,	/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru 並び順変更
	F_HANKATATOZENKATA		,	/* 半角カタカナ→全角カタカナ */
	F_HANKATATOZENHIRA		,	/* 半角カタカナ→全角ひらがな */
	F_TABTOSPACE			,	/* TAB→空白 */
	F_SPACETOTAB			,	/* 空白→TAB */  //---- Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS		,	/* 自動判別→SJISコード変換 */
	F_CODECNV_EMAIL			,	//E-Mail(JIS→SJIS)コード変換
	F_CODECNV_EUC2SJIS		,	//EUC→SJISコード変換
	F_CODECNV_UNICODE2SJIS	,	//Unicode→SJISコード変換
	F_CODECNV_UNICODEBE2SJIS	,	//Unicode→SJISコード変換
	F_CODECNV_UTF82SJIS		,	/* UTF-8→SJISコード変換 */
	F_CODECNV_UTF72SJIS		,	/* UTF-7→SJISコード変換 */
	F_CODECNV_SJIS2JIS		,	/* SJIS→JISコード変換 */
	F_CODECNV_SJIS2EUC		,	/* SJIS→EUCコード変換 */
	F_CODECNV_SJIS2UTF8		,	/* SJIS→UTF-8コード変換 */
	F_CODECNV_SJIS2UTF7		,	/* SJIS→UTF-7コード変換 */
	F_BASE64DECODE			,	//Base64デコードして保存
	F_UUDECODE					//uudecodeして保存
	//Sept. 30, 2000JEPRO コメントアウトされてあったのを復活させた(動作しないのかも？)
	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更
};
const int nFincList_Convert_Num = _countof( pnFuncList_Convert );	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)


/* 検索系 */
const EFunctionCode pnFuncList_Search[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	F_SEARCH_WORD		,	//現在位置の単語で検索
	F_SEARCH_WORDEND	,	//現在位置から単語末尾までの文字列で検索
	F_SEARCH_DIALOG		,	//検索(単語検索ダイアログ)
	F_SEARCH_BOX		,	//検索(ボックス)
	F_SEARCH_NEXT		,	//次を検索	//Sept. 16, 2000 JEPRO "次"を"前"の前に移動
	F_SEARCH_PREV		,	//前を検索
	F_REPLACE_DIALOG	,	//置換
	F_SEARCH_CLEARMARK	,	//検索マークのクリア
	F_JUMP_SRCHSTARTPOS	,	//検索開始位置へ戻る	// 02/06/26 ai
	F_GREP_DIALOG		,	//Grep
	F_JUMP_DIALOG		,	//指定行ヘジャンプ
	F_OUTLINE			,	//アウトライン解析
	F_OUTLINE_TOGGLE	,	//アウトライン解析(toggle) // 20060201 aroka
	F_TAGJUMP			,	//タグジャンプ機能
	F_TAGJUMP_CLOSE		,	//閉じてタグジャンプ(元ウィンドウを閉じる) // Apr. 03, 2003 genta
	F_TAGJUMPBACK		,	//タグジャンプバック機能
	F_TAGS_MAKE			,	//タグファイルの作成	//@@@ 2003.04.13 MIK
	F_DIRECT_TAGJUMP	,	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
	F_SYMBOL_SEARCH		,	//シンボル検索(キーワード指定タグジャンプ)
	F_COMPARE			,	//ファイル内容比較
	F_DIFF_DIALOG		,	//DIFF差分表示(ダイアログ)
	F_DIFF_NEXT			,	//次の差分へ
	F_DIFF_PREV			,	//前の差分へ
	F_DIFF_RESET		,	//差分の全解除
	F_BRACKETPAIR		,	//対括弧の検索
	F_BOOKMARK_SET		,	//ブックマーク設定・解除
	F_BOOKMARK_NEXT		,	//次のブックマークへ
	F_BOOKMARK_PREV		,	//前のブックマークへ
	F_BOOKMARK_RESET	,	//ブックマークの全解除
	F_BOOKMARK_VIEW		,	//ブックマークの一覧
	F_ISEARCH_NEXT	    ,   //前方インクリメンタルサーチ //2004.10.13 isearch
	F_ISEARCH_PREV		,	//後方インクリメンタルサーチ //2004.10.13 isearch
	F_ISEARCH_REGEXP_NEXT,	//前方正規表現インクリメンタルサーチ  //2004.10.13 isearch
	F_ISEARCH_REGEXP_PREV,	//後方正規表現インクリメンタルサーチ  //2004.10.13 isearch
	F_ISEARCH_MIGEMO_NEXT,	//前方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
	F_ISEARCH_MIGEMO_PREV	//後方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
};
const int nFincList_Search_Num = _countof( pnFuncList_Search );	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)


/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
const EFunctionCode pnFuncList_Mode[] = {
	F_CHGMOD_INS		,	//挿入／上書きモード切り替え
	F_CHG_CHARSET		,	//文字コードセット指定		2010/6/14 Uchi
	F_CHGMOD_EOL_CRLF	,	//入力改行コード指定(CRLF)	2003.06.23 Moca
	F_CHGMOD_EOL_LF		,	//入力改行コード指定(LF)	2003.06.23 Moca
	F_CHGMOD_EOL_CR		,	//入力改行コード指定(CR)	2003.06.23 Moca
	F_CANCEL_MODE			//各種モードの取り消し
};
const int nFincList_Mode_Num = _countof( pnFuncList_Mode );	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)


/* 設定系 */
const EFunctionCode pnFuncList_Set[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	F_SHOWTOOLBAR		,	/* ツールバーの表示 */
	F_SHOWFUNCKEY		,	/* ファンクションキーの表示 */	//Sept. 14, 2000 JEPRO ファンクションキーとステータスバーを入れ替え
	F_SHOWTAB			,	/* タブの表示 */	//@@@ 2003.06.10 MIK
	F_SHOWSTATUSBAR		,	/* ステータスバーの表示 */
	F_TYPE_LIST			,	/* タイプ別設定一覧 */			//Sept. 17, 2000 JEPRO 設定系に入ってなかったので追加
	F_OPTION_TYPE		,	/* タイプ別設定 */
	F_OPTION			,	/* 共通設定 */
	F_FONT				,	/* フォント設定 */
	F_FONT_ZOOM_IN		,	/* フォントの拡大 */
	F_FONT_ZOOM_OUT		,	/* フォントの縮小 */
	F_FONT_ZOOM_RESET	,	/* フォントサイズのリセット */
	F_FONT_MINIMIZE		,	/* フォントサイズ最小化 */
	F_SHOW_TAB_CHAR		,	/* TAB記号の表示(トグル) */
	F_SHOW_EOL_CHAR		,	/* 改行記号の表示(トグル) */
	F_TAB_WIDTH_2		,	/* TAB 幅 2 */
	F_TAB_WIDTH_4		,	/* TAB 幅 4 */
	F_TAB_WIDTH_8		,	/* TAB 幅 8 */
	F_WRAPWINDOWWIDTH	,	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	F_PRINT_PAGESETUP	,	//印刷ページ設定				//Sept. 14, 2000 JEPRO 「印刷のページレイアウトの設定」を「印刷ページ設定」に変更	//Oct. 17, 2000 コマンド本家は「ファイル操作系」
	F_FAVORITE			,	//履歴の管理	//@@@ 2003.04.08 MIK
	F_TMPWRAPNOWRAP		,	//折り返さない（一時設定）			// 2008.05.30 nasukoji
	F_TMPWRAPSETTING	,	//指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
	F_TMPWRAPWINDOW		,	//右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	F_SELECT_COUNT_MODE		//文字カウント設定	// 2009.07.06 syat
};
int		nFincList_Set_Num = _countof( pnFuncList_Set );	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)


/* マクロ系 */
const EFunctionCode pnFuncList_Macro[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
	F_RECKEYMACRO	,	/* キーマクロの記録開始／終了 */
	F_SAVEKEYMACRO	,	/* キーマクロの保存 */
	F_LOADKEYMACRO	,	/* キーマクロの読み込み */
	F_EXECKEYMACRO	,	/* キーマクロの実行 */
	F_EXECEXTMACRO	,	/* 名前を指定してマクロ実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	F_EXECCMMAND		/* 外部コマンド実行 */
	F_EXECMD_DIALOG		/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

};
const int nFincList_Macro_Num = _countof( pnFuncList_Macro);	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)


/* カスタムメニュー */	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#if 0
//	From Here Oct. 15, 2001 genta
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除．
const EFunctionCode pnFuncList_Menu[] = {
	F_MENU_RBUTTON				,	/* 右クリックメニュー */
	F_CUSTMENU_1				,	/* カスタムメニュー1 */
	F_CUSTMENU_2				,	/* カスタムメニュー2 */
	F_CUSTMENU_3				,	/* カスタムメニュー3 */
	F_CUSTMENU_4				,	/* カスタムメニュー4 */
	F_CUSTMENU_5				,	/* カスタムメニュー5 */
	F_CUSTMENU_6				,	/* カスタムメニュー6 */
	F_CUSTMENU_7				,	/* カスタムメニュー7 */
	F_CUSTMENU_8				,	/* カスタムメニュー8 */
	F_CUSTMENU_9				,	/* カスタムメニュー9 */
	F_CUSTMENU_10				,	/* カスタムメニュー10 */
	F_CUSTMENU_11				,	/* カスタムメニュー11 */
	F_CUSTMENU_12				,	/* カスタムメニュー12 */
	F_CUSTMENU_13				,	/* カスタムメニュー13 */
	F_CUSTMENU_14				,	/* カスタムメニュー14 */
	F_CUSTMENU_15				,	/* カスタムメニュー15 */
	F_CUSTMENU_16				,	/* カスタムメニュー16 */
	F_CUSTMENU_17				,	/* カスタムメニュー17 */
	F_CUSTMENU_18				,	/* カスタムメニュー18 */
	F_CUSTMENU_19				,	/* カスタムメニュー19 */
	F_CUSTMENU_20				,	/* カスタムメニュー20 */
	F_CUSTMENU_21				,	/* カスタムメニュー21 */
	F_CUSTMENU_22				,	/* カスタムメニュー22 */
	F_CUSTMENU_23				,	/* カスタムメニュー23 */
	F_CUSTMENU_24				 	/* カスタムメニュー24 */
};
const int nFincList_Menu_Num = _countof( pnFuncList_Menu );	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#endif

/* ウィンドウ系 */
const EFunctionCode pnFuncList_Win[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	F_SPLIT_V			,	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	F_SPLIT_H			,	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	F_SPLIT_VH			,	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	F_WINCLOSE			,	//ウィンドウを閉じる
	F_WIN_CLOSEALL		,	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	F_GREP_WIN_ALL_CLOSE	,	// Grep ウィンドウをすべて閉じる
	F_TAB_CLOSEOTHER	,	//このタブ以外を閉じる	// 2008.11.22 syat
	F_NEXTWINDOW		,	//次のウィンドウ
	F_PREVWINDOW		,	//前のウィンドウ
 	F_WINLIST			,	//開いているウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
	F_CASCADE			,	//重ねて表示
	F_TILE_V			,	//上下に並べて表示
	F_TILE_H			,	//左右に並べて表示
	F_TOPMOST			,	//常に手前に表示
	F_BIND_WINDOW		,	//結合して表示	// 2006.04.22 ryoji
	F_GROUPCLOSE		,	//グループを閉じる	// 2007.06.20 ryoji
	F_NEXTGROUP			,	//次のグループ	// 2007.06.20 ryoji
	F_PREVGROUP			,	//前のグループ	// 2007.06.20 ryoji
	F_TAB_MOVERIGHT		,	//タブを右に移動	// 2007.06.20 ryoji
	F_TAB_MOVELEFT		,	//タブを左に移動	// 2007.06.20 ryoji
	F_TAB_SEPARATE		,	//新規グループ	// 2007.06.20 ryoji
	F_TAB_JOINTNEXT		,	//次のグループに移動	// 2007.06.20 ryoji
	F_TAB_JOINTPREV		,	//前のグループに移動	// 2007.06.20 ryoji
	F_TAB_CLOSELEFT 	,	//左をすべて閉じる		// 2008.11.22 syat
	F_TAB_CLOSERIGHT	,	//右をすべて閉じる		// 2008.11.22 syat
	F_MAXIMIZE			,	//ウィンドウを最大化
	F_MAXIMIZE_V		,	//縦方向に最大化
	F_MAXIMIZE_H		,	//横方向に最大化 //2001.02.10 by MIK
	F_MINIMIZE_ALL		,	//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	F_REDRAW			,	//再描画
	F_WIN_OUTPUT		,	//アウトプットウィンドウ表示
};
const int nFincList_Win_Num = _countof( pnFuncList_Win );	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)


/* 支援 */
const EFunctionCode pnFuncList_Support[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	F_HOKAN						,	/* 入力補完 */
	F_TOGGLE_KEY_SEARCH			,	/* キャレット位置の単語を辞書検索する機能ON/OFF */	// 2006.03.24 fon
	F_MENU_ALLFUNC				,	/* コマンド一覧 */
	F_ABOUT							/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加
};
const int nFincList_Support_Num = _countof( pnFuncList_Support );	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)


/* その他 */	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
const EFunctionCode pnFuncList_Others[] = {
	F_EXEC_CALC,
	F_EXEC_CMD_PROMPT,
	F_EXEC_EXPLORER,
	F_EXEC_THG_ANNOTATE
};
const int nFincList_Others_Num = _countof( pnFuncList_Others );	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)





const int nsFuncCode::pnFuncListNumArr[] = {
//	nFincList_Undef_Num,	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
	nFincList_File_Num,		/* ファイル操作系 */	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	nFincList_Edit_Num,		/* 編集系 */			//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	nFincList_Move_Num,		/* カーソル移動系 */	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	nFincList_Select_Num,	/* 選択系 */			//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
	nFincList_Box_Num,		/* 矩形選択系 */		//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	nFincList_Clip_Num,		/* クリップボード系 */	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	nFincList_Insert_Num,	/* 挿入系 */
	nFincList_Convert_Num,	/* 変換系 */			//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	nFincList_Search_Num,	/* 検索系 */			//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	nFincList_Mode_Num,		/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
	nFincList_Set_Num,		/* 設定系 */			//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	nFincList_Macro_Num,	/* マクロ系 */			//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除
//	nFincList_Menu_Num,		/* カスタムメニュー */	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
	nFincList_Win_Num,		/* ウィンドウ系 */		//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	nFincList_Support_Num,	/* 支援 */				//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	nFincList_Others_Num	/* その他 */			//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
};
const EFunctionCode* nsFuncCode::ppnFuncListArr[] = {
//	pnFuncList_Undef,	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
	pnFuncList_File,	/* ファイル操作系 */	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	pnFuncList_Edit,	/* 編集系 */			//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	pnFuncList_Move,	/* カーソル移動系 */	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	pnFuncList_Select,/* 選択系 */			//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動  (矩形選択)は新設され次第ここにおく
	pnFuncList_Box,	/* 矩形選択系 */		//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	pnFuncList_Clip,	/* クリップボード系 */	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	pnFuncList_Insert,/* 挿入系 */
	pnFuncList_Convert,/* 変換系 */			//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	pnFuncList_Search,/* 検索系 */			//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	pnFuncList_Mode,	/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
	pnFuncList_Set,	/* 設定系 */			//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	pnFuncList_Macro,	/* マクロ系 */			//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除
//	pnFuncList_Menu,	/* カスタムメニュー */	//Oct. 21, 2000 JEPRO「その他」から分離独立化
	pnFuncList_Win,	/* ウィンドウ系 */		//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	pnFuncList_Support,/* 支援 */				//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	pnFuncList_Others	/* その他 */			//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
};
const int nsFuncCode::nFincListNumArrNum = _countof( nsFuncCode::pnFuncListNumArr );




/* 機能が利用可能か調べる */
bool IsFuncEnable( CEditDoc* pcEditDoc, DLLSHAREDATA* pShareData, EFunctionCode nId )
{
	/* 書き換え禁止のときを一括チェック */
	if( pcEditDoc->IsModificationForbidden( nId ) )
		return false;

	switch( nId ){
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_sFlags.m_hwndRecordingKeyMacro == CEditWnd::getInstance()->GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				return true;
			}else{
				return false;
			}
		}else{
			return true;
		}
	case F_SAVEKEYMACRO:	/* キーマクロの保存 */
		//	Jun. 16, 2002 genta
		//	キーマクロエンジン以外のマクロを読み込んでいるときは
		//	実行はできるが保存はできない．
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_sFlags.m_hwndRecordingKeyMacro == CEditWnd::getInstance()->GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				return true;
			}else{
				return false;
			}
		}else{
			return CEditApp::getInstance()->m_pcSMacroMgr->IsSaveOk();
		}
	case F_EXECKEYMACRO:	/* キーマクロの実行 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_sFlags.m_hwndRecordingKeyMacro == CEditWnd::getInstance()->GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				return true;
			}else{
				return false;
			}
		}else{
			//@@@ 2002.1.24 YAZAKI m_szKeyMacroFileNameにファイル名がコピーされているかどうか。
			if (pShareData->m_Common.m_sMacro.m_szKeyMacroFileName[0] ) {
				return true;
			}else{
				return false;
			}
		}
	case F_LOADKEYMACRO:	/* キーマクロの読み込み */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_sFlags.m_hwndRecordingKeyMacro == CEditWnd::getInstance()->GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				return true;
			}else{
				return false;
			}
		}else{
			return true;
		}
	case F_EXECEXTMACRO:	/* 名前を指定してマクロ実行 */
		return true;

	case F_SEARCH_CLEARMARK:	//検索マークのクリア
		return true;

	// 02/06/26 ai Start
	case F_JUMP_SRCHSTARTPOS:	// 検索開始位置へ戻る
		if( pcEditDoc->m_pcEditWnd->GetActiveView().m_ptSrchStartPos_PHY.BothNatural() ){
			return true;
		}else{
			return false;
		}
	// 02/06/26 ai End

	case F_COMPARE:	/* ファイル内容比較 */
		if( 2 <= pShareData->m_sNodes.m_nEditArrNum ){
			return true;
		}else{
			return false;
		}

	case F_DIFF_NEXT:	/* 次の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:	/* 前の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:	/* 差分の全解除 */	//@@@ 2002.05.25 MIK
		if( !CDiffManager::getInstance()->IsDiffUse() ) return false;
		return true;
	case F_DIFF_DIALOG:	/* DIFF差分表示 */	//@@@ 2002.05.25 MIK
		//if( pcEditDoc->IsModified() ) return false;
		//if( ! pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;
		return true;

	case F_BEGIN_BOX:	//矩形範囲選択開始
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		if( pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
			return true;
		}else{
			return false;
		}
	case F_PASTEBOX:
		/* クリップボードから貼り付け可能か？ */
		if( pcEditDoc->m_cDocEditor.IsEnablePaste() && pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH ){
			return true;
		}else{
			return false;
		}
	case F_PASTE:
		/* クリップボードから貼り付け可能か？ */
		if( pcEditDoc->m_cDocEditor.IsEnablePaste() ){
			return true;
		}else{
			return false;
		}

	case F_FILENEW:	/* 新規作成 */
	case F_GREP_DIALOG:	/* Grep */
		/* 編集ウィンドウの上限チェック */
		if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			return false;
		}else{
			return true;
		}

	case F_FILESAVE:	/* 上書き保存 */
		if( !CAppMode::getInstance()->IsViewMode() ){	/* ビューモード */
			if( pcEditDoc->m_cDocEditor.IsModified() ){	/* 変更フラグ */
				return true;
			}
			else if (pcEditDoc->m_cDocFile.IsChgCodeSet()) {	// 文字コードの変更
				return true;
			}
			else {
				/* 無変更でも上書きするか */
				if( !pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite ){
					return false;
				}else{
					return true;
				}
			}
		}else{
			return false;
		}
	case F_COPYLINES:				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	//選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:				//選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	//選択範囲内行番号色付きHTMLコピー
		//テキストが選択されていればtrue
		return pcEditDoc->m_pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_TOLOWER:					/* 小文字 */
	case F_TOUPPER:					/* 大文字 */
	case F_TOHANKAKU:				/* 全角→半角 */
	case F_TOHANKATA:				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI:					/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI:					/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA:			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKATA:		/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA:		/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE:				/* TAB→空白 */
	case F_SPACETOTAB:				/* 空白→TAB */  //---- Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:	/* Unicode→SJISコード変換 */
	case F_CODECNV_UNICODEBE2SJIS:	/* UnicodeBE→SJISコード変換 */
	case F_CODECNV_UTF82SJIS:		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			/* Base64デコードして保存 */
	case F_UUDECODE:				//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更
		// テキストが選択されていればtrue
		return pcEditDoc->m_pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_CUT_LINE:	//行切り取り(折り返し単位)
	case F_DELETE_LINE:	//行削除(折り返し単位)
		// テキストが選択されていなければtrue
		return !pcEditDoc->m_pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_UNDO:		return pcEditDoc->m_cDocEditor.IsEnableUndo();	/* Undo(元に戻す)可能な状態か？ */
	case F_REDO:		return pcEditDoc->m_cDocEditor.IsEnableRedo();	/* Redo(やり直し)可能な状態か？ */

	case F_COPYPATH:
	case F_COPYTAG:
	case F_COPYFNAME:					// 2002/2/3 aroka
	case F_OPEN_HfromtoC:				//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
//	case F_OPEN_HHPP:					//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	case F_OPEN_CCPP:					//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	case F_PLSQL_COMPILE_ON_SQLPLUS:	/* Oracle SQL*Plusで実行 */
	case F_BROWSE:						//ブラウズ
	//case F_VIEWMODE:					//ビューモード	//	Sep. 10, 2002 genta 常に使えるように
	//case F_PROPERTY_FILE:				//ファイルのプロパティ	// 2009.04.11 ryoji コメントアウト
		return pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath();	// 現在編集中のファイルのパス名をクリップボードにコピーできるか

	case F_JUMPHIST_PREV:	//	移動履歴: 前へ
		if( pcEditDoc->m_pcEditWnd->GetActiveView().m_cHistory->CheckPrev() )
			return true;
		else
			return false;
	case F_JUMPHIST_NEXT:	//	移動履歴: 次へ
		if( pcEditDoc->m_pcEditWnd->GetActiveView().m_cHistory->CheckNext() )
			return true;
		else
			return false;
	case F_JUMPHIST_SET:	//	現在位置を移動履歴に登録
		return true;
	// 20100402 Moca (無題)もダイレクトタグジャンプできるように
	case F_DIRECT_TAGJUMP:	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
	case F_SYMBOL_SEARCH:	//シンボル検索(キーワード指定タグジャンプ)
	//	2003.05.12 MIK タグファイル作成先を選べるようにしたので、常に作成可能とする
//	case F_TAGS_MAKE:	//タグファイルの作成	//@@@ 2003.04.13 MIK
		if( false == CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode
			&& pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
			return true;
		}else{
			return false;
		}
	
	//タブモード時はウインドウ並べ替え禁止です。	@@@ 2003.06.12 MIK
	case F_TILE_H:
	case F_TILE_V:
	case F_CASCADE:
		//Start 2004.07.15 Kazika タブウィンド時も実行可能
		return true;
		//End 2004.07.15 Kazika
	case F_BIND_WINDOW:	//2004.07.14 Kazika 新規追加
	case F_TAB_MOVERIGHT:	// 2007.06.20 ryoji 追加
	case F_TAB_MOVELEFT:	// 2007.06.20 ryoji 追加
	case F_TAB_CLOSELEFT:	// 2009.12.26 syat 追加
	case F_TAB_CLOSERIGHT:	// 2009.12.26 syat 追加
		//非タブモード時はウィンドウを結合して表示できない
		return pShareData->m_Common.m_sTabBar.m_bDispTabWnd != FALSE;
	case F_GROUPCLOSE:		// 2007.06.20 ryoji 追加
	case F_NEXTGROUP:		// 2007.06.20 ryoji 追加
	case F_PREVGROUP:		// 2007.06.20 ryoji 追加
		return ( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin );
	case F_TAB_SEPARATE:	// 2007.06.20 ryoji 追加
	case F_TAB_JOINTNEXT:	// 2007.06.20 ryoji 追加
	case F_TAB_JOINTPREV:	// 2007.06.20 ryoji 追加
	case F_FILENEW_NEWWINDOW:	// 2011.11.15 syat 追加
		return ( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin );
	}
	return true;
}



/* 機能がチェック状態か調べる */
bool IsFuncChecked( CEditDoc* pcEditDoc, DLLSHAREDATA*	pShareData, EFunctionCode nId )
{
	CEditWnd* pCEditWnd;
	// Modified by KEITA for WIN64 2003.9.6
	pCEditWnd = ( CEditWnd* )::GetWindowLongPtr( CEditWnd::getInstance()->GetHwnd(), GWLP_USERDATA );
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより、プレビュー判定削除
	ECodeType eDocCode = pcEditDoc->GetDocumentEncoding();
	switch( nId ){
	case F_FILE_REOPEN_SJIS:		return CODE_SJIS == eDocCode;
	case F_FILE_REOPEN_JIS:			return CODE_JIS == eDocCode;
	case F_FILE_REOPEN_EUC:			return CODE_EUC == eDocCode;
	case F_FILE_REOPEN_LATIN1:		return CODE_LATIN1 == eDocCode;		// 2010/3/20 Uchi
	case F_FILE_REOPEN_UNICODE:		return CODE_UNICODE == eDocCode;
	case F_FILE_REOPEN_UNICODEBE:	return CODE_UNICODEBE == eDocCode;
	case F_FILE_REOPEN_UTF8:		return CODE_UTF8 == eDocCode;
	case F_FILE_REOPEN_CESU8:		return CODE_CESU8 == eDocCode;
	case F_FILE_REOPEN_UTF7:		return CODE_UTF7 == eDocCode;
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			if( pShareData->m_sFlags.m_hwndRecordingKeyMacro == CEditWnd::getInstance()->GetHwnd() ){	/* キーボードマクロを記録中のウィンドウ */
				return true;
			}else{
				return false;
			}
		}else{
			return false;
		}
	case F_SHOWTOOLBAR:			return pCEditWnd->m_cToolbar.GetToolbarHwnd() != NULL;
	case F_SHOWFUNCKEY:			return pCEditWnd->m_CFuncKeyWnd.GetHwnd() != NULL;
	case F_SHOWTAB:				return pCEditWnd->m_cTabWnd.GetHwnd() != NULL;	//@@@ 2003.06.10 MIK
	case F_SHOWSTATUSBAR:		return pCEditWnd->m_cStatusBar.GetStatusHwnd() != NULL;
	// 2008.05.30 nasukoji	テキストの折り返し方法
	case F_TMPWRAPNOWRAP:		return ( pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP );		// 折り返さない
	case F_TMPWRAPSETTING:		return ( pcEditDoc->m_nTextWrapMethodCur == WRAP_SETTING_WIDTH );		// 指定桁で折り返す
	case F_TMPWRAPWINDOW:		return ( pcEditDoc->m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH );		// 右端で折り返す
	// 2009.07.06 syat  文字カウント方法
	case F_SELECT_COUNT_MODE:	return ( pCEditWnd->m_nSelectCountMode == SELECT_COUNT_TOGGLE ?
											pShareData->m_Common.m_sStatusbar.m_bDispSelCountByByte == TRUE :
											pCEditWnd->m_nSelectCountMode == SELECT_COUNT_BY_BYTE );
	// Mar. 6, 2002 genta
	case F_VIEWMODE:			return CAppMode::getInstance()->IsViewMode(); //ビューモード
	//	From Here 2003.06.23 Moca
	case F_CHGMOD_EOL_CRLF:		return EOL_CRLF == pcEditDoc->m_cDocEditor.GetNewLineCode();
	case F_CHGMOD_EOL_LF:		return EOL_LF == pcEditDoc->m_cDocEditor.GetNewLineCode();
	case F_CHGMOD_EOL_CR:		return EOL_CR == pcEditDoc->m_cDocEditor.GetNewLineCode();
	//	To Here 2003.06.23 Moca
	//	2003.07.21 genta
	case F_CHGMOD_INS:			return pcEditDoc->m_cDocEditor.IsInsMode();	//	Oct. 2, 2005 genta 挿入モードはドキュメント毎に補完するように変更した
	case F_TOGGLE_KEY_SEARCH:	return pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord != FALSE;	//	2007.02.03 genta キーワードポップアップのON/OFF状態を反映する
	case F_BIND_WINDOW:			return ((pShareData->m_Common.m_sTabBar.m_bDispTabWnd) && !(pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin));	//2004.07.14 Kazika 追加
	case F_TOPMOST:				return ((DWORD)::GetWindowLongPtr( pCEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST) != 0;	// 2004.09.21 Moca
	// Jan. 10, 2004 genta インクリメンタルサーチ
	case F_ISEARCH_NEXT:
	case F_ISEARCH_PREV:
	case F_ISEARCH_REGEXP_NEXT:
	case F_ISEARCH_REGEXP_PREV:
	case F_ISEARCH_MIGEMO_NEXT:
	case F_ISEARCH_MIGEMO_PREV:
		return pcEditDoc->m_pcEditWnd->GetActiveView().IsISearchEnabled( nId );
	case F_OUTLINE_TOGGLE: // 20060201 aroka アウトラインウィンドウ
		// ToDo:ブックマークリストが出ているときもへこんでしまう。
		return pcEditDoc->m_pcEditWnd->m_cDlgFuncList.GetHwnd() != NULL;
	}
	//End 2004.07.14 Kazika

	return false;
}
