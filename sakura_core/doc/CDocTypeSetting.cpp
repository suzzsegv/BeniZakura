// 2000.10.08 JEPRO  背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
// 2000.12.09 Jepro  note: color setting (詳細は CshareData.h を参照のこと)
// 2000.09.04 JEPRO  シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
// 2000.10.17 JEPRO  色分け表示するように変更(最初のFALSE→TRUE)
// 2008.03.27 kobake 大整理

#include "StdAfx.h"
#include "CDocTypeSetting.h"
#include "view/Colors/EColorIndexType.h"


//! 色設定(保存用)
struct ColorInfoIni {
	const TCHAR*	m_pszName;			//!< 項目名
	ColorInfoBase	m_sColorInfoBase;		//!< 色設定
};

static ColorInfoIni ColorInfo_DEFAULT[COLOR_THEME_MAX][COLORIDX_LAST] = {
	/* 紅桜 */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x50,0x00 ),
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル行の背景色"),				FALSE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x08,0x08,0x20 ),
		_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB( 0x80,0x80,0xa0 ),	RGB( 0x00,0x00,0x00 ),
		_T("行番号(変更行)"),					TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x40,0x40,0xa0 ),
		_T("TAB記号"),							TRUE,		FALSE,		FALSE,		RGB( 0x40,0x10,0x80 ),	RGB( 0x00,0x00,0x00 ),
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("改行記号"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0x00,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB( 0xc0,0xc0,0xc0 ),	RGB( 0x00,0x00,0x00 ),
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB( 0x00,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("対括弧の強調表示"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0xff,0x40,0x40 ),
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB( 0xd0,0xd0,0x00 ),	RGB( 0xd0,0xd0,0x00 ),
		_T("検索文字列"),						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		_T("検索文字列2"),						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		_T("検索文字列3"),						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		_T("検索文字列4"),						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		_T("検索文字列5"),						TRUE,		FALSE,		FALSE,		RGB( 0x80,0xff,0xff ),	RGB( 0x00,0x00,0xa0 ),
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB( 0x00,0xa0,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB( 0x80,0x80,0x80 ),	RGB( 0x00,0x00,0x00 ),
		_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB( 0x80,0x80,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x98,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード2"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x60,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x40 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x40,0x98 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB( 0x98,0xfb,0x98 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB( 0x98,0x98,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB( 0x40,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ),
		_T("ブックマーク"),						TRUE ,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0xff,0xff,0x00 ),
	},

	/* サクラエディタ */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 239, 239, 239 ),
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 255,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("カーソル行の背景色"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255, 128 ),
		_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB( 128, 128, 255 ),	RGB( 255, 251, 240 ),
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
		_T("行番号(変更行)"),					TRUE,		TRUE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 239, 239, 239 ),
		_T("TAB記号"),							TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB( 255, 255,   0 ),	RGB( 255, 251, 240 ),
		_T("改行記号"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128, 255 ),	RGB( 255, 251, 240 ),
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB( 255,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB( 192, 192, 192 ),	RGB( 255, 251, 240 ),
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB(   0, 255, 255 ),	RGB(   0,   0,   0 ),
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("対括弧の強調表示"),					TRUE,		TRUE,		FALSE,		RGB( 128,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB( 131, 162, 211 ),	RGB( 131, 162, 211 ),
		_T("検索文字列"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 255,   0 ),
		_T("検索文字列2"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 160, 255, 255 ),
		_T("検索文字列3"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 153, 255, 153 ),
		_T("検索文字列4"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 153, 153 ),
		_T("検索文字列5"),						TRUE,		FALSE,		FALSE,		RGB(   0,   0,   0 ),	RGB( 255, 102, 255 ),
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB(   0, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB( 128, 128, 128 ),	RGB( 255, 251, 240 ),
		_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB(  64, 128, 128 ),	RGB( 255, 251, 240 ),
		_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB( 128,   0,  64 ),	RGB( 255, 251, 240 ),
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード2"),					TRUE,		FALSE,		FALSE,		RGB( 128,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB(   0,  64, 128 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB( 235,   0,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB( 255, 128,   0 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 255 ),	RGB( 255, 251, 240 ),
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB(   0,   0, 210 ),	RGB( 162, 208, 255 ),
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB(   0, 111,   0 ),	RGB( 189, 253, 192 ),
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB( 213, 106,   0 ),	RGB( 255, 233, 172 ),
		_T("ブックマーク"),						TRUE ,		FALSE,		FALSE,		RGB( 255, 251, 240 ),   RGB(   0, 128, 192 )
	},

	/* Visual Studio 2012 Dark */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB( 0xdc,0xdc,0xdc ),	RGB( 0x1e,0x1e,0x1e ), // "TXT"
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x33,0x33,0x33 ), // "RUL"
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x00,0x00,0x00 ), // "CAR"
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0x30,0x30 ),	RGB( 0x00,0x00,0x00 ), // "IME"
		_T("カーソル行の背景色"),				TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x00,0x00,0x00 ), // "CBK"
		_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x1e,0x1e,0x1e ), // "UND"
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x00,0x00,0x00 ), // "CVL"
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB( 0x2b,0x91,0xaf ),	RGB( 0x1e,0x1e,0x1e ), // "LNO"
		_T("行番号(変更行)"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "MOD"
		_T("TAB記号"),							FALSE,		FALSE,		FALSE,		RGB( 0x33,0x33,0x33 ),	RGB( 0x1e,0x1e,0x1e ), // "TAB"
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "SPC"
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "ZEN"
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "CTL"
		_T("改行記号"),							FALSE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x1e,0x1e,0x1e ), // "EOL"
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "RAP"
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB( 0x14,0x48,0x50 ),	RGB( 0x1e,0x1e,0x1e ), // "VER"
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "EOF"
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB( 0xb5,0xce,0xa8 ),	RGB( 0x1e,0x1e,0x1e ), // "NUM"
		_T("対括弧の強調表示"),					TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x0e,0x45,0x83 ), // "BRC"
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB( 0x33,0x99,0xff ),	RGB( 0x33,0x99,0xff ), // "SEL"
		_T("検索文字列"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FND"
		_T("検索文字列2"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN2"
		_T("検索文字列3"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN3"
		_T("検索文字列4"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN4"
		_T("検索文字列5"),						TRUE,		FALSE,		FALSE,		RGB( 0xff,0xd8,0x00 ),	RGB( 0x77,0x38,0x00 ), // "FN5"
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB( 0x57,0xa6,0x4a ),	RGB( 0x1e,0x1e,0x1e ), // "CMT"
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB( 0x60,0x60,0x60 ),	RGB( 0x1e,0x1e,0x1e ), // "CM2"
		_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "SQT"
		_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "WQT"
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB( 0x52,0x63,0xda ),	RGB( 0x1e,0x1e,0x1e ), // "URL"
		_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "KW1"
		_T("強調キーワード2"),					TRUE,		FALSE,		FALSE,		RGB( 0x9b,0x9b,0x9b ),	RGB( 0x1e,0x1e,0x1e ), // "KW2"
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB( 0x56,0x9c,0xd6 ),	RGB( 0x1e,0x1e,0x1e ), // "KW3"
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB( 0xb5,0xce,0xa8 ),	RGB( 0x1e,0x1e,0x1e ), // "KW4"
		_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW5"
		_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW6"
		_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW7"
		_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW8"
		_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KW9"
		_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB( 0xd6,0x9d,0x85 ),	RGB( 0x1e,0x1e,0x1e ), // "KWA"
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK1"
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK2"
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK3"
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK4"
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK5"
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK6"
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK7"
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK8"
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RK9"
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB( 0x4e,0xc9,0xb0 ),	RGB( 0x1e,0x1e,0x1e ), // "RKA"
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFA"
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFC"
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x1e,0x1e,0x1e ), // "DFD"
		_T("ブックマーク"),						TRUE ,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x7e,0x65,0x51 ), // "MRK"
	},

	/* Monokai */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB( 0xf8,0xf8,0xf2 ),	RGB( 0x28,0x28,0x28 ),
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x30,0x30,0x30 ),
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB( 0xf8,0xf8,0xf2 ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル行の背景色"),				TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル行アンダーライン"),			FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x00,0x00,0x00 ),
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x00,0x00,0x00 ),
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x30,0x30,0x30 ),
		_T("行番号(変更行)"),					TRUE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x49,0x48,0x3e ),
		_T("TAB記号"),							FALSE,		FALSE,		FALSE,		RGB( 0x3f,0x3d,0x30 ),	RGB( 0x28,0x28,0x28 ),
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		_T("改行記号"),							FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB( 0x8f,0x90,0x8b ),	RGB( 0x28,0x28,0x28 ),
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB( 0x00,0x00,0x00 ),	RGB( 0x66,0xd9,0xef ),
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("対括弧の強調表示"),					TRUE,		FALSE,		TRUE,		RGB( 0xff,0xff,0xff ),	RGB( 0xf9,0x26,0x72 ),
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB( 0x6f,0x6d,0x5e ),	RGB( 0x6f,0x6d,0x5e ),
		_T("検索文字列"),						TRUE,		TRUE,		TRUE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		_T("検索文字列2"),						FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		_T("検索文字列3"),						FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		_T("検索文字列4"),						FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		_T("検索文字列5"),						FALSE,		FALSE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x00,0x00,0x00 ),
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB( 0x75,0x71,0x5e ),	RGB( 0x28,0x28,0x28 ),
		_T("シングルクォーテーション文字列"),	TRUE,		TRUE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		_T("ダブルクォーテーション文字列"),		TRUE,		TRUE,		FALSE,		RGB( 0xe6,0xdb,0x74 ),	RGB( 0x28,0x28,0x28 ),
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード2"),					TRUE,		TRUE,		FALSE,		RGB( 0xf9,0x26,0x72 ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB( 0xae,0x81,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード5"),					FALSE,		FALSE,		FALSE,		RGB( 0xa6,0xe2,0x2e ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード6"),					FALSE,		FALSE,		FALSE,		RGB( 0xfd,0x97,0x1f ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード7"),					FALSE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード8"),					FALSE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード9"),					FALSE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		_T("強調キーワード10"),					FALSE,		FALSE,		FALSE,		RGB( 0xff,0x80,0x00 ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB( 0x66,0xd9,0xef ),	RGB( 0x28,0x28,0x28 ),
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB( 0xff,0xff,0xff ),	RGB( 0x28,0x28,0x28 ),
		_T("ブックマーク"),						TRUE,		FALSE,		FALSE,		RGB( 0x27,0x28,0x22 ),	RGB( 0xe6,0xdb,0x74 ),
	},

	/* MIFES Bule */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0x40, 0x40),	RGB(0x14, 0x14, 0x28),
		_T("カーソル行の背景色"),				FALSE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0x08, 0x08, 0x20),
		_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		_T("行番号(変更行)"),					TRUE,		TRUE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		_T("TAB記号"),							TRUE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB(0x20, 0x20, 0x60),	RGB(0x14, 0x14, 0x28),
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		_T("改行記号"),							TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB(0x78, 0x82, 0xc8),	RGB(0x14, 0x14, 0x28),
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB(0x3c, 0x3c, 0x7d),	RGB(0x14, 0x14, 0x28),
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB(0x96, 0xa0, 0xf0),	RGB(0x14, 0x14, 0x28),
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		_T("対括弧の強調表示"),					TRUE,		TRUE,		FALSE,		RGB(0xf5, 0xf5, 0xf5),	RGB(0x00, 0x00, 0xf5),
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xf5, 0xf5, 0xf5),
		_T("検索文字列"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		_T("検索文字列2"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		_T("検索文字列3"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		_T("検索文字列4"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		_T("検索文字列5"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB(0x46, 0x4b, 0x64),	RGB(0x14, 0x14, 0x28),
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB(0x46, 0x4b, 0x64),	RGB(0x14, 0x14, 0x28),
		_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB(0x80, 0xf5, 0xf5),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード1"),					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード2"),					TRUE,		FALSE,		FALSE,		RGB(0xeb, 0xd7, 0x6e),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x78, 0xa0),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0x78),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x82, 0x64),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB(0xf0, 0x82, 0x64),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB(0xb4, 0x78, 0xdc),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB(0x96, 0xc8, 0x78),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x14, 0x14, 0x28),
		_T("ブックマーク"),						TRUE,		FALSE,		FALSE,		RGB(0x14, 0x14, 0x28),	RGB(0xeb, 0xd7, 0x6e),
	},

	/* 秀丸エディタ Black */
	{
		//	項目名,								表示,		太字,		下線,		文字色,					背景色,
		_T("テキスト"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("ルーラー"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("カーソル"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("カーソル(IME ON)"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0x40, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("カーソル行の背景色"),				FALSE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0x18, 0x18, 0x00),
		_T("カーソル行アンダーライン"),			TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("カーソル位置縦線"),					FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("行番号"),							TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("行番号(変更行)"),					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("TAB記号"),							TRUE,		FALSE,		FALSE,		RGB(0x50, 0x50, 0x50),	RGB(0x00, 0x00, 0x00),
		_T("半角空白")		,					FALSE,		FALSE,		FALSE,		RGB(0xc0, 0xc0, 0xc0),	RGB(0x00, 0x00, 0x00),
		_T("日本語空白"),						TRUE,		FALSE,		FALSE,		RGB(0xc0, 0xc0, 0xc0),	RGB(0x00, 0x00, 0x00),
		_T("コントロールコード"),				TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("改行記号"),							TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("折り返し記号"),						TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("指定桁縦線"),						FALSE,		FALSE,		FALSE,		RGB(0x40, 0x40, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("EOF記号"),							TRUE,		FALSE,		FALSE,		RGB(0x40, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("半角数値"),							TRUE,		FALSE,		FALSE,		RGB(0x00, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("対括弧の強調表示"),					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("選択範囲"),							TRUE,		FALSE,		FALSE,		RGB(0x00, 0x00, 0x00),	RGB(0xff, 0xff, 0xff),
		_T("検索文字列"),						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		_T("検索文字列2"),						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		_T("検索文字列3"),						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		_T("検索文字列4"),						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		_T("検索文字列5"),						TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0xff),
		_T("コメント"),							TRUE,		FALSE,		FALSE,		RGB(0x00, 0xd4, 0x00),	RGB(0x00, 0x00, 0x00),
		_T("C/C++ プリプロセッサコメント"),		TRUE,		FALSE,		FALSE,		RGB(0xd4, 0x00, 0xd4),	RGB(0x00, 0x00, 0x00),
		_T("シングルクォーテーション文字列"),	TRUE,		FALSE,		FALSE,		RGB(0xff, 0x00, 0x00),	RGB(0x00, 0x00, 0x00),
		_T("ダブルクォーテーション文字列"),		TRUE,		FALSE,		FALSE,		RGB(0xff, 0x00, 0x00),	RGB(0x00, 0x00, 0x00),
		_T("URL"),								TRUE,		FALSE,		TRUE,		RGB(0xff, 0x40, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード1"),					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード2"),					TRUE,		TRUE,		FALSE,		RGB(0xff, 0xff, 0x40),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード3"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード4"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード5"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード6"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード7"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード8"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード9"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("強調キーワード10"),					TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード1"),				TRUE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード2"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード3"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード4"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード5"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード6"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード7"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード8"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード9"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("正規表現キーワード10"),				FALSE,		FALSE,		FALSE,		RGB(0xf0, 0xf0, 0xf0),	RGB(0x00, 0x00, 0x00),
		_T("DIFF差分表示(追加)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("DIFF差分表示(変更)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("DIFF差分表示(削除)"),				FALSE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x00, 0x00),
		_T("ブックマーク"),						TRUE,		FALSE,		FALSE,		RGB(0xff, 0xff, 0xff),	RGB(0x00, 0x80, 0x00),
	}
};

/*!
	SetDefaultColorInfo - 画面表示色に関する情報を指定テーマの初期値に設定する

	@param [out] ColorInfo* pColorInfo
	@param [in]  ColorTheme themeNum
	@retval なし
 */
void SetDefaultColorInfo(ColorInfo* pColorInfo, ColorTheme themeNum)
{
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		ColorInfoBase* p = pColorInfo;
		*p = ColorInfo_DEFAULT[themeNum][i].m_sColorInfoBase;
		_tcscpy(pColorInfo->m_szName, ColorInfo_DEFAULT[themeNum][i].m_pszName);
		pColorInfo->m_nColorIdx = i;
		pColorInfo++;
	}
}

/*!
	SetColorTheme - 画面表示色を指定のテーマ配色に設定する

	@param [out] ColorInfo* pColorInfo
	@param [in]  ColorTheme themeNum
	@retval なし
 */
void SetColorTheme(ColorInfo* pColorInfo, ColorTheme themeNum)
{
	pColorInfo[COLORIDX_CARETLINEBG].m_bDisp = ColorInfo_DEFAULT[themeNum][COLORIDX_CARETLINEBG].m_sColorInfoBase.m_bDisp;
	pColorInfo[COLORIDX_UNDERLINE].m_bDisp = ColorInfo_DEFAULT[themeNum][COLORIDX_UNDERLINE].m_sColorInfoBase.m_bDisp;
	pColorInfo[COLORIDX_CURSORVLINE].m_bDisp = ColorInfo_DEFAULT[themeNum][COLORIDX_CURSORVLINE].m_sColorInfoBase.m_bDisp;
	pColorInfo[COLORIDX_TAB].m_bDisp = ColorInfo_DEFAULT[themeNum][COLORIDX_TAB].m_sColorInfoBase.m_bDisp;
	pColorInfo[COLORIDX_EOL].m_bDisp = ColorInfo_DEFAULT[themeNum][COLORIDX_EOL].m_sColorInfoBase.m_bDisp;

	for( int i = 0; i < COLORIDX_LAST; i++ ){
		pColorInfo->m_bBoldFont = ColorInfo_DEFAULT[themeNum][i].m_sColorInfoBase.m_bBoldFont;
		pColorInfo->m_bUnderLine = ColorInfo_DEFAULT[themeNum][i].m_sColorInfoBase.m_bUnderLine;
		pColorInfo->m_colTEXT = ColorInfo_DEFAULT[themeNum][i].m_sColorInfoBase.m_colTEXT;
		pColorInfo->m_colBACK = ColorInfo_DEFAULT[themeNum][i].m_sColorInfoBase.m_colBACK;
		pColorInfo++;
	}
}

