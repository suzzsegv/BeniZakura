/*! @file
	@brief タイプ別設定 - 支援

	@date 2008.04.12 kobake CPropTypes.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, こおり, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "dlg/CDlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/file.h" // _IS_REL_PATH
#include "util/window.h"
#include "sakura_rc.h"


struct SHokanMethod{
	int nMethod;
	std::wstring name;
};

static std::vector<SHokanMethod>* GetHokanMethodList()
{
	static std::vector<SHokanMethod> methodList;
	return &methodList;
}


// 2001/06/13 Start By asa-o: タイプ別設定の支援タブに関する処理

/* メッセージ処理 */
INT_PTR CPropTypesSupport::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p2 */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		/* 入力補完 単語ファイル */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_HOKANFILE ), _MAX_PATH - 1 );

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID、 コントロールID、 またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			switch( wID ){
			case IDC_BUTTON_HOKANFILE_REF:	/* 入力補完 単語ファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szHokanFile ) ){
						GetInidirOrExedir( szPath, m_Types.m_szHokanFile );
					}else{
						_tcscpy( szPath, m_Types.m_szHokanFile );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szHokanFile, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );
					}
				}
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_KILLACTIVE:
			/* ダイアログデータの取得 p2 */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = 3;
			return TRUE;
		}
		break;

	}
	return FALSE;
}

/* ダイアログデータの設定 */
void CPropTypesSupport::SetData( HWND hwndDlg )
{
	/* 入力補完 単語ファイル */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile );

	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		std::vector<SHokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString( hCombo, L"なし" );
		Combo_SetCurSel( hCombo, 0 );
		size_t nSize = pMedothList->size();
		for( size_t i = 0; i < nSize; i++ ){
			ApiWrap::Combo_AddString( hCombo, (*pMedothList)[i].name.c_str() );
			if( m_Types.m_nHokanType == (*pMedothList)[i].nMethod ){
				Combo_SetCurSel( hCombo, i + 1 );
			}
		}
	}

//	2001/06/19 asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANLOHICASE, m_Types.m_bHokanLoHiCase ? BST_CHECKED : BST_UNCHECKED);

	// 2003.06.25 Moca ファイルからの補完機能
	::CheckDlgButton( hwndDlg, IDC_CHECK_HOKANBYFILE, m_Types.m_bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButtonBool( hwndDlg, IDC_CHECK_HOKANBYKEYWORD, m_Types.m_bUseHokanByKeyword );

	// 終了時、改行の一致を検査する	2013/4/14 Uchi
	::CheckDlgButton( hwndDlg, IDC_CHECK_CHKENTERATEND, m_Types.m_bChkEnterAtEnd ? BST_CHECKED : BST_UNCHECKED);
}

/* ダイアログデータの取得 */
int CPropTypesSupport::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 3;

//	2001/06/19	asa-o
	/* 入力補完機能：英大文字小文字を同一視する */
	m_Types.m_bHokanLoHiCase = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANLOHICASE ) != 0;

	m_Types.m_bUseHokanByFile = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HOKANBYFILE ) != 0;
	m_Types.m_bUseHokanByKeyword = IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_HOKANBYKEYWORD );

	/* 入力補完 単語ファイル */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_HOKANFILE, m_Types.m_szHokanFile, _countof2( m_Types.m_szHokanFile ));

	// 入力補完種別
	{
		HWND hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_HOKAN_TYPE );
		int i = Combo_GetCurSel( hCombo );
		if( 0 == i ){
			m_Types.m_nHokanType = 0;
		}else if( CB_ERR != i ){
			m_Types.m_nHokanType = (*GetHokanMethodList())[i - 1].nMethod;
		}
	}

	// 終了時、改行の一致を検査する	2013/4/14 Uchi
	m_Types.m_bChkEnterAtEnd = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CHKENTERATEND ) != 0;

	return TRUE;
}

// 2001/06/13 End

/*! 補完種別の追加
/*/
void CPropTypesSupport::AddHokanMethod(int nMethod, const WCHAR* szName)
{
	SHokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

void CPropTypesSupport::RemoveHokanMethod(int nMethod, const WCHAR* szName)
{
	int nSize = GetHokanMethodList()->size();
	for(int i = 0; i < nSize; i++ ){
		if( (*GetHokanMethodList())[i].nMethod == (EOutlineType)nMethod ){
			GetHokanMethodList()->erase( GetHokanMethodList()->begin() + i );
			break;
		}
	}
}
