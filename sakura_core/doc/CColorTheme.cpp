/*!
 *	@file
 *	@brief	カラーテーマ (プリセットの配色とカスタム) の定義と適用
 *
 * プリセットの配色表 (doc/CDocTypeSetting.cpp から移動) と、テーマをタイプ別設定の色設定に適用する処理。
 * テーマは全項目の文字色・背景色・太字・下線を決める。「色分け/表示」はテーマも全項目の値を持ち、
 * テーマを切り替えたときにタイプ別設定へ適用する (ApplyColorThemeDisp。その後はタイプごとに変更できる)。
 * テーマの番号は ColorTheme の値 (プリセット) または COLOR_THEME_CUSTOM。ini には名前の文字列で保存する。
 *
 * プリコンパイル済みヘッダー (StdAfx.h) は使用しない (単体テストからも単独でビルドするため)。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/18 新規作成
 */
/*!
 *	Copyright (C) 2026, Suzuki Satoshi
 *
 *	This software is provided 'as-is', without any express or implied
 *	warranty. In no event will the authors be held liable for any damages
 *	arising from the use of this software.
 *
 *	Permission is granted to anyone to use this software for any purpose,
 *	including commercial applications, and to alter it and redistribute it
 *	freely, subject to the following restrictions:
 *
 *		1. The origin of this software must not be misrepresented;
 *		   you must not claim that you wrote the original software.
 *		   If you use this software in a product, an acknowledgment
 *		   in the product documentation would be appreciated but is
 *		   not required.
 *
 *		2. Altered source versions must be plainly marked as such,
 *		   and must not be misrepresented as being the original software.
 *
 *		3. This notice may not be removed or altered from any source
 *		   distribution.
 */

#include "CColorTheme.h"

#include <string.h>
#include <wchar.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       プリセットの配色                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//! 色設定(保存用)
struct ColorInfoIni {
	const WCHAR*	m_pszName;			//!< 項目名
	ColorInfoBase	m_sColorInfoBase;		//!< 色設定
};

