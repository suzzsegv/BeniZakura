/*!
 *	@file
 *	@brief	色設定の一覧 (オーナードローのリストボックス) と色ボタンの描画・操作
 *
 * タイプ別設定の「カラー」タブと、共通設定の「カラー」タブで共通に使う。
 * 一覧の描画、一覧の中のマウス操作、色ボタンの描画、色選択ダイアログの処理は、
 * typeprop/CPropTypesColor.cpp (Copyright (C) 1998-2009, Norio Nakatani ほか) から移動したもの。
 * マウス操作は、呼び出し側が渡す判定関数で変更してよいと判定された場合だけ受け付けるようにした。
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

#include "StdAfx.h"
#include "CColorListUi.h"
#include "doc/CColorTheme.h"
#include "view/colors/CColorStrategy.h"	// g_ColorAttributeArr
#include "uiparts/CGraphics.h"
#include "uiparts/CDarkMode.h"
#include "util/window.h"
#include "sakura_rc.h"

//! 一覧のウィンドウに CColorListUi を結び付けるプロパティ名
static const WCHAR* TSTR_COLORLISTUI = L"CColorListUi";

/*!
 * CColorListUi::CColorListUi - コンストラクタ
 */
CColorListUi::CColorListUi()
: m_hwndList( NULL )
, m_pfnOldProc( NULL )
, m_pCustColors( NULL )
, m_pfnCanEdit( NULL )
, m_pParam( NULL )
{
}

/*!
 * CColorListUi::Attach - 一覧のリストボックスをサブクラス化して、マウス操作を受け付けるようにする
 *
 * ダイアログの WM_INITDIALOG で呼ぶ。一覧のウィンドウが破棄されると、サブクラス化を解除する。
 */
void CColorListUi::Attach(
	HWND		hwndList,		//!< [in] 一覧のリストボックス
	DWORD*		pCustColors,	//!< [in] 色選択ダイアログのカスタムパレット (16 色。ダイアログの間は有効なもの)
	CanEditFunc	pfnCanEdit,		//!< [in] 変更してよいかの判定関数
	void*		pParam			//!< [in] 判定関数に渡す値
)
{
	m_hwndList = hwndList;
	m_pCustColors = pCustColors;
	m_pfnCanEdit = pfnCanEdit;
	m_pParam = pParam;
	::SetProp( hwndList, TSTR_COLORLISTUI, (HANDLE)this );
	m_pfnOldProc = (WNDPROC)::SetWindowLongPtr( hwndList, GWLP_WNDPROC, (LONG_PTR)SubclassProc );
}

/*!
 * CColorListUi::CanEdit - 色の項目の値を変更してよいかを返す
 *
 *	@return	true: 変更してよい / false: 変更できない
 */
bool CColorListUi::CanEdit(
	int				nColorIdx,	//!< [in] 色の項目 (EColorIndexType)
	EColorListEdit	eEdit		//!< [in] 変更する値
) const
{
	if( m_pfnCanEdit == NULL ){
		return true;
	}
	return m_pfnCanEdit( m_pParam, nColorIdx, eEdit );
}

/*!
 * CColorListUi::SubclassProc - サブクラス化した一覧のウィンドウプロシージャ
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CColorListUi::SubclassProc(
	HWND	hwnd,		//!< [in] 一覧のリストボックス
	UINT	uMsg,		//!< [in] メッセージ
	WPARAM	wParam,		//!< [in] パラメータ
	LPARAM	lParam		//!< [in] パラメータ
)
{
	CColorListUi* pThis = (CColorListUi*)::GetProp( hwnd, TSTR_COLORLISTUI );
	if( pThis == NULL ){
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
	WNDPROC pfnOldProc = pThis->m_pfnOldProc;
	switch( uMsg ){
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONUP:
		pThis->OnMouse( hwnd, uMsg, lParam );
		break;
	case WM_DESTROY:
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)pfnOldProc );
		::RemoveProp( hwnd, TSTR_COLORLISTUI );
		break;
	}
	return ::CallWindowProc( pfnOldProc, hwnd, uMsg, wParam, lParam );
}

/*!
 * CColorListUi::OnMouse - 一覧の中のマウス操作で、色設定を変更する
 *
 * 左端のチェックのクリックで色分け/表示、右クリックで下線、ダブルクリックで太字、
 * 右端の色見本のクリックで文字色・背景色を変更する。変更できない項目・値の操作は無視する。
 */
