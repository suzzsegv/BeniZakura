/*!
 *	@file
 *	@brief	カラーテーマ (プリセットの配色とカスタム) の定義と適用
 *
 * 画面表示色のプリセット (紅桜、サクラエディタなど) の配色表と、テーマをタイプ別設定の色設定に
 * 適用する処理。テーマは全タイプ共通で、文字色・背景色・太字・下線と、一部の項目の「色分け/表示」を決める。
 * それ以外の項目の「色分け/表示」はタイプごとの設定として、テーマを適用しても変更しない。
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

#ifndef SAKURA_CCOLORTHEME_H_
#define SAKURA_CCOLORTHEME_H_

#include <Windows.h>
#include "view/colors/EColorIndexType.h"

//! プリセットのカラーテーマ (配色表の並び順)
enum ColorTheme {
	COLOR_THEME_BENIZAKURA = 0,
	COLOR_THEME_SAKURA,
	COLOR_THEME_VS2012DARK,
	COLOR_THEME_MONOKAI,
	COLOR_THEME_MIFES_BLUE,
	COLOR_THEME_HIDEMARU_BLACK,

	COLOR_THEME_MAX
};

//! カスタムテーマのテーマ番号
const int COLOR_THEME_CUSTOM = 100;

//! テーマの選択肢の数 (プリセット + カスタム)
const int COLOR_THEME_SEL_COUNT = COLOR_THEME_MAX + 1;

//! 色設定
struct ColorInfoBase{
	bool		m_bDisp;			//!< 表示
	bool		m_bBoldFont;		//!< 太字
	bool		m_bUnderLine;		//!< 下線
	COLORREF	m_colTEXT;			//!< 文字色
	COLORREF	m_colBACK;			//!< 背景色
};

//! 名前とインデックス付き色設定
struct ColorInfo : public ColorInfoBase{
	int			m_nColorIdx;		//!< インデックス
	WCHAR		m_szName[64];		//!< 名前
};

int GetColorThemeBySelIndex( int nSelIndex );
int GetColorThemeSelIndex( int nColorTheme );
const WCHAR* GetColorThemeDispName( int nColorTheme );
const wchar_t* GetColorThemeKeyName( int nColorTheme );
int GetColorThemeFromKeyName( const wchar_t* pszKeyName );
bool IsColorThemePreset( int nColorTheme );
bool IsColorThemeDispItem( int nColorIdx );
void SetPresetColorInfo( ColorInfo* pColorInfoArr, int nColorTheme );
void ApplyColorTheme( ColorInfo* pColorInfoArr, int nColorTheme, const ColorInfo* pCustomColorInfoArr );
void SetCustomColorThemeFromType( int* pnColorTheme, ColorInfo* pCustomColorInfoArr, const ColorInfo* pTypeColorInfoArr );

#endif /* SAKURA_CCOLORTHEME_H_ */