static ColorInfoIni ColorInfo_DEFAULT[COLOR_THEME_MAX][COLORIDX_LAST] = {
	/* 紅桜 */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x50,0x00 ),
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル行の背景色",				FALSE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x08,0x08,0x20 ),
		L"カーソル行アンダーライン",			TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB( 0x80,0x80,0xa0 ),	RGB( 0x00,0x00,0x00 ),
		L"行番号(変更行)",					TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x40,0x40,0xa0 ),
		L"TAB記号",							TRUE,		FALSE,		FALSE,		RGB( 0x40,0x10,0x80 ),	RGB( 0x00,0x00,0x00 ),
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"改行記号",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0x00,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"対括弧の強調表示",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0xff,0x40,0x40 ),
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB( 0xd0,0xd0,0x00 ),	RGB( 0xd0,0xd0,0x00 ),
		L"検索文字列",						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		L"検索文字列2",						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		L"検索文字列3",						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		L"検索文字列4",						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		L"検索文字列5",						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB( 0x00,0xa0,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB( 0x80,0x80,0x80 ),	RGB( 0x00,0x00,0x00 ),
		L"シングルクォーテーション文字列",	TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"ダブルクォーテーション文字列",		TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"URL",								TRUE,		FALSE,		TRUE,		RGB( 0x80,0x80,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード1",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x98,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード2",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x60,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x98 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB( 0x98,0xfb,0x98 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB( 0x98,0x98,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		L"ブックマーク",						TRUE ,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0xff,0xff,0x00 ),
	},

	/* サクラエディタ */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ),
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ),
		L"カーソル行の背景色",				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255, 128 ),
		L"カーソル行アンダーライン",			TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ),
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
		L"行番号(変更行)",					TRUE,		TRUE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
		L"TAB記号",							TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ),
		L"改行記号",							TRUE,		FALSE,		FALSE,		RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ),
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ),
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ),
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),
		L"対括弧の強調表示",					TRUE,		TRUE,		FALSE,		RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ),
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB( 131, 162, 211 ),	RGB( 131, 162, 211 ),
		L"検索文字列",						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ),
		L"検索文字列2",						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ),
		L"検索文字列3",						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ),
		L"検索文字列4",						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ),
		L"検索文字列5",						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ),
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ),
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),
		L"シングルクォーテーション文字列",	TRUE,		FALSE,		FALSE,		RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ),
		L"ダブルクォーテーション文字列",		TRUE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
		L"URL",								TRUE,		FALSE,		TRUE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"強調キーワード1",					TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"強調キーワード2",					TRUE,		FALSE,		FALSE,		RGB( 128,   0, 255 ),	RGB( 255, 251, 240 ),
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB(   0,  64, 128 ),	RGB( 255, 251, 240 ),
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 210 ),	RGB( 162, 208, 255 ),
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB(   0, 111,   0 ),	RGB( 189, 253, 192 ),
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB( 213, 106,   0 ),	RGB( 255, 233, 172 ),
		L"ブックマーク",						TRUE ,		FALSE,		FALSE,		RGB( 255, 251, 240 ),   RGB(   0, 128, 192 )
	},

	/* Visual Studio 2012 Dark */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB( 0xdc,0xdc,0xdc ),	RGB( 0x1e,0x1e,0x1e ), // "TXT"
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x33,0x33,0x33 ), // "RUL"
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ), // "CAR"
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x30,0x30 ),	RGB( 0x00,0x00,0x00 ), // "IME"
		L"カーソル行の背景色",				TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x00,0x00,0x00 ), // "CBK"
		L"カーソル行アンダーライン",			TRUE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x1e,0x1e,0x1e ), // "UND"
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x00,0x00,0x00 ), // "CVL"
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB( 0x2b,0x91,0xaf ),	RGB( 0x1e,0x1e,0x1e ), // "LNO"
		L"行番号(変更行)",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "MOD"
		L"TAB記号",							FALSE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x1e,0x1e,0x1e ), // "TAB"
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "SPC"
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "ZEN"
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "CTL"
		L"改行記号",							FALSE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "EOL"
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "RAP"
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB( 0x14,0x48,0x50 ),	RGB( 0x1e,0x1e,0x1e ), // "VER"
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "EOF"
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB( 0xb5,0xce,0xa8 ),	RGB( 0x1e,0x1e,0x1e ), // "NUM"
		L"対括弧の強調表示",					TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x0e,0x45,0x83 ), // "BRC"
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB( 0x33,0x99,0xff ),	RGB( 0x33,0x99,0xff ), // "SEL"
		L"検索文字列",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FND"
		L"検索文字列2",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN2"
		L"検索文字列3",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN3"
		L"検索文字列4",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN4"
		L"検索文字列5",						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN5"
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB( 0x57,0xa6,0x4a ),	RGB( 0x1e,0x1e,0x1e ), // "CMT"
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB( 0x60,0x60,0x60 ),	RGB( 0x1e,0x1e,0x1e ), // "CM2"
		L"シングルクォーテーション文字列",	TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "SQT"
		L"ダブルクォーテーション文字列",		TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "WQT"
		L"URL",								TRUE,		FALSE,		TRUE,		RGB( 0x52,0x63,0xda ),	RGB( 0x1e,0x1e,0x1e ), // "URL"
		L"強調キーワード1",					TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "KW1"
		L"強調キーワード2",					TRUE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "KW2"
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "KW3"
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB( 0xb5,0xce,0xa8 ),	RGB( 0x1e,0x1e,0x1e ), // "KW4"
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW5"
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW6"
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW7"
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW8"
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW9"
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KWA"
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK1"
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK2"
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK3"
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK4"
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK5"
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK6"
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK7"
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK8"
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK9"
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RKA"
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFA"
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFC"
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFD"
		L"ブックマーク",						TRUE ,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x7e,0x65,0x51 ), // "MRK"
	},

	/* Monokai */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB( 0xf8,0xf8,0xf2 ),	RGB( 0x28,0x28,0x28 ),
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x30,0x30,0x30 ),
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB( 0xf8,0xf8,0xf2 ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル行の背景色",				TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル行アンダーライン",			FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x00,0x00,0x00 ),
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x00,0x00,0x00 ),
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x30,0x30,0x30 ),
		L"行番号(変更行)",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x49,0x48,0x3e ),
		L"TAB記号",							FALSE,		FALSE,		FALSE,		RGB( 0x3f,0x3d,0x30 ),	RGB( 0x28,0x28,0x28 ),
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		L"改行記号",							FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x28,0x28,0x28 ),
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x66,0xd9,0xef ),
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"対括弧の強調表示",					TRUE,		FALSE,		TRUE,		RGB( 0xff,0xff,0xff ),	RGB( 0xf9,0x26,0x72 ),
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB( 0x6f,0x6d,0x5e ),	RGB( 0x6f,0x6d,0x5e ),
		L"検索文字列",						TRUE,		TRUE,		TRUE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		L"検索文字列2",						TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		L"検索文字列3",						TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		L"検索文字列4",						TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		L"検索文字列5",						TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		L"シングルクォーテーション文字列",	TRUE,		TRUE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		L"ダブルクォーテーション文字列",		TRUE,		TRUE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		L"URL",								TRUE,		FALSE,		TRUE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード1",					TRUE,		FALSE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード2",					TRUE,		TRUE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB( 0xa6,0xe2,0x2e ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB( 0xfd,0x97,0x1f ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		L"ブックマーク",						TRUE,		FALSE,		FALSE,		RGB( 0x27,0x28,0x22 ),	RGB( 0xe6,0xdb,0x74 ),
	},

	/* MIFES Bule */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0x40, 0x40),	RGB(0x14, 0x14, 0x28),
		L"カーソル行の背景色",				FALSE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0x08, 0x08, 0x20),
		L"カーソル行アンダーライン",			TRUE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		L"行番号(変更行)",					TRUE,		TRUE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		L"TAB記号",							TRUE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		L"改行記号",							TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB(0x3c, 0x3c, 0x7d),	RGB(0x14, 0x14, 0x28),
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		L"対括弧の強調表示",					TRUE,		TRUE,		FALSE,		RGB(0xf5, 0xf5, 0xf5),	RGB(0x00, 0x00, 0xf5),
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xf5, 0xf5, 0xf5),
		L"検索文字列",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		L"検索文字列2",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		L"検索文字列3",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		L"検索文字列4",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		L"検索文字列5",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB(0x46, 0x4b, 0x64),	RGB(0x14, 0x14, 0x28),
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB(0x46, 0x4b, 0x64),	RGB(0x14, 0x14, 0x28),
		L"シングルクォーテーション文字列",	TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		L"ダブルクォーテーション文字列",		TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		L"URL",								TRUE,		FALSE,		TRUE,		RGB(0x80, 0xf5, 0xf5),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード1",					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0xff),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード2",					TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0xff),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0x78),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x82, 0x64),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x82, 0x64),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0x78),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		L"ブックマーク",						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
	},

	/* 秀丸エディタ Black */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		L"テキスト",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"ルーラー",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"カーソル",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"カーソル(IME ON)",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0x40, 0x40),	RGB(0x00, 0x00, 0x00),
		L"カーソル行の背景色",				FALSE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0x18, 0x18, 0x00),
		L"カーソル行アンダーライン",			TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"カーソル位置縦線",					FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"行番号",							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"行番号(変更行)",					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"TAB記号",							TRUE,		FALSE,		FALSE,		RGB(0x50, 0x50, 0x50),	RGB(0x00, 0x00, 0x00),
		L"半角空白"		,					FALSE,		FALSE,		FALSE,		RGB(0xc0, 0xc0, 0xc0),	RGB(0x00, 0x00, 0x00),
		L"日本語空白",						TRUE,		FALSE,		FALSE,		RGB(0xc0, 0xc0, 0xc0),	RGB(0x00, 0x00, 0x00),
		L"コントロールコード",				TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"改行記号",							TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"折り返し記号",						TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"指定桁縦線",						FALSE,		FALSE,		FALSE,		RGB(0x40, 0x40, 0xff),	RGB(0x00, 0x00, 0x00),
		L"EOF記号",							TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"半角数値",							TRUE,		FALSE,		FALSE,		RGB(0x00, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"対括弧の強調表示",					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"選択範囲",							TRUE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0xff, 0xff, 0xff),
		L"検索文字列",						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		L"検索文字列2",						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		L"検索文字列3",						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		L"検索文字列4",						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		L"検索文字列5",						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		L"コメント",							TRUE,		FALSE,		FALSE,		RGB(0x00, 0xd4, 0x00),	RGB(0x00, 0x00, 0x00),
		L"C/C++ プリプロセッサコメント",		TRUE,		FALSE,		FALSE,		RGB(0xd4, 0x00, 0xd4),	RGB(0x00, 0x00, 0x00),
		L"シングルクォーテーション文字列",	TRUE,		FALSE,		FALSE,		RGB(0xff, 0x00, 0x00),	RGB(0x00, 0x00, 0x00),
		L"ダブルクォーテーション文字列",		TRUE,		FALSE,		FALSE,		RGB(0xff, 0x00, 0x00),	RGB(0x00, 0x00, 0x00),
		L"URL",								TRUE,		FALSE,		TRUE,		RGB(0xff, 0x40, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード1",					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード2",					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード3",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード4",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード5",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード6",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード7",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード8",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード9",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"強調キーワード10",					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード1",				TRUE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード2",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード3",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード4",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード5",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード6",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード7",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード8",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード9",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"正規表現キーワード10",				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		L"DIFF差分表示(追加)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"DIFF差分表示(変更)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"DIFF差分表示(削除)",				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		L"ブックマーク",						TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x80, 0x00),
	}
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     テーマの名前と番号                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! テーマの名前 (選択肢の表示順)
struct SColorThemeName {
	int				nColorTheme;	//!< テーマ番号
	const WCHAR*	pszDispName;	//!< 表示名
	const wchar_t*	pszKeyName;		//!< ini に保存する名前
};

