/*!	@file
	@brief GREPダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/CDlgGrep.h"
#include "CGrepAgent.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/file.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "sakura_rc.h"


CDlgGrep::CDlgGrep()
{
	m_bSubFolder = FALSE;				// サブフォルダからも検索する
	m_bFromThisText = FALSE;			// この編集中のテキストから検索する
	m_sSearchOption.Reset();			// 検索オプション
	m_nGrepCharSet = CODE_SJIS;			// 文字コードセット
	m_bGrepOutputLine = TRUE;			// 行を出力するか該当部分だけ出力するか
	m_nGrepOutputStyle = 1;				// Grep: 出力形式
	m_szFile[0] = 0;
	m_szFolder[0] = 0;
	return;
}

/*!
	コンボボックスのドロップダウンメッセージを捕捉する

	@date 2013.03.24 novice 新規作成
*/
BOOL CDlgGrep::OnCbnDropDown( HWND hwndCtl, int wID )
{
	switch( wID ){
	case IDC_COMBO_TEXT:
		if ( ::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_sSearchKeywords.m_aSearchKeys.size();
			for( int i = 0; i < nSize; ++i ){
				Combo_AddString( hwndCtl, m_pShareData->m_sSearchKeywords.m_aSearchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_FILE:
		if ( ::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_sSearchKeywords.m_aGrepFiles.size();
			for( int i = 0; i < nSize; ++i ){
				Combo_AddString( hwndCtl, m_pShareData->m_sSearchKeywords.m_aGrepFiles[i] );
			}
		}
		break;
	case IDC_COMBO_FOLDER:
		if ( ::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_sSearchKeywords.m_aGrepFolders.size();
			for( int i = 0; i < nSize; ++i ){
				Combo_AddString( hwndCtl, m_pShareData->m_sSearchKeywords.m_aGrepFolders[i] );
			}
		}
		break;
	}
	return CDialog::OnCbnDropDown( hwndCtl, wID );
}

/* モーダルダイアログの表示 */
int CDlgGrep::DoModal( HINSTANCE hInstance, HWND hwndParent, const TCHAR* pszCurrentFilePath )
{
	m_bSubFolder = m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder;			// Grep: サブフォルダも検索
	m_sSearchOption = m_pShareData->m_sSearchOptionForGrepDialog;		// 検索オプション
	m_nGrepCharSet = m_pShareData->m_Common.m_sSearch.m_nGrepCharSet;			// 文字コードセット
	m_bGrepOutputLine = m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine;	// 行を出力するか該当部分だけ出力するか
	m_nGrepOutputStyle = m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle;	// Grep: 出力形式

	// 2013.05.21 コンストラクタからDoModalに移動
	// m_strText は呼び出し元で設定済み
	if( m_szFile[0] == _T('\0') && m_pShareData->m_sSearchKeywords.m_aGrepFiles.size() ){
		_tcscpy( m_szFile, m_pShareData->m_sSearchKeywords.m_aGrepFiles[0] );		/* 検索ファイル */
	}
	if( m_szFolder[0] == _T('\0') && m_pShareData->m_sSearchKeywords.m_aGrepFolders.size() ){
		_tcscpy( m_szFolder, m_pShareData->m_sSearchKeywords.m_aGrepFolders[0] );	/* 検索フォルダ */
	}

	if( pszCurrentFilePath ){	// 2010.01.10 ryoji
		_tcscpy(m_szCurrentFilePath, pszCurrentFilePath);
	}

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_GREP, NULL );
}

//	2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL CDlgGrep::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	/* ユーザーがコンボボックスのエディット コントロールに入力できるテキストの長さを制限する */
	//	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), _MAX_PATH - 1 );
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), _MAX_PATH - 1 );
	Combo_LimitText( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), _MAX_PATH - 1 );

	/* コンボボックスのユーザー インターフェイスを拡張インターフェースにする */
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_TEXT ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ), TRUE );
	Combo_SetExtendedUI( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ), TRUE );

	/* ダイアログのアイコン */