void CColorListUi::OnMouse(
	HWND	hwnd,		//!< [in] 一覧のリストボックス
	UINT	uMsg,		//!< [in] メッセージ (WM_RBUTTONDOWN / WM_LBUTTONDBLCLK / WM_LBUTTONUP)
	LPARAM	lParam		//!< [in] マウスの位置
)
{
	const int xPos = (short)LOWORD( lParam );
	POINT poMouse = { xPos, (short)HIWORD( lParam ) };
	RECT rcItem = {0,0,0,0};
	int nIndex = -1;
	const int nItemNum = List_GetCount( hwnd );
	for( int i = 0; i < nItemNum; ++i ){
		List_GetItemRect( hwnd, i, &rcItem );
		if( ::PtInRect( &rcItem, poMouse ) ){
			nIndex = i;
			break;
		}
	}
	if( -1 == nIndex ){
		return;
	}
	ColorInfo* pColorInfo = (ColorInfo*)List_GetItemData( hwnd, nIndex );
	if( pColorInfo == NULL ){
		return;
	}
	const int nColorIdx = pColorInfo->m_nColorIdx;
	const unsigned int fAttribute = g_ColorAttributeArr[nColorIdx].fAttribute;
	HWND hwndDlg = ::GetParent( hwnd );

	switch( uMsg ){
	case WM_RBUTTONDOWN:
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji 有効範囲の制限追加
			List_SetCurSel( hwnd, nIndex );
			::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_COLORS, LBN_SELCHANGE ), (LPARAM)hwnd );
			/* 下線 */
			if( 0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE) && CanEdit( nColorIdx, COLORLIST_EDIT_UNDERLINE ) ){
				pColorInfo->m_bUnderLine = !pColorInfo->m_bUnderLine;
				::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE, pColorInfo->m_bUnderLine ? BST_CHECKED : BST_UNCHECKED );
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if( 18 <= xPos && xPos <= rcItem.right - 29 ){	// 2009.02.22 ryoji 有効範囲の制限追加
			/* 太字で表示 */
			if( 0 == (fAttribute & COLOR_ATTRIB_NO_BOLD) && CanEdit( nColorIdx, COLORLIST_EDIT_BOLD ) ){
				pColorInfo->m_bBoldFont = !pColorInfo->m_bBoldFont;
				::CheckDlgButton( hwndDlg, IDC_CHECK_BOLD, pColorInfo->m_bBoldFont ? BST_CHECKED : BST_UNCHECKED );
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}
		break;

	case WM_LBUTTONUP:
		/* 色分け/表示 する */
		if( 2 <= xPos && xPos <= 16 ){
			if( 0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP) && CanEdit( nColorIdx, COLORLIST_EDIT_DISP ) ){
				pColorInfo->m_bDisp = !pColorInfo->m_bDisp;
				if( List_GetCurSel( hwnd ) == nIndex ){
					::CheckDlgButton( hwndDlg, IDC_CHECK_DISP, pColorInfo->m_bDisp ? BST_CHECKED : BST_UNCHECKED );
				}
				::InvalidateRect( hwnd, &rcItem, TRUE );
			}
		}else
		/* 前景色見本 矩形 */
		if( rcItem.right - 27 <= xPos && xPos <= rcItem.right - 27 + 12 ){
			if( 0 == (fAttribute & COLOR_ATTRIB_NO_TEXT) && CanEdit( nColorIdx, COLORLIST_EDIT_TEXT ) ){
				/* 色選択ダイアログ */
				if( SelectColor( hwnd, &pColorInfo->m_colTEXT, m_pCustColors ) ){
					::InvalidateRect( hwnd, &rcItem, TRUE );
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
				}
			}
		}else
		/* 背景色見本 矩形 */
		if( rcItem.right - 13 <= xPos && xPos <= rcItem.right - 13 + 12 ){
			if( 0 == (fAttribute & COLOR_ATTRIB_NO_BACK) && CanEdit( nColorIdx, COLORLIST_EDIT_BACK ) ){
				/* 色選択ダイアログ */
				if( SelectColor( hwnd, &pColorInfo->m_colBACK, m_pCustColors ) ){
					::InvalidateRect( hwnd, &rcItem, TRUE );
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
				}
			}
		}
		break;
	}
}