static const SColorThemeName COLOR_THEME_NAMES[COLOR_THEME_SEL_COUNT] = {
	{ COLOR_THEME_BENIZAKURA,		L"紅桜",					L"BeniZakura" },
	{ COLOR_THEME_SAKURA,			L"サクラエディタ",		L"Sakura" },
	{ COLOR_THEME_HIDEMARU_BLACK,	L"秀丸エディタ Black",	L"HidemaruBlack" },
	{ COLOR_THEME_MIFES_BLUE,		L"MIFES Blue",			L"MifesBlue" },
	{ COLOR_THEME_MONOKAI,			L"Monokai",				L"Monokai" },
	{ COLOR_THEME_VS2012DARK,		L"VS2012 Dark",			L"VS2012Dark" },
	{ COLOR_THEME_CUSTOM,			L"カスタム",				L"Custom" },
};

/*!
 * GetColorThemeBySelIndex - 選択肢の位置 (表示順) から、テーマ番号を返す
 *
 *	@return	テーマ番号 (位置が範囲外の場合は COLOR_THEME_BENIZAKURA)
 */
int GetColorThemeBySelIndex(
	int		nSelIndex	//!< [in] 選択肢の位置 (0 ～ COLOR_THEME_SEL_COUNT - 1)
)
{
	if( nSelIndex < 0 || COLOR_THEME_SEL_COUNT <= nSelIndex ){
		return COLOR_THEME_BENIZAKURA;
	}
	return COLOR_THEME_NAMES[nSelIndex].nColorTheme;
}

