/*!	@file
	@brief タイプ別設定インポート確認ダイアログ

	@author Uchi
	@date 2010/4/17 新規作成
*/
/*
	Copyright (C) 2010, Uchi

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
#include "CDlgTypeAscertain.h"
#include "util/shell.h"
#include "sakura_rc.h"

//内部使用定数
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

//  Constructors
CDlgTypeAscertain::CDlgTypeAscertain()
	: m_psi(NULL)
{
}

// モーダルダイアログの表示
int CDlgTypeAscertain::DoModal( HINSTANCE hInstance, HWND hwndParent, SAscertainInfo* psAscertainInfo )
{
	m_psi = psAscertainInfo;

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPE_ASCERTAIN, (LPARAM)NULL );
}

// ボタンクリック
BOOL CDlgTypeAscertain::OnBnClicked( int wID )
{
	switch( wID ){
	case IDOK:
		::EndDialog( GetHwnd(), TRUE );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}


/* ダイアログデータの設定 */
void CDlgTypeAscertain::SetData( void )
{
	// タイプ名設定
	::SetWindowText( ::GetDlgItem( GetHwnd(), IDC_STATIC_TYPE_TO   ), to_tchar(m_psi->sTypeNameTo.c_str()) );
	::SetWindowText( ::GetDlgItem( GetHwnd(), IDC_STATIC_TYPE_FILE ), to_tchar(m_psi->sTypeNameFile.c_str()) );
	return;
}

