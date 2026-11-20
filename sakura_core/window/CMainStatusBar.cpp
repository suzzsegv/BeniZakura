#include "StdAfx.h"
#include "CMainStatusBar.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"
#include "uiparts/CDarkMode.h"

CMainStatusBar::CMainStatusBar(CEditWnd* pOwner)
: m_pOwner(pOwner)
, m_hwndStatusBar( NULL )
, m_hwndProgressBar( NULL )
, m_nRecMarkState( -1 )
{
}


//	キーワード：ステータスバー順序
/* ステータスバー作成 */
void CMainStatusBar::CreateStatusBar()
{
	if( m_hwndStatusBar )return;

	m_nRecMarkState = -1;	// 「REC」の欄は未設定	// 2026.11.20

	/* ステータスバー */
	m_hwndStatusBar = ::CreateStatusWindow(
		WS_CHILD/* | WS_VISIBLE*/ | WS_EX_RIGHT | SBARS_SIZEGRIP,	// 2007.03.08 ryoji WS_VISIBLE 除去
		L"",
		m_pOwner->GetHwnd(),
		IDW_STATUSBAR
	);
	// ダークモードのときに自分で描画するためにサブクラス化する	// 2026.11.20
	if( NULL != m_hwndStatusBar ){
		::SetWindowSubclass( m_hwndStatusBar, StatusBarSubclassProc, 0, (DWORD_PTR)this );
	}

	/* プログレスバー */
	m_hwndProgressBar = ::CreateWindowEx(
		WS_EX_TOOLWINDOW,
		PROGRESS_CLASS,
		NULL,
		WS_CHILD /*|  WS_VISIBLE*/,
		3,
		5,
		150,
		13,
		m_hwndStatusBar,
		NULL,
		CEditApp::getInstance()->GetAppInstance(),
		0
	);

	if( NULL != m_pOwner->m_CFuncKeyWnd.GetHwnd() ){
		m_pOwner->m_CFuncKeyWnd.SizeBox_ONOFF( FALSE );
	}

	//スプリッターの、サイズボックスの位置を変更
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1);
}


/* ステータスバー破棄 */
void CMainStatusBar::DestroyStatusBar()
{
	if( NULL != m_hwndProgressBar ){
		::DestroyWindow( m_hwndProgressBar );
		m_hwndProgressBar = NULL;
	}
	::DestroyWindow( m_hwndStatusBar );
	m_hwndStatusBar = NULL;

	if( NULL != m_pOwner->m_CFuncKeyWnd.GetHwnd() ){
		bool bSizeBox;
		if( GetDllShareData().m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
			/* サイズボックスの表示／非表示切り替え */
			bSizeBox = false;
		}
		else{
			bSizeBox = true;
			/* ステータスパーを表示している場合はサイズボックスを表示しない */
			if( NULL != m_hwndStatusBar ){
				bSizeBox = false;
			}
		}
		m_pOwner->m_CFuncKeyWnd.SizeBox_ONOFF( bSizeBox );
	}
	//スプリッターの、サイズボックスの位置を変更
	m_pOwner->m_cSplitterWnd.DoSplit( -1, -1 );
}


/*!
	@brief メッセージの表示
	
	指定されたメッセージをステータスバーに表示する．
	メニューバー右端に入らないものや，桁位置表示を隠したくないものに使う
	
	呼び出し前にSendStatusMessage2IsEffective()で処理の有無を
	確認することで無駄な処理を省くことが出来る．

	@param msg [in] 表示するメッセージ
	@date 2005.07.09 genta 新規作成
	
	@sa SendStatusMessage2IsEffective
*/
void CMainStatusBar::SendStatusMessage2( const WCHAR* msg )
{
	if( NULL != m_hwndStatusBar ){
		// ステータスバーへ
		SetStatusText( 0, SBT_NOBORDERS, msg );
	}
}


void CMainStatusBar::SetStatusText(int nIndex, int nOption, const WCHAR* pszText)
{
	if( NULL == m_hwndStatusBar ){
		return;
	}
	// 現在の欄の内容と同じなら設定しない (SB_SETTEXT は同じ内容でも欄を再描画するため)	// 2026.11.20
	if( IsSameStatusText( nIndex, nOption, pszText ) ){
		return;
	}
	StatusBar_SetText( m_hwndStatusBar, nIndex | nOption, pszText );
}