/*!
 * GetColorThemeSelIndex - テーマ番号から、選択肢の位置 (表示順) を返す
 *
 *	@return	選択肢の位置 (不明なテーマ番号の場合は紅桜の位置)
 */
int GetColorThemeSelIndex(
	int		nColorTheme	//!< [in] テーマ番号
)
{
	for( int i = 0; i < COLOR_THEME_SEL_COUNT; ++i ){
		if( COLOR_THEME_NAMES[i].nColorTheme == nColorTheme ){
			return i;
		}
	}
	return GetColorThemeSelIndex( COLOR_THEME_BENIZAKURA );
}

/*!
 * GetColorThemeDispName - テーマの表示名を返す
 *
 *	@return	表示名 (不明なテーマ番号の場合は紅桜の表示名)
 */
const WCHAR* GetColorThemeDispName(
	int		nColorTheme	//!< [in] テーマ番号
)
{
	return COLOR_THEME_NAMES[GetColorThemeSelIndex( nColorTheme )].pszDispName;
}

/*!
 * GetColorThemeKeyName - テーマの ini に保存する名前を返す
 *
 *	@return	ini に保存する名前 (不明なテーマ番号の場合は紅桜の名前)
 */
const wchar_t* GetColorThemeKeyName(
	int		nColorTheme	//!< [in] テーマ番号
)
{
	return COLOR_THEME_NAMES[GetColorThemeSelIndex( nColorTheme )].pszKeyName;
}

