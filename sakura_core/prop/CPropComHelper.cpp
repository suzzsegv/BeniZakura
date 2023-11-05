/*!	@file
	@brief 共通設定ダイアログボックス、「支援」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, MIK, jepro, asa-o
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Moca
	Copyright (C) 2013, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "dlg/CDlgOpenFile.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"


//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropHelper::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropHelper::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* Helper メッセージ処理 */
INT_PTR CPropHelper::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Helper */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 Helper */
			GetData( hwndDlg );
			switch( wID ){
			// ai 02/05/21 Add S
			case IDC_BUTTON_KEYWORDHELPFONT:	/* キーワードヘルプの「フォント」ボタン */
				{
					LOGFONT   lf = m_Common.m_sHelper.m_lf;
					INT nPointSize = m_Common.m_sHelper.m_nPointSize;

					if( MySelectFont( &lf, &nPointSize, hwndDlg, false) ){
						m_Common.m_sHelper.m_lf = lf;
						m_Common.m_sHelper.m_nPointSize = nPointSize;	// 2009.10.01 ryoji
						// キーワードヘルプ フォント表示	// 2013/4/24 Uchi
						HFONT hFont = SetFontLabel( hwndDlg, IDC_STATIC_KEYWORDHELPFONT, m_Common.m_sHelper.m_lf, m_Common.m_sHelper.m_nPointSize);
						if(m_hKeywordHelpFont != NULL){
							::DeleteObject( m_hKeywordHelpFont );
						}
						m_hKeywordHelpFont = hFont;
					}
				}
				return TRUE;
			// ai 02/05/21 Add E
			case IDC_BUTTON_OPENMDLL:	/* MIGEMODLL場所指定「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパス
					// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDll ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDll, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDll );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.dll"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDll, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll );
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENMDICT:	/* MigemoDict場所指定「参照...」ボタン */
				{
					TCHAR	szPath[_MAX_PATH];
					/* 検索フォルダ */
					// 2007.05.27 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDict ) ){
						GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDict, TRUE );
					}else{
						_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDict );
					}
					if( SelectDir( hwndDlg, _T("検索するフォルダを選んでください"), szPath, szPath ) ){
						_tcscpy( m_Common.m_sHelper.m_szMigemoDict, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
//		switch( idCtrl ){
//		case ???????:
//			return 0L;
//		default:
			switch( pNMHDR->code ){
			case PSN_KILLACTIVE:
//				MYTRACE( _T("Helper PSN_KILLACTIVE\n") );
				/* ダイアログデータの取得 Helper */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_HELPER;
				return TRUE;
			}
//			break;	/* default */
//		}

//		MYTRACE( _T("pNMHDR->hwndFrom=%xh\n"), pNMHDR->hwndFrom );
//		MYTRACE( _T("pNMHDR->idFrom  =%xh\n"), pNMHDR->idFrom );
//		MYTRACE( _T("pNMHDR->code    =%xh\n"), pNMHDR->code );
//		MYTRACE( _T("pMNUD->iPos    =%d\n"), pMNUD->iPos );
//		MYTRACE( _T("pMNUD->iDelta  =%d\n"), pMNUD->iDelta );
		break;	/* WM_NOTIFY */

	case WM_DESTROY:
		// キーワードヘルプ フォント破棄	// 2013/4/24 Uchi
		if (m_hKeywordHelpFont != NULL) {
			::DeleteObject( m_hKeywordHelpFont );
			m_hKeywordHelpFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}

/* ダイアログデータの設定 Helper */
void CPropHelper::SetData( HWND hwndDlg )
{
	/* 補完候補決定キー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_sHelper.m_bHokanKey_RETURN );	//VK_RETURN 補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_sHelper.m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_sHelper.m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	// キーワードヘルプ フォント	// 2013/4/24 Uchi
	m_hKeywordHelpFont = SetFontLabel( hwndDlg, IDC_STATIC_KEYWORDHELPFONT, m_Common.m_sHelper.m_lf, m_Common.m_sHelper.m_nPointSize);

	//migemo dict
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll);
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict);
}


/* ダイアログデータの取得 Helper */
int CPropHelper::GetData( HWND hwndDlg )
{
	/* 補完候補決定キー */
	m_Common.m_sHelper.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN 補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	//migemo dict
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll, _countof( m_Common.m_sHelper.m_szMigemoDll ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict, _countof( m_Common.m_sHelper.m_szMigemoDict ));

	return TRUE;
}