/*!
 * CColorListUi::DrawItem - 色設定の一覧の項目を描画する (WM_DRAWITEM)
 *
 * 項目名 (太字・下線を反映)、左端の「色分け/表示」のチェック、右端の文字色・背景色の見本を描く。
 * 「色分け/表示」のチェックは、判定関数が COLORLIST_SHOW_DISP を許可した項目だけに描く。
 */
void CColorListUi::DrawItem(
	DRAWITEMSTRUCT*	pDis	//!< [in] 描画情報
) const
{
	COLORREF	cRim = (COLORREF)CDarkMode::GetUiColor( COLOR_3DSHADOW );

	if( pDis == NULL || pDis->itemData == 0 ) return;

	//描画対象
	CGraphics gr(pDis->hDC);

	RECT rc1 = pDis->rcItem;

	/* アイテムデータの取得 */
	const ColorInfo* pColorInfo = (const ColorInfo*)pDis->itemData;

	/* アイテム矩形塗りつぶし */
	gr.SetBrushColor( CDarkMode::GetUiColor( COLOR_WINDOW ) );
	gr.FillMyRect( pDis->rcItem );

	/* アイテムが選択されている */
	if( pDis->itemState & ODS_SELECTED ){
		gr.SetBrushColor( CDarkMode::GetUiColor( COLOR_HIGHLIGHT ) );
		gr.SetTextForeColor( CDarkMode::GetUiColor( COLOR_HIGHLIGHTTEXT ) );
	}else{
		gr.SetBrushColor( CDarkMode::GetUiColor( COLOR_WINDOW ) );
		gr.SetTextForeColor( CDarkMode::GetUiColor( COLOR_WINDOWTEXT ) );
	}

	rc1.left+= (2 + 16);
	rc1.top += 2;
	rc1.right -= ( 2 + 27 );
	rc1.bottom -= 2;
	/* 選択ハイライト矩形 */
	gr.FillMyRect(rc1);
	/* テキスト */
	::SetBkMode( gr, TRANSPARENT );
	::TextOut( gr, rc1.left, rc1.top, pColorInfo->m_szName, wcslen( pColorInfo->m_szName ) );
	if( pColorInfo->m_bBoldFont ){	/* 太字か */
		::TextOut( gr, rc1.left + 1, rc1.top, pColorInfo->m_szName, wcslen( pColorInfo->m_szName ) );
	}
	if( pColorInfo->m_bUnderLine ){	/* 下線か */
		SIZE	sz;
		::GetTextExtentPoint32( gr, pColorInfo->m_szName, wcslen( pColorInfo->m_szName ), &sz );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 2, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 2 );
		::MoveToEx( gr, rc1.left,		rc1.bottom - 1, NULL );
		::LineTo( gr, rc1.left + sz.cx,	rc1.bottom - 1 );
	}

	/* アイテムにフォーカスがある */	// 2006.05.01 ryoji 描画条件の不正を修正
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( gr, &pDis->rcItem );
	}

	/* 「色分け/表示する」のチェック */
	rc1 = pDis->rcItem;
	rc1.left += 2;
	rc1.top += 3;
	rc1.right = rc1.left + 12;
	rc1.bottom = rc1.top + 12;
	if( pColorInfo->m_bDisp && CanEdit( pColorInfo->m_nColorIdx, COLORLIST_SHOW_DISP ) ){	/* 色分け/表示する */
		// 2006.04.26 ryoji テキスト色を使う（「ハイコントラスト黒」のような設定でも見えるように）
		gr.SetPen( CDarkMode::GetUiColor( COLOR_WINDOWTEXT ) );

		for( int i = 0; i < 3; i++ ){
			::MoveToEx( gr,	rc1.left + 2, rc1.top + 6, NULL );
			::LineTo( gr,	rc1.left + 5, rc1.bottom - 3 );
			::LineTo( gr,	rc1.right - 2, rc1.top + 4 );
			rc1.top -= 1;
			rc1.bottom -= 1;
		}
	}

	if ( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
	{
		/* 背景色 見本矩形 */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 13;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;

		gr.SetBrushColor( pColorInfo->m_colBACK );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}

	if( 0 == (g_ColorAttributeArr[pColorInfo->m_nColorIdx].fAttribute & COLOR_ATTRIB_NO_TEXT) )
	{
		/* 前景色 見本矩形 */
		rc1 = pDis->rcItem;
		rc1.left = rc1.right - 27;
		rc1.top += 2;
		rc1.right = rc1.left + 12;
		rc1.bottom -= 2;
		gr.SetBrushColor( pColorInfo->m_colTEXT );
		gr.SetPen( cRim );
		::RoundRect( pDis->hDC, rc1.left, rc1.top, rc1.right, rc1.bottom , 3, 3 );
	}
}