/*!
 * GetColorThemeFromKeyName - ini に保存した名前から、テーマ番号を返す
 *
 * 名前は英大文字と小文字を区別しない。
 *
 *	@return	テーマ番号 (不明な名前の場合は COLOR_THEME_BENIZAKURA)
 */
int GetColorThemeFromKeyName(
	const wchar_t*	pszKeyName	//!< [in] ini に保存した名前
)
{
	if( pszKeyName != NULL ){
		for( int i = 0; i < COLOR_THEME_SEL_COUNT; ++i ){
			if( _wcsicmp( COLOR_THEME_NAMES[i].pszKeyName, pszKeyName ) == 0 ){
				return COLOR_THEME_NAMES[i].nColorTheme;
			}
		}
	}
	return COLOR_THEME_BENIZAKURA;
}

/*!
 * IsColorThemePreset - テーマ番号がプリセットのテーマかを返す
 *
 *	@return	true: プリセット / false: カスタム、または不明なテーマ番号
 */
bool IsColorThemePreset(
	int		nColorTheme	//!< [in] テーマ番号
)
{
	return 0 <= nColorTheme && nColorTheme < COLOR_THEME_MAX;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       テーマの適用                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * IsColorThemeDispItem - 項目の「色分け/表示」を、テーマの値だけで決めるかを返す
 *
 * カーソル行の背景色、カーソル行アンダーライン、カーソル位置縦線、TAB 記号、改行記号は、
 * テーマごとに表示の有無が異なるため、テーマを切り替えたときにテーマの値をそのまま全タイプへ適用する。
 * それ以外の項目は、テーマの値とタイプの初期値の両方が「表示」のときに「表示」にする (ApplyColorThemeDisp)。
 * どちらも、適用した後はタイプごとに変更できる。
 *
 *	@return	true: テーマの値だけで決める / false: テーマの値とタイプの初期値で決める
 */
bool IsColorThemeDispItem(
	int		nColorIdx	//!< [in] 色の項目 (EColorIndexType)
)
{
	switch( nColorIdx ){
	case COLORIDX_CARETLINEBG:
	case COLORIDX_UNDERLINE:
	case COLORIDX_CURSORVLINE:
	case COLORIDX_TAB:
	case COLORIDX_EOL:
		return true;
	}
	return false;
}

/*!
 * SetPresetColorInfo - 色設定のすべての値 (名前と「色分け/表示」を含む) を、プリセットの配色にする
 *
 * タイプ別設定の初期化と、プリセットの配色の表示・コピーに使う。
 * カスタム、または不明なテーマ番号の場合は、紅桜の配色にする。
 */
void SetPresetColorInfo(
	ColorInfo*	pColorInfoArr,	//!< [out] 色設定 (COLORIDX_LAST 個)
	int			nColorTheme		//!< [in] テーマ番号
)
{
	if( !IsColorThemePreset( nColorTheme ) ){
		nColorTheme = COLOR_THEME_BENIZAKURA;
	}
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		ColorInfoBase* p = &pColorInfoArr[i];
		*p = ColorInfo_DEFAULT[nColorTheme][i].m_sColorInfoBase;
		wcscpy_s( pColorInfoArr[i].m_szName, _countof(pColorInfoArr[i].m_szName), ColorInfo_DEFAULT[nColorTheme][i].m_pszName );
		pColorInfoArr[i].m_nColorIdx = i;
	}
}