//2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig   = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessageAny( GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

	// 2002/09/22 Moca Add
	int i;
	/* 文字コードセット選択コンボボックス初期化 */
	CCodeTypesForCombobox cCodeTypes;
	for( i = 0; i < cCodeTypes.GetCount(); ++i ){
		int idx = Combo_AddString( ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET ), cCodeTypes.GetName(i) );
		Combo_SetItemData( ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET ), idx, cCodeTypes.GetCode(i) );
	}
	//	2007.02.09 bosagami
	HWND hFolder = ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER );
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);


	// フォント設定	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessageAny( GetItemHwnd( IDC_COMBO_TEXT ), WM_GETFONT, 0, 0 );
	HFONT hFont = SetMainFont( GetItemHwnd( IDC_COMBO_TEXT ) );
	m_cFontText.SetFont( hFontOld, hFont, GetItemHwnd( IDC_COMBO_TEXT ) );

	/* 基底クラスメンバ */
//	CreateSizeBox();
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

/*! @brief フォルダ指定EditBoxのコールバック関数

	@date 2007.02.09 bosagami 新規作成
	@date 2007.09.02 genta ディレクトリチェックを強化
*/
LRESULT CALLBACK OnFolderProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	if(msg == WM_DROPFILES) 
	do {
		//	From Here 2007.09.02 genta 
		SFilePath sPath;
		if( DragQueryFile((HDROP)wparam, 0, NULL, 0 ) > _countof2(sPath) - 1 ){
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, sPath, _countof2(sPath) - 1);

		//ファイルパスの解決
		CSakuraEnvironment::ResolvePath(sPath);
		
		//	ファイルがドロップされた場合はフォルダを切り出す
		//	フォルダの場合は最後が失われるのでsplitしてはいけない．
		if( IsFileExists( sPath, true )){	//	第2引数がtrueだとディレクトリは対象外
			SFilePath szWork;
			SplitPath_FolderAndFile( sPath, szWork, NULL );
			_tcscpy( sPath, szWork );
		}

		SetWindowText(hwnd, sPath);
	}
	while(0);	//	1回しか通らない. breakでここまで飛ぶ

	return  CallWindowProc(g_pOnFolderProc,hwnd,msg,wparam,lparam);
}

BOOL CDlgGrep::OnDestroy()
{
	m_cFontText.ReleaseOnDestroy();
	return CDialog::OnDestroy();
}

BOOL CDlgGrep::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_CHK_FROMTHISTEXT:	/* この編集中のテキストから検索する */
		// 2010.05.30 関数化
		SetDataFromThisText( 0 != ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_FROMTHISTEXT ) );
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	/* 現在編集中のファイルのフォルダ */
		SetCurrentFolderToFolderComboBox();
		return TRUE;
	case IDC_BUTTON_FOLDER_UP:
		{
			HWND hwnd = GetItemHwnd( IDC_COMBO_FOLDER );
			TCHAR szFolder[_MAX_PATH];
			::GetWindowText( hwnd, szFolder, _countof(szFolder) );
			std::vector<std::tstring> vPaths;
			CGrepAgent::CreateFolders( szFolder, vPaths );
			int nFolderLen = 0;
			if( 0 < vPaths.size() ){
				// 最後のパスが操作対象
				auto_strcpy( szFolder, vPaths.rbegin()->c_str() );
				if( DirectoryUp( szFolder ) ){
					*(vPaths.rbegin()) = szFolder;
					szFolder[0] = _T('\0');
					for( int i = 0 ; i < (int)vPaths.size(); i++ ){
						TCHAR szFolderItem[_MAX_PATH];
						auto_strcpy( szFolderItem, vPaths[i].c_str() );
						if( auto_strchr( szFolderItem, _T(';') ) ){
							szFolderItem[0] = _T('"');
							auto_strcpy( szFolderItem + 1, vPaths[i].c_str() );
							auto_strcat( szFolderItem, _T("\"") );
						}
						if( i ){
							auto_strcat( szFolder, _T(";") );
						}
						auto_strcat( szFolder, szFolderItem );
					}
					::SetWindowText( hwnd, szFolder );
				}
			}
		}
		return TRUE;


//	case IDC_CHK_LOHICASE:	/* 英大文字と英小文字を区別する */
//		MYTRACE( _T("IDC_CHK_LOHICASE\n") );
//		return TRUE;
	case IDC_CHK_REGULAREXP:	/* 正規表現 */
