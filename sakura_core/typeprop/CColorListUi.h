/*!
 *	@file
 *	@brief	色設定の一覧 (オーナードローのリストボックス) と色ボタンの描画・操作
 *
 * タイプ別設定の「カラー」タブと、共通設定の「カラー」タブで共通に使う。
 * 一覧の各項目のアイテムデータは ColorInfo へのポインタ。一覧の中のマウス操作 (色分け/表示、太字、下線、
 * 文字色・背景色の変更) は、呼び出し側が渡す判定関数で変更してよいと判定された場合だけ受け付ける。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/18 新規作成 (CPropTypesColor から分離)
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

#ifndef SAKURA_CCOLORLISTUI_H_
#define SAKURA_CCOLORLISTUI_H_

//! 色設定の一覧で変更する値 (変更してよいかの判定に使う)
enum EColorListEdit {
	COLORLIST_EDIT_DISP,		//!< 色分け/表示
	COLORLIST_EDIT_BOLD,		//!< 太字
	COLORLIST_EDIT_UNDERLINE,	//!< 下線
	COLORLIST_EDIT_TEXT,		//!< 文字色
	COLORLIST_EDIT_BACK,		//!< 背景色
	COLORLIST_SHOW_DISP,		//!< 一覧に「色分け/表示」のチェックを描くか (変更ではなく表示の判定)
};

/*!
 * 色設定の一覧と色ボタンの描画・操作
 */
class CColorListUi
{
public:
	//! 変更してよいかの判定関数 (true: 変更してよい)
	typedef bool (*CanEditFunc)( void* pParam, int nColorIdx, EColorListEdit eEdit );

	CColorListUi();

	void Attach( HWND hwndList, DWORD* pCustColors, CanEditFunc pfnCanEdit, void* pParam );
	bool CanEdit( int nColorIdx, EColorListEdit eEdit ) const;
	void DrawItem( DRAWITEMSTRUCT* pDis ) const;

	static void DrawColorButton( DRAWITEMSTRUCT* pDis, COLORREF cColor );
	static BOOL SelectColor( HWND hwndParent, COLORREF* pColor, DWORD* pCustColors );

private:
	static LRESULT CALLBACK SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void OnMouse( HWND hwnd, UINT uMsg, LPARAM lParam );

	HWND		m_hwndList;		//!< 一覧のリストボックス
	WNDPROC		m_pfnOldProc;	//!< サブクラス化する前のウィンドウプロシージャ
	DWORD*		m_pCustColors;	//!< 色選択ダイアログのカスタムパレット (16 色)
	CanEditFunc	m_pfnCanEdit;	//!< 変更してよいかの判定関数
	void*		m_pParam;		//!< 判定関数に渡す値
};

#endif /* SAKURA_CCOLORLISTUI_H_ */