/*!
 * ApplyColorTheme - テーマの配色を、タイプ別設定の色設定に適用する
 *
 * 全項目の文字色・背景色・太字・下線を、テーマの値にする。
 * 「色分け/表示」(タイプごとの設定)、名前、インデックスは変更しない (テーマの「色分け/表示」は ApplyColorThemeDisp で適用する)。
 * カスタムの色がない場合 (NULL)、または不明なテーマ番号の場合は、紅桜の配色を適用する。
 */
void ApplyColorTheme(
	ColorInfo*			pColorInfoArr,			//!< [i/o] タイプ別設定の色設定 (COLORIDX_LAST 個)
	int					nColorTheme,			//!< [in] テーマ番号
	const ColorInfo*	pCustomColorInfoArr		//!< [in] カスタムテーマの色 (COLORIDX_LAST 個)
)
{
	const bool bCustom = ( nColorTheme == COLOR_THEME_CUSTOM && pCustomColorInfoArr != NULL );
	if( !bCustom && !IsColorThemePreset( nColorTheme ) ){
		nColorTheme = COLOR_THEME_BENIZAKURA;
	}
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		const ColorInfoBase& src = bCustom ? pCustomColorInfoArr[i] : ColorInfo_DEFAULT[nColorTheme][i].m_sColorInfoBase;
		ColorInfo& dst = pColorInfoArr[i];
		dst.m_bBoldFont		= src.m_bBoldFont;
		dst.m_bUnderLine	= src.m_bUnderLine;
		dst.m_colTEXT		= src.m_colTEXT;
		dst.m_colBACK		= src.m_colBACK;
	}
}

/*!
 * ApplyColorThemeDisp - テーマの「色分け/表示」を、タイプ別設定の色設定に適用する
 *
 * 全項目の「色分け/表示」を、次の値にする。文字色・背景色・太字・下線、名前、インデックスは変更しない。
 * - IsColorThemeDispItem() の項目 (カーソル行の背景色など 5 項目): テーマの値
 * - それ以外の項目: テーマの値とタイプの初期値の両方が「表示」のときに「表示」
 *   (テーマで「表示しない」にした項目は全タイプで表示しない。「表示」にした項目はタイプの初期値になる)
 * テーマを切り替えたときに使う (ApplyColorTheme と違い、起動時やタイプ別設定の OK では使わない)。
 * カスタムの色がない場合 (NULL)、または不明なテーマ番号の場合は、紅桜の値を適用する。
 */
