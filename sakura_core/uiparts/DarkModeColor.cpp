/*!
 *	@file
 *	@brief	ダークモード (画面の配色) の配色表と、有効かどうかの判定
 *
 * 画面の配色の設定 (ライト / ダーク / OS の設定に従う) と OS の状態から、ダークモードを使うかを判定する。
 * ダークモードのときに、ウィンドウの部品の描画で使うシステムカラー (COLOR_MENU など) の代わりの色を返す。
 * 配色は Windows 11 のダークモードの標準的な値 (背景 #202020、部品 #2B2B2B、文字 #FFFFFF など) を元にしている。
 *
 * プリコンパイル済みヘッダー (StdAfx.h) は使用しない (単体テストからも単独でビルドするため)。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/20 新規作成
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

#include "uiparts/DarkModeColor.h"

/*!
 * IsDarkModeEnabledFor - 画面の配色の設定と OS の状態から、ダークモードを使うかを判定する
 *
 * ハイコントラストのときは、OS のハイコントラストの配色を優先してダークモードを使わない。
 * 「OS の設定に従う」は、OS のアプリのモードがダークのときだけダークモードを使う。
 * 不明な設定値はライトとして扱う。
 *
 *	@return	true: ダークモードを使う / false: 使わない (ライト)
 */
bool IsDarkModeEnabledFor(
	int		nUiColorMode,	//!< [in] 画面の配色の設定 (EUiColorMode)
	bool	bOsDarkMode,	//!< [in] OS のアプリのモードがダークか
	bool	bHighContrast	//!< [in] ハイコントラストが有効か
)
{
	if( bHighContrast ){
		return false;
	}
	switch( nUiColorMode ){
	case UI_COLOR_MODE_DARK:
		return true;
	case UI_COLOR_MODE_SYSTEM:
		return bOsDarkMode;
	default:
		return false;
	}
}

/*!
 * GetDarkModeSysColor - ダークモードでの、システムカラーの代わりの色を返す
 *
 * ウィンドウの部品の描画で使うシステムカラーの番号 (COLOR_MENU など) に対して、ダークモードの色を返す。
 * 配色表にない番号は false を返す (呼び出し側でシステムカラーをそのまま使う)。
 *
 *	@return	true: 色を返却 / false: 配色表にない番号
 */
bool GetDarkModeSysColor(
	int			nSysColorIndex,	//!< [in] システムカラーの番号 (COLOR_*)
	COLORREF*	pColor			//!< [out] ダークモードの色
)
{
	COLORREF col;
	switch( nSysColorIndex ){
	// 背景
	case COLOR_WINDOW:
	case COLOR_MENUBAR:
	case COLOR_APPWORKSPACE:
		col = RGB( 0x20, 0x20, 0x20 );
		break;
	case COLOR_MENU:
	case COLOR_3DFACE:			// = COLOR_BTNFACE
	case COLOR_INFOBK:
		col = RGB( 0x2B, 0x2B, 0x2B );
		break;
	// 文字
	case COLOR_WINDOWTEXT:
	case COLOR_MENUTEXT:
	case COLOR_BTNTEXT:			// (COLOR_BTNTEXT と同じ値の番号はない)
	case COLOR_INFOTEXT:
	case COLOR_CAPTIONTEXT:
	case COLOR_HIGHLIGHTTEXT:
		col = RGB( 0xFF, 0xFF, 0xFF );
		break;
	case COLOR_GRAYTEXT:
		col = RGB( 0x8A, 0x8A, 0x8A );
		break;
	// 立体の枠・影
	case COLOR_3DHIGHLIGHT:		// = COLOR_BTNHIGHLIGHT
		col = RGB( 0x46, 0x46, 0x46 );
		break;
	case COLOR_3DLIGHT:
		col = RGB( 0x3C, 0x3C, 0x3C );
		break;
	case COLOR_3DSHADOW:		// = COLOR_BTNSHADOW
		col = RGB( 0x50, 0x50, 0x50 );
		break;
	case COLOR_3DDKSHADOW:
		col = RGB( 0x14, 0x14, 0x14 );
		break;
	case COLOR_WINDOWFRAME:
	case COLOR_ACTIVEBORDER:
	case COLOR_INACTIVEBORDER:
		col = RGB( 0x3F, 0x3F, 0x3F );
		break;
	// 選択・強調
	case COLOR_HIGHLIGHT:
		col = RGB( 0x00, 0x5A, 0x9E );
		break;
	case COLOR_MENUHILIGHT:
		col = RGB( 0x41, 0x41, 0x41 );
		break;
	case COLOR_HOTLIGHT:
		col = RGB( 0x99, 0xCC, 0xFF );
		break;
	default:
		return false;
	}
	*pColor = col;
	return true;
}
