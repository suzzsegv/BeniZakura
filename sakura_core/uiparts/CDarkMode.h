/*!
 *	@file
 *	@brief	ダークモード (画面の配色) の状態の管理と、ウィンドウへの適用
 *
 * 共通設定の「画面の配色」(ライト / ダーク / OS の設定に従う) と、OS のアプリのモード・ハイコントラストから
 * ダークモードを使うかを決め、プロセス内で共有する。ウィンドウの部品の描画では、システムカラーの代わりに
 * GetUiColor() / GetUiColorBrush() を使う (ライトのときはシステムカラーそのものを返す)。
 * タイトルバーやスクロールバーなど、Windows が描く部分は公開 API (DWM、SetWindowTheme) で暗くする。
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

#ifndef SAKURA_CDARKMODE_H_
#define SAKURA_CDARKMODE_H_

#include "uiparts/DarkModeColor.h"

class CDarkMode
{
public:
	static bool Refresh();							// 設定と OS の状態から、ダークモードを使うかを決め直す
	static bool IsEnabled();						// ダークモードを使うか
	static bool IsOsDarkMode();						// OS のアプリのモードがダークか

	static COLORREF GetUiColor( int nSysColorIndex );		// ウィンドウの部品の描画に使う色
	static HBRUSH GetUiColorBrush( int nSysColorIndex );	// ウィンドウの部品の描画に使うブラシ

	static void ApplyTitleBar( HWND hwnd );			// タイトルバーの配色を設定する
	static void ApplyControlTheme( HWND hwnd );		// スクロールバーなどのコントロールのテーマを設定する
	static void ApplyMenuBackground( HMENU hMenu );	// メニューの背景のブラシを設定する
	static void ApplyMenuWindowBorder( HWND hwndMenu );	// 開いているメニューのウィンドウの枠の色を設定する
	static void AttachSizeBox( HWND hwndSizeBox );	// サイズボックスを、ダークモードのときに暗く描くようにする
	static void DrawSizeGrip( HDC hdc, const RECT& rc );	// ダークモードの、サイズ変更のつかむ位置の模様を描く

	// ダイアログ・コントロール (段階 2)	// 2026.11.21
	static void ApplyDialog( HWND hDlg );			// ダイアログのタイトルバーと子コントロールに画面の配色を適用する
	static void ApplyChildControls( HWND hwndParent );	// 子コントロールに画面の配色を適用する
	static void ApplyTooltip( HWND hwndTooltip );	// ツールチップに画面の配色を適用する
	static void ApplyListViewTheme( HWND hwndList );	// リストビューのテーマとダブルバッファを、画面の配色に合わせて設定する	// 2026.11.26
	static INT_PTR OnCtlColor( UINT uMsg, WPARAM wParam, LPARAM lParam );	// WM_CTLCOLOR* の処理 (ブラシを返す)
	static bool OnButtonCustomDraw( LPNMCUSTOMDRAW pnmcd, LRESULT* plResult );	// ボタンの NM_CUSTOMDRAW の処理

	// ダイアログプロシージャの共通処理・プロパティシート (段階 3)	// 2026.11.21
	static bool HandleDialogMessageBefore( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	// ダイアログプロシージャの前に行う処理
	static INT_PTR HandleDialogMessageAfter( UINT uMsg, WPARAM wParam, LPARAM lParam, INT_PTR nRet );	// ダイアログプロシージャの後に行う処理
	static void ApplyPropertySheet( HWND hwndSheet );	// プロパティシートの枠に画面の配色を適用する

private:
	static LRESULT CALLBACK BorderSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static void PaintBorderDark( HWND hwnd );
	static LRESULT CALLBACK UpDownSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static LRESULT CALLBACK BuddyEditSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static int GetBuddyEditBorderState( HWND hwndBuddy );
	static void PaintBuddyEditBorderDark( HWND hwnd, HWND hwndUpDown );
	static bool IsUpDownAttachedToBuddy( HWND hwnd );
	static void PaintUpDownDark( HWND hwnd );
	static LRESULT CALLBACK StaticSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static bool PaintStaticDark( HWND hwnd );
	static LRESULT CALLBACK PropertySheetSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static BOOL CALLBACK ApplyChildControlProc( HWND hwnd, LPARAM lParam );
	static BOOL CALLBACK ApplyListViewThemeProc( HWND hwnd, LPARAM lParam );	// 2026.11.26
	static LRESULT CALLBACK GroupBoxSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static void PaintGroupBoxDark( HWND hwnd );
	static LRESULT CALLBACK TabControlSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static void PaintTabControlDark( HWND hwnd );
	static LRESULT CALLBACK ListViewSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static void DrawPushButtonDark( LPNMCUSTOMDRAW pnmcd );
	static void DrawCheckRadioDark( LPNMCUSTOMDRAW pnmcd, LONG_PTR lStyle );
	static void DeleteBrushes();
	static LRESULT CALLBACK SizeBoxSubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );
	static void PaintSizeBoxDark( HWND hwnd );

	static bool		s_bEnabled;		//!< ダークモードを使うか
	static HBRUSH	s_ahBrush[32];	//!< ダークモードのブラシ (システムカラーの番号ごと)
};

#endif /* SAKURA_CDARKMODE_H_ */