//		MYTRACE( _T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) = %d\n"), ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) );
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ) ){
			// From Here Jun. 26, 2001 genta
			//	正規表現ライブラリの差し替えに伴う処理の見直し
			if( !CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, true ) ){
				::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
			}else{
				//	To Here Jun. 26, 2001 genta
				/* 英大文字と英小文字を区別する */
				//	正規表現のときも選択できるように。
//				::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
//				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

				//2001/06/23 N.Nakatani
				/* 単語単位で検索 */
				::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
			}
		}else{
			/* 英大文字と英小文字を区別する */
			//	正規表現のときも選択できるように。
//			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), TRUE );
//			::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 0 );


//2001/06/23 N.Nakatani
//単語単位のgrepが実装されたらコメントを外すと思います
//2002/03/07実装してみた。
			/* 単語単位で検索 */
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), TRUE );

		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		/* フォルダ参照ボタン */
		{
			TCHAR	szFolder[MAX_PATH];
			/* 検索フォルダ */
			::DlgItem_GetText( GetHwnd(), IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1 );
			if( szFolder[0] == _T('\0') ){
				::GetCurrentDirectory( _countof( szFolder ), szFolder );
			}
			if( SelectDir( GetHwnd(), _T("検索するフォルダを選んでください"), szFolder, szFolder ) ){
				::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szFolder );
			}
		}

		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		/* フォルダの初期値をカレントフォルダにする */
		{
			m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_DEFAULTFOLDER );
		}
		return TRUE;
	case IDOK:
		/* ダイアログデータの取得 */
		if( GetData() ){
//			::EndDialog( hwndDlg, TRUE );
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog( hwndDlg, FALSE );
		CloseDialog( FALSE );
		return TRUE;
	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}



/* ダイアログデータの設定 */
void CDlgGrep::SetData( void )
{
	/* 検索文字列 */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_TEXT, m_strText.c_str() );

	/* 検索ファイル */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_FILE, m_szFile );

	/* 検索フォルダ */
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, m_szFolder );
	if( m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder ){
		if( SetVcsRepositoryRootFolderToFolderComboBox() == false ){
			SetCurrentFolderToFolderComboBox();
		}
	}

	/* サブフォルダからも検索する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_SUBFOLDER, m_bSubFolder );

	// この編集中のテキストから検索する
	::CheckDlgButton( GetHwnd(), IDC_CHK_FROMTHISTEXT, m_bFromThisText );
	// 2010.05.30 関数化
	SetDataFromThisText( m_bFromThisText != FALSE );

	/* 英大文字と英小文字を区別する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase );

	// 2001/06/23 N.Nakatani 現時点ではGrepでは単語単位の検索はサポートできていません
	// 2002/03/07 テストサポート
	/* 一致する単語のみ検索する */
	::CheckDlgButton( GetHwnd(), IDC_CHK_WORD, m_sSearchOption.bWordOnly );
//	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ) , false );	//チェックボックスを使用不可にすも


	/* 文字コード自動判別 */
//	::CheckDlgButton( GetHwnd(), IDC_CHK_KANJICODEAUTODETECT, m_bKanjiCode_AutoDetect );

	// 2002/09/22 Moca Add
	/* 文字コードセット */
	{
		int		nIdx, nCurIdx;
		ECodeType nCharSet;
		HWND	hWndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET );
		nCurIdx = Combo_GetCurSel( hWndCombo );
		CCodeTypesForCombobox cCodeTypes;
		for( nIdx = 0; nIdx < cCodeTypes.GetCount(); nIdx++ ){
			nCharSet = (ECodeType)Combo_GetItemData( hWndCombo, nIdx );
			if( nCharSet == m_nGrepCharSet ){
				nCurIdx = nIdx;
			}
		}
		Combo_SetCurSel( hWndCombo, nCurIdx );
	}

	/* 行を出力するか該当部分だけ出力するか */
	if( m_bGrepOutputLine ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTLINE, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTMARKED, TRUE );
	}

	/* Grep: 出力形式 */
	if( 1 == m_nGrepOutputStyle ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}else
	if( 2 == m_nGrepOutputStyle ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE2, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1, TRUE );
	}

	// From Here Jun. 29, 2001 genta
	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if( CheckRegexpVersion( GetHwnd(), IDC_STATIC_JRE32VER, false )
		&& m_sSearchOption.bRegularExp){
		/* 英大文字と英小文字を区別する */
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 1 );
		//	正規表現のときも選択できるように。
