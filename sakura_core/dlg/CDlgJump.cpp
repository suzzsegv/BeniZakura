/*!	@file
	@brief 指定行へのジャンプダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, MIK, YAZAKI
	Copyright (C) 2004, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgJump.h"
#include "doc/CEditDoc.h"
#include "func/Funccode.h"
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"
#include "util/shell.h"
#include "window/CEditWnd.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ジャンプ CDlgJump.cpp
const DWORD p_helpids[] = {	//12800
	IDC_BUTTON_JUMP,				HIDC_JUMP_BUTTON_JUMP,			//ジャンプ
	IDCANCEL,						HIDCANCEL_JUMP,					//キャンセル
	IDC_EDIT_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			//行番号
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_JUMP_RADIO_LINENUM_LAYOUT,	//折り返し単位
	IDC_RADIO_LINENUM_CRLF,			HIDC_JUMP_RADIO_LINENUM_CRLF,	//改行単位
	IDC_SPIN_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			//12870,	//
	0, 0
};



CDlgJump::CDlgJump()
{
	m_nLineNum = 0;			/* 行番号 */

	return;
}



/* モーダルダイアログの表示 */
int CDlgJump::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return CDialog::DoModal( hInstance, hwndParent, IDD_JUMP, lParam );
}



BOOL CDlgJump::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	int				nData;

	idCtrl = (int)wParam;
	pMNUD  = (NM_UPDOWN*)lParam;
	/* スピンコントロールの処理 */
	switch( idCtrl ){
		case IDC_SPIN_LINENUM:
			/* ジャンプしたい行番号の指定 */
			nData = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nData;
			}else
			if( pMNUD->iDelta > 0 ){
				nData--;
			}
			if( nData < 1 ){
				nData = 1;
			}
			::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, nData, FALSE );
			break;

		default:
			break;
	}

	return TRUE;
}



BOOL CDlgJump::OnBnClicked( int wID )
{
	switch( wID ){
		case IDC_BUTTON_JUMP:			/* 指定行へジャンプ */
			if( 0 < GetData() ){
				CloseDialog( 1 );
			}else{
				OkMessage( GetHwnd(), _T("正しく行番号を入力してください。") );
			}
			{
				CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
				pCEditDoc->m_pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP, TRUE, 0, 0, 0, 0);	//	ジャンプコマンド発行
			}
			return TRUE;

		case IDCANCEL:
			::EndDialog( GetHwnd(), FALSE );

			return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}



/* ダイアログデータの設定 */
void CDlgJump::SetData( void )
{
	if( 0 == m_nLineNum ){
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_LINENUM, _T("") );	/* 行番号 */
	}else{
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, m_nLineNum, FALSE );	/* 前回の行番号 */
	}

	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ), TRUE );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_CRLF ), TRUE );

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( m_pShareData->m_bLineNumIsCRLF_ForJump ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, FALSE );
	}

	return;
}



/* ダイアログデータの取得 */
/*   TRUE==正常   FALSE==入力エラー  */
int CDlgJump::GetData( void )
{
	BOOL	pTranslated;

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ) ){
		m_pShareData->m_bLineNumIsCRLF_ForJump = false;
	}else{
		m_pShareData->m_bLineNumIsCRLF_ForJump = true;
	}

	/* 行番号 */
	m_nLineNum = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, &pTranslated, FALSE );
	if( m_nLineNum == 0 && !pTranslated ){
		return FALSE;
	}
	return TRUE;
}



LPVOID CDlgJump::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}