void ApplyColorThemeDisp(
	ColorInfo*			pColorInfoArr,			//!< [i/o] タイプ別設定の色設定 (COLORIDX_LAST 個)
	int					nColorTheme,			//!< [in] テーマ番号
	const ColorInfo*	pCustomColorInfoArr,	//!< [in] カスタムテーマの色 (COLORIDX_LAST 個)
	const bool*			pbTypeInitDisp			//!< [in] タイプの「色分け/表示」の初期値 (COLORIDX_LAST 個)
)
{
	const bool bCustom = ( nColorTheme == COLOR_THEME_CUSTOM && pCustomColorInfoArr != NULL );
	if( !bCustom && !IsColorThemePreset( nColorTheme ) ){
		nColorTheme = COLOR_THEME_BENIZAKURA;
	}
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		const ColorInfoBase& src = bCustom ? pCustomColorInfoArr[i] : ColorInfo_DEFAULT[nColorTheme][i].m_sColorInfoBase;
		if( IsColorThemeDispItem( i ) ){
			pColorInfoArr[i].m_bDisp = src.m_bDisp;
		}else{
			pColorInfoArr[i].m_bDisp = ( src.m_bDisp && pbTypeInitDisp[i] );
		}
	}
}

/*!
 * IsColorThemeDispChanged - テーマの変更で、テーマの「色分け/表示」を全タイプに適用する必要があるかを返す
 *
 * 変更前と変更後でテーマ番号が異なる場合、またはどちらもカスタムで、カスタムの色の
 * 「色分け/表示」(いずれかの項目) が異なる場合に、適用が必要とする。
 * カスタムの文字色・背景色・太字・下線だけが異なる場合は、適用しない。
 *
 *	@return	true: 適用する必要がある / false: 適用しない
 */
bool IsColorThemeDispChanged(
	int					nOldColorTheme,				//!< [in] 変更前のテーマ番号
	const ColorInfo*	pOldCustomColorInfoArr,		//!< [in] 変更前のカスタムテーマの色 (COLORIDX_LAST 個)
	int					nNewColorTheme,				//!< [in] 変更後のテーマ番号
	const ColorInfo*	pNewCustomColorInfoArr		//!< [in] 変更後のカスタムテーマの色 (COLORIDX_LAST 個)
)
{
	if( nOldColorTheme != nNewColorTheme ){
		return true;
	}
	if( nNewColorTheme != COLOR_THEME_CUSTOM ){
		return false;
	}
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		if( pOldCustomColorInfoArr[i].m_bDisp != pNewCustomColorInfoArr[i].m_bDisp ){
			return true;
		}
	}
	return false;
}

/*!
 * SetCustomColorThemeFromType - タイプ別設定の色を、カスタムテーマの色にしてカスタムを選ぶ
 *
 * テーマの設定がない ini (カラーテーマの導入前の ini) の移行に使う。
 * 基本タイプの色をカスタムの色にすることで、移行前と同じ色で表示する。
 * ただし、IsColorThemeDispItem() 以外の項目の「色分け/表示」は、基本タイプ固有の値 (引用符の文字列を色分けしないなど) が
 * テーマの切り替えで全タイプに広がらないように、共有データの初期値 (紅桜の配色の値) にする。	// 2026.11.21
 */
void SetCustomColorThemeFromType(
	int*				pnColorTheme,			//!< [out] テーマ番号 (COLOR_THEME_CUSTOM にする)
	ColorInfo*			pCustomColorInfoArr,	//!< [out] カスタムテーマの色 (COLORIDX_LAST 個)
	const ColorInfo*	pTypeColorInfoArr		//!< [in] タイプ別設定の色設定 (COLORIDX_LAST 個)
)
{
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		pCustomColorInfoArr[i] = pTypeColorInfoArr[i];
		if( !IsColorThemeDispItem( i ) ){
			pCustomColorInfoArr[i].m_bDisp = ColorInfo_DEFAULT[COLOR_THEME_BENIZAKURA][i].m_sColorInfoBase.m_bDisp;
		}
	}
	*pnColorTheme = COLOR_THEME_CUSTOM;
}