/*!
 * CColorListUi::DrawColorButton - 色ボタン (文字色・背景色のボタン) を描画する (WM_DRAWITEM)
 */
void CColorListUi::DrawColorButton(
	DRAWITEMSTRUCT*	pDis,		//!< [in] 描画情報
	COLORREF		cColor		//!< [in] ボタンに表示する色
)
{
	COLORREF	cBtnHiLight		= (COLORREF)CDarkMode::GetUiColor(COLOR_3DHILIGHT);
	COLORREF	cBtnShadow		= (COLORREF)CDarkMode::GetUiColor(COLOR_3DSHADOW);
	COLORREF	cBtnDkShadow	= (COLORREF)CDarkMode::GetUiColor(COLOR_3DDKSHADOW);
	COLORREF	cBtnFace		= (COLORREF)CDarkMode::GetUiColor(COLOR_3DFACE);
	RECT		rc;
	RECT		rcFocus;

	//描画対象
	CGraphics gr(pDis->hDC);

	/* ボタンの表面の色で塗りつぶす */
	gr.SetBrushColor( cBtnFace );
	gr.FillMyRect( pDis->rcItem );

	/* 枠の描画 */
	rc = pDis->rcItem;
	rc.top += 4;
	rc.left += 4;
	rc.right -= 4;
	rc.bottom -= 4;
	rcFocus = rc;

	if( pDis->itemState & ODS_SELECTED ){

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 3, NULL );
		::LineTo( gr, 1, 1 );
		::LineTo( gr, pDis->rcItem.right - 2, 1 );

		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );

		rc.top += 1;
		rc.left += 1;
		rc.right += 1;
		rc.bottom += 1;

		rcFocus.top += 1;
		rcFocus.left += 1;
		rcFocus.right += 1;
		rcFocus.bottom += 1;

	}
	else{
		gr.SetPen(cBtnHiLight);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, 0, 0 );
		::LineTo( gr, pDis->rcItem.right - 1, 0 );

		gr.SetPen(cBtnShadow);
		::MoveToEx( gr, 1, pDis->rcItem.bottom - 2, NULL );
		::LineTo( gr, pDis->rcItem.right - 2, pDis->rcItem.bottom - 2 );
		::LineTo( gr, pDis->rcItem.right - 2, 0 );

		gr.SetPen(cBtnDkShadow);
		::MoveToEx( gr, 0, pDis->rcItem.bottom - 1, NULL );
		::LineTo( gr, pDis->rcItem.right - 1, pDis->rcItem.bottom - 1 );
		::LineTo( gr, pDis->rcItem.right - 1, -1 );
	}

	if((pDis->itemState & ODS_DISABLED)==0){
		/* 指定色で塗りつぶす */
		gr.SetBrushColor(cColor);
		gr.SetPen(cBtnShadow);
		::RoundRect( gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );
	}

	/* フォーカスの長方形 */
	if( pDis->itemState & ODS_FOCUS ){
		rcFocus.top -= 3;
		rcFocus.left -= 3;
		rcFocus.right += 2;
		rcFocus.bottom += 2;
		::DrawFocusRect( gr, &rcFocus );
	}
}

/*!
 * CColorListUi::SelectColor - 色選択ダイアログを表示する
 *
 *	@return	TRUE: 色を選んだ (pColor を変更) / FALSE: キャンセルした
 */
BOOL CColorListUi::SelectColor(
	HWND		hwndParent,		//!< [in] 親ウィンドウ
	COLORREF*	pColor,			//!< [i/o] 色
	DWORD*		pCustColors		//!< [i/o] カスタムパレット (16 色)
)
{
	CHOOSECOLOR		cc;
	cc.lStructSize = sizeof_raw( cc );
	cc.hwndOwner = hwndParent;
	cc.hInstance = NULL;
	cc.rgbResult = *pColor;
	cc.lpCustColors = pCustColors;
	cc.Flags = /*CC_PREVENTFULLOPEN |*/ CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( !::ChooseColor( &cc ) ){
		return FALSE;
	}
	*pColor = cc.rgbResult;
	return TRUE;
}