//		::CheckDlgButton( GetHwnd(), IDC_CHK_LOHICASE, 1 );
//		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_LOHICASE ), FALSE );

		// 2001/06/23 N.Nakatani
		/* 単語単位で探す */
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_WORD ), FALSE );
	}
	else {
		::CheckDlgButton( GetHwnd(), IDC_CHK_REGULAREXP, 0 );
	}
	// To Here Jun. 29, 2001 genta

	if( m_szCurrentFilePath[0] != _T('\0') ){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_FROMTHISTEXT ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_FROMTHISTEXT ), FALSE );
	}

	// フォルダの初期値をカレントフォルダにする
	::CheckDlgButton( GetHwnd(), IDC_CHK_DEFAULTFOLDER, m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder );

	return;
}


/*!
	現在編集中ファイルから検索チェックでの設定
*/
void CDlgGrep::SetDataFromThisText( bool bChecked )
{
	BOOL bEnableControls = TRUE;
	if( 0 != m_szCurrentFilePath[0] && bChecked ){
		TCHAR	szWorkFolder[MAX_PATH];
		TCHAR	szWorkFile[MAX_PATH];
		// 2003.08.01 Moca ファイル名はスペースなどは区切り記号になるので、""で囲い、エスケープする
		szWorkFile[0] = _T('"');
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile + 1 );
		_tcscat( szWorkFile, _T("\"") ); // 2003.08.01 Moca
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_FILE, szWorkFile );

		::CheckDlgButton( GetHwnd(), IDC_CHK_SUBFOLDER, BST_UNCHECKED );
		bEnableControls = FALSE;
	}
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FILE ),    bEnableControls );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_FOLDER ),  bEnableControls );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_BUTTON_FOLDER ), bEnableControls );
	::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHK_SUBFOLDER ), bEnableControls );
	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー  */
int CDlgGrep::GetData( void )
{

	/* サブフォルダからも検索する*/
	m_bSubFolder = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_SUBFOLDER );

	m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = m_bSubFolder;		/* Grep：サブフォルダも検索 */

	/* この編集中のテキストから検索する */
	m_bFromThisText = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_FROMTHISTEXT );
	/* 英大文字と英小文字を区別する */
	m_sSearchOption.bLoHiCase = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_LOHICASE ));

	//2001/06/23 N.Nakatani
	/* 単語単位で検索 */
	m_sSearchOption.bWordOnly = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_WORD ));

	/* 正規表現 */
	m_sSearchOption.bRegularExp = (0!=::IsDlgButtonChecked( GetHwnd(), IDC_CHK_REGULAREXP ));

	/* 文字コード自動判別 */
//	m_bKanjiCode_AutoDetect = ::IsDlgButtonChecked( GetHwnd(), IDC_CHK_KANJICODEAUTODETECT );

	/* 文字コードセット */
	{
		int		nIdx;
		HWND	hWndCombo = ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET );
		nIdx = Combo_GetCurSel( hWndCombo );
		m_nGrepCharSet = (ECodeType)Combo_GetItemData( hWndCombo, nIdx );
	}


	/* 行を出力するか該当部分だけ出力するか */
	m_bGrepOutputLine = ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTLINE );

	/* Grep: 出力形式 */
	if( TRUE == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTSTYLE1 ) ){
		m_nGrepOutputStyle = 1;				/* Grep: 出力形式 */
	}
	if( TRUE == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_OUTPUTSTYLE2 ) ){
		m_nGrepOutputStyle = 2;				/* Grep: 出力形式 */
	}



	/* 検索文字列 */
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_TEXT, &vText[0], nBufferSize);
	m_strText = to_wchar(&vText[0]);
	/* 検索ファイル */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_FILE, m_szFile, _countof2(m_szFile) );
	/* 検索フォルダ */
	::DlgItem_GetText( GetHwnd(), IDC_COMBO_FOLDER, m_szFolder, _countof2(m_szFolder) );

	m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = m_nGrepCharSet;			// 文字コード自動判別
	m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = m_bGrepOutputLine;	// 行を出力するか該当部分だけ出力するか
	m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = m_nGrepOutputStyle;	// Grep: 出力形式


