/*!
 *	@file
 *	@brief	ダークモード (画面の配色) の配色表と、有効かどうかの判定
 *
 * 画面の配色の設定 (ライト / ダーク / OS の設定に従う) と OS の状態から、ダークモードを使うかを判定する。
 * ダークモードのときに、ウィンドウの部品の描画で使うシステムカラー (COLOR_MENU など) の代わりの色を返す。
 * 編集領域の配色 (カラーテーマ) とは関係しない。
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

#ifndef SAKURA_DARKMODECOLOR_H_
#define SAKURA_DARKMODECOLOR_H_

#include <Windows.h>

//! 画面の配色の設定 (共通設定 → カラー タブ)
enum EUiColorMode {
	UI_COLOR_MODE_LIGHT = 0,	//!< ライト (従来どおり)
	UI_COLOR_MODE_DARK,			//!< ダーク
	UI_COLOR_MODE_SYSTEM,		//!< OS の設定に従う

	UI_COLOR_MODE_MAX
};

bool IsDarkModeEnabledFor( int nUiColorMode, bool bOsDarkMode, bool bHighContrast );	// ダークモードを使うかを判定する
bool GetDarkModeSysColor( int nSysColorIndex, COLORREF* pColor );						// ダークモードでのシステムカラーの代わりの色を返す

#endif /* SAKURA_DARKMODECOLOR_H_ */
