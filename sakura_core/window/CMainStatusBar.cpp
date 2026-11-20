#include "StdAfx.h"
#include "CMainStatusBar.h"
#include "window/CEditWnd.h"
#include "CEditApp.h"

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