//やめました
//	if( 0 == wcslen( m_szText ) ){
//		WarningMessage(	GetHwnd(), _T("検索のキーワードを指定してください。") );
//		return FALSE;
//	}
	/* この編集中のテキストから検索する */
	if( m_szFile[0] == _T('\0') ){
		//	Jun. 16, 2003 Moca
		//	検索パターンが指定されていない場合のメッセージ表示をやめ、
		//	「*.*」が指定されたものと見なす．
		_tcscpy( m_szFile, _T("*.*") );
	}
	if( m_szFolder[0] == _T('\0') ){
		WarningMessage(	GetHwnd(), _T("検索対象フォルダを指定してください。") );
		return FALSE;
	}

	{
		//カレントディレクトリを保存。このブロックから抜けるときに自動でカレントディレクトリは復元される。
		CCurrentDirectoryBackupPoint cCurDirBackup;
		
		// 相対パス→絶対パス
		if( !::SetCurrentDirectory( m_szFolder ) ){
			WarningMessage(	GetHwnd(), _T("検索対象フォルダが正しくありません。") );
			return FALSE;
		}
		::GetCurrentDirectory( MAX_PATH, m_szFolder );
	}

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()追加に伴う変更
	/* 検索文字列 */
	if( 0 < m_strText.size() ){
		// From Here Jun. 26, 2001 genta
		//	正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( m_sSearchOption.bRegularExp  && !CheckRegexpSyntax( m_strText.c_str(), GetHwnd(), true, nFlag) ){
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta 正規表現ライブラリ差し替え
		if( m_strText.size() < _MAX_PATH ){
			CSearchKeywordManager().AddToSearchKeyArr( m_strText.c_str() );
			m_pShareData->m_Common.m_sSearch.m_sSearchOption = m_sSearchOption;		// 検索オプション
			m_pShareData->m_sSearchOptionForGrepDialog = m_sSearchOption;
		}
	}

	// この編集中のテキストから検索する場合、履歴に残さない	Uchi 2008/5/23
	if (!m_bFromThisText) {
		/* 検索ファイル */
		CSearchKeywordManager().AddToGrepFileArr( m_szFile );

		/* 検索フォルダ */
		CSearchKeywordManager().AddToGrepFolderArr( m_szFolder );
	}

	return TRUE;
}


/*!
 *	SetCurrentFolderToFolderComboBox - カレントフォルダを検索フォルダ指定用コンボボックスに設定する
 *
 *	編集対象のファイルを開いていない場合には、代わりにカレントディレクトリを設定する。
 */
void CDlgGrep::SetCurrentFolderToFolderComboBox( void )
{
	TCHAR	szWorkFolder[MAX_PATH];
	TCHAR	szWorkFile[MAX_PATH];

	/* ファイルを開いているか */
	if( _tcslen( m_szCurrentFilePath ) > 0 ){
		SplitPath_FolderAndFile( m_szCurrentFilePath, szWorkFolder, szWorkFile );
	}else{
		/* 現在のプロセスのカレントディレクトリを取得します */
		::GetCurrentDirectory( _countof( szWorkFolder ) - 1, szWorkFolder );
	}
	::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, szWorkFolder );
}


/*!
 * SetVcsRepositoryRootDirectoryToFolderComboBox - VCSルートディレクトリを検索し、検索フォルダ指定用コンボボックスに設定する
 *
 *	@return true: VCSルートディレクトリを設定 / false: VCSルートディレクトリを検出できず設定失敗
 */
bool CDlgGrep::SetVcsRepositoryRootFolderToFolderComboBox( void )
{
	TCHAR currentPathName[MAX_PATH];
	TCHAR fileOrDirectoryName[MAX_PATH];
	bool vcsRepositoryDetected;
	TCHAR vcsDirectoryPathName[MAX_PATH];

	SplitPath_FolderAndFile( m_szCurrentFilePath, currentPathName, fileOrDirectoryName );
	vcsRepositoryDetected = GetVcsRepositoryRootDir( vcsDirectoryPathName, currentPathName );
	if( vcsRepositoryDetected == true ){
		::DlgItem_SetText( GetHwnd(), IDC_COMBO_FOLDER, vcsDirectoryPathName );
		return true;
	}

	return false;
}