/*!
 * CMainStatusBar::StatusBarSubclassProc - ステータスバーのサブクラスプロシージャ
 *
 * ダークモードのときは、背景の消去を省き、WM_PAINT で全体を自分で描画する (PaintDark)。
 * ライトのときは、ステータスバー本来の処理にそのまま渡す。
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CMainStatusBar::StatusBarSubclassProc(
	HWND		hwnd,		//!< [in] ステータスバー
	UINT		uMsg,		//!< [in] メッセージ
	WPARAM		wParam,		//!< [in] パラメータ
	LPARAM		lParam,		//!< [in] パラメータ
	UINT_PTR	uIdSubclass,	//!< [in] サブクラスの ID
	DWORD_PTR	dwRefData	//!< [in] CMainStatusBar のポインタ
)
{
	switch( uMsg ){
	case WM_ERASEBKGND:
		if( CDarkMode::IsEnabled() ){
			return 1;
		}
		break;
	case WM_PAINT:
		if( CDarkMode::IsEnabled() ){
			((CMainStatusBar*)dwRefData)->PaintDark( hwnd );
			return 0;
		}
		break;
	case WM_NCDESTROY:
		::RemoveWindowSubclass( hwnd, StatusBarSubclassProc, uIdSubclass );
		break;
	}
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

/*!
 * CMainStatusBar::PaintDark - ダークモードのとき、ステータスバー全体を描画する
 *
 * ステータスバーはテーマで明るく描かれるため、ダークモードでは背景を暗い色で塗り、各欄の文字と区切り線を描く。
 * 自前描画 (SBT_OWNERDRAW) の欄は、通常と同じく親ウィンドウに WM_DRAWITEM を送って描かせる。
 */
void CMainStatusBar::PaintDark(
	HWND	hwnd		//!< [in] ステータスバー
)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint( hwnd, &ps );
	RECT rcClient;
	::GetClientRect( hwnd, &rcClient );
	::FillRect( hdc, &rcClient, CDarkMode::GetUiColorBrush( COLOR_3DFACE ) );

	HFONT hFontOld = (HFONT)::SelectObject( hdc, (HFONT)::SendMessage( hwnd, WM_GETFONT, 0, 0 ) );
	int nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	COLORREF colTextOld = ::SetTextColor( hdc, CDarkMode::GetUiColor( COLOR_BTNTEXT ) );
	HPEN hPen = ::CreatePen( PS_SOLID, 1, CDarkMode::GetUiColor( COLOR_3DSHADOW ) );
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );
	const int nMargin = DpiScaleX( 4 );

	int nParts = (int)::SendMessage( hwnd, SB_GETPARTS, 0, 0 );
	for( int i = 0; i < nParts; i++ ){
		RECT rcPart, rcDummy;
		if( !::SendMessage( hwnd, SB_GETRECT, (WPARAM)i, (LPARAM)&rcPart ) || !::IntersectRect( &rcDummy, &rcPart, &ps.rcPaint ) ){
			continue;
		}
		LRESULT lLen = ::SendMessage( hwnd, SB_GETTEXTLENGTH, (WPARAM)i, 0 );
		::SetTextColor( hdc, CDarkMode::GetUiColor( COLOR_BTNTEXT ) );	// 自前描画の欄で変更されることがあるため、欄ごとに設定する
		WORD wType = HIWORD( lLen );
		if( wType & SBT_OWNERDRAW ){
			DRAWITEMSTRUCT dis;
			dis.CtlType = 0;
			dis.CtlID = ::GetDlgCtrlID( hwnd );
			dis.itemID = i;
			dis.itemAction = ODA_DRAWENTIRE;
			dis.itemState = 0;
			dis.hwndItem = hwnd;
			dis.hDC = hdc;
			dis.rcItem = rcPart;
			dis.itemData = (ULONG_PTR)::SendMessage( hwnd, SB_GETTEXT, (WPARAM)i, 0 );
			::SendMessage( ::GetParent( hwnd ), WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis );
		}else{
			WCHAR szText[1024];
			if( LOWORD( lLen ) < _countof( szText ) ){
				szText[0] = L'\0';
				::SendMessage( hwnd, SB_GETTEXT, (WPARAM)i, (LPARAM)szText );
				RECT rcText = rcPart;
				rcText.left += nMargin;
				::DrawText( hdc, szText, -1, &rcText, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS );
			}
		}
		// 区切り線 (枠なしの欄と最後の欄には描かない)
		if( 0 == ( wType & SBT_NOBORDERS ) && i < nParts - 1 ){
			::MoveToEx( hdc, rcPart.right, rcPart.top + 2, NULL );
			::LineTo( hdc, rcPart.right, rcPart.bottom - 2 );
		}
	}

	// サイズ変更のつかむ位置 (SBARS_SIZEGRIP)。ウィンドウが最大化されているときは、ステータスバー本来の描画と同じく描かない
	if( ( ::GetWindowLongPtr( hwnd, GWL_STYLE ) & SBARS_SIZEGRIP ) && !::IsZoomed( ::GetAncestor( hwnd, GA_ROOT ) ) ){
		RECT rcGrip = rcClient;
		rcGrip.left = rcGrip.right - ::GetSystemMetrics( SM_CXVSCROLL );
		CDarkMode::DrawSizeGrip( hdc, rcGrip );
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
	::SetTextColor( hdc, colTextOld );
	::SetBkMode( hdc, nBkModeOld );
	::SelectObject( hdc, hFontOld );
	::EndPaint( hwnd, &ps );
}

/*!
 * CMainStatusBar::IsSameStatusText - ステータスバーの欄の現在の内容が、指定した内容と同じかを調べる
 *
 * SB_GETTEXTLENGTH で欄の描画方法 (SBT_*) と文字列の長さを、SB_GETTEXT で文字列を取得して比較する。
 * 自前描画 (SBT_OWNERDRAW) の欄は文字列を持たないので、常に「異なる」とする。
 *
 *	@return	true: 同じ / false: 異なる (または比較できない)
 */
bool CMainStatusBar::IsSameStatusText(
	int				nIndex,		//!< [in] 欄の番号
	int				nOption,	//!< [in] 描画方法 (SBT_NOBORDERS など)
	const WCHAR*	pszText		//!< [in] 設定しようとしている文字列
) const
{
	if( NULL == pszText || 0 != (nOption & SBT_OWNERDRAW) ){
		return false;
	}
	LRESULT lRes = ::SendMessage( m_hwndStatusBar, SB_GETTEXTLENGTH, (WPARAM)nIndex, 0 );
	if( HIWORD( lRes ) != (WORD)nOption ){
		return false;
	}
	size_t nLen = wcslen( pszText );
	if( LOWORD( lRes ) != nLen ){
		return false;
	}
	WCHAR szCurrent[1024];
	if( _countof( szCurrent ) <= nLen ){
		return false;
	}
	::SendMessage( m_hwndStatusBar, SB_GETTEXT, (WPARAM)nIndex, (LPARAM)szCurrent );
	szCurrent[nLen] = L'\0';
	return 0 == wcscmp( szCurrent, pszText );
}


/*!
 * CMainStatusBar::SetKeyMacroRecMark - キーマクロの記録中を示す「REC」の欄を設定する
 *
 * 「REC」の欄は自前描画 (SBT_OWNERDRAW) で、CEditWnd が WM_DRAWITEM で記録中かどうかに応じて描く。
 * 文字列では比較できないため、前回設定したときの記録中の状態を覚えておき、変わったときだけ設定する。
 * ステータスバーを作り直したときは未設定に戻すので、最初の 1 回は必ず設定する。
 */
void CMainStatusBar::SetKeyMacroRecMark(
	bool	bRecording		//!< [in] このウィンドウでキーマクロを記録中か
)
{
	if( NULL == m_hwndStatusBar ){
		return;
	}
	int nState = bRecording ? 1 : 0;
	if( nState == m_nRecMarkState ){
		return;
	}
	m_nRecMarkState = nState;
	StatusBar_SetText( m_hwndStatusBar, 5 | SBT_OWNERDRAW, L"" );
}


