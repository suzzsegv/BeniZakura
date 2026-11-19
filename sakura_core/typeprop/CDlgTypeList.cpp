/*!	@file
	@brief ファイルタイプ一覧ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, Uchi, Beta.Ito, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "types/CType.h" // use CDlgTypeList定義

#include "typeprop/CDlgTypeList.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/24 Uchi
#include "env/CDocTypeManager.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "util/RegKey.h"

typedef std::basic_string<WCHAR> wstring;

#define BUFFER_SIZE 1024
#define ACTION_NAME	(L"SakuraEditor")
#define PROGID_BACKUP_NAME	(L"SakuraEditorBackup")
#define ACTION_BACKUP_PATH	(L"\\ShellBackup")

//関数プロトタイプ
int CopyRegistry(HKEY srcRoot, const wstring srcPath, HKEY destRoot, const wstring destPath);
int RegistExt(LPCWSTR sExt, bool bDefProg);
int UnregistExt(LPCWSTR sExt);
int CheckExt(LPCWSTR sExt, bool *pbRMenu, bool *pbDblClick);

//内部使用定数
static const int PROP_TEMPCHANGE_FLAG = 0x10000;

/* モーダルダイアログの表示 */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, SResult* psResult )
{
	int	nRet;
	m_nSettingType = psResult->cDocumentType;
	m_bAlertFileAssociation = true;
	m_bEnableTempChange = psResult->bTempChange;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, (LPARAM)NULL );
	if( -1 == nRet ){
		return FALSE;
	}
	else{
		//結果
		psResult->cDocumentType = CTypeConfig(nRet & ~PROP_TEMPCHANGE_FLAG);
		psResult->bTempChange   = ((nRet & PROP_TEMPCHANGE_FLAG) != 0);
		return TRUE;
	}
}


BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		//	Nov. 29, 2000	genta
		//	動作変更: 指定タイプの設定ダイアログ→一時的に別の設定を適用
		::EndDialog(
			GetHwnd(),
			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	//	Nov. 29, 2000	From Here	genta
	//	適用する型の一時的変更
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog(
			GetHwnd(),
 			List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) )
			| PROP_TEMPCHANGE_FLAG
		);
		return TRUE;
	//	Nov. 29, 2000	To Here
	case IDOK:
		::EndDialog( GetHwnd(), List_GetCurSel( GetDlgItem( GetHwnd(), IDC_LIST_TYPES ) ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), -1 );
		return TRUE;
	case IDC_BUTTON_IMPORT:
		Import();
		return TRUE;
	case IDC_BUTTON_EXPORT:
		Export();
		return TRUE;
	case IDC_BUTTON_INITIALIZE:
		InitializeType();
		return TRUE;
	}
	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );

}


INT_PTR CDlgTypeList::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
	HWND hwndRMenu = GetDlgItem( GetHwnd(), IDC_CHECK_EXT_RMENU );
	HWND hwndDblClick = GetDlgItem( GetHwnd(), IDC_CHECK_EXT_DBLCLICK );

	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		if( LOWORD(wParam) == IDC_LIST_TYPES )
		{
			switch( HIWORD(wParam) )
			{
			case LBN_SELCHANGE:
				if( types.m_szTypeExts[0] == '\0' ){
					::EnableWindow( hwndRMenu, FALSE );
					::EnableWindow( hwndDblClick, FALSE );
				}else{
					::EnableWindow( GetDlgItem( GetHwnd(), IDC_CHECK_EXT_RMENU ), TRUE );
					if( !m_bRegistryChecked[ nIdx ] ){
						WCHAR exts[_countof(types.m_szTypeExts)] = {0};
						wcscpy( exts, types.m_szTypeExts );
						const WCHAR	pszSeps[] = L" ;,";	// separator
						WCHAR *ext = _wcstok( exts, pszSeps );

						m_bExtRMenu[ nIdx ] = true;
						m_bExtDblClick[ nIdx ] = true;
						while( NULL != ext ){
							bool bRMenu;
							bool bDblClick;
							CheckExt( ext, &bRMenu, &bDblClick );
							m_bExtRMenu[ nIdx ] &= bRMenu;
							m_bExtDblClick[ nIdx ] &= bDblClick;
							ext = _wcstok( NULL, pszSeps );
						}
						m_bRegistryChecked[ nIdx ] = true;
					}
					BtnCtl_SetCheck( hwndRMenu, m_bExtRMenu[ nIdx ] );
					::EnableWindow( hwndDblClick, m_bExtRMenu[ nIdx ] );
					BtnCtl_SetCheck( hwndDblClick, m_bExtDblClick[ nIdx ] );
				}
				return TRUE;
			}
		}
		else if( LOWORD(wParam) == IDC_CHECK_EXT_RMENU && HIWORD(wParam) == BN_CLICKED )
		{
			bool checked = ( BtnCtl_GetCheck( hwndRMenu ) == TRUE ? true : false );
			if( ! AlertFileAssociation() ){		//レジストリ変更確認
				BtnCtl_SetCheck( hwndRMenu, !checked );
				break;
			}
			WCHAR exts[_countof(types.m_szTypeExts)] = {0};
			wcscpy( exts, types.m_szTypeExts );
			const WCHAR	pszSeps[] = L" ;,";	// separator
			WCHAR *ext = _wcstok( exts, pszSeps );
			int nRet;
			while( NULL != ext ){
				if( checked ){	//「右クリック」チェックON
					if( (nRet = RegistExt( ext, true )) != 0 )
					{
						WCHAR buf[BUFFER_SIZE] = {0};
						::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
						::MessageBox( GetHwnd(), (wstring(L"関連付けに失敗しました\n") + buf).c_str(), GSTR_APPNAME, MB_OK );
						break;
					}
				}else{			//「右クリック」チェックOFF
					if( (nRet = UnregistExt( ext )) != 0 )
					{
						WCHAR buf[BUFFER_SIZE] = {0};
						::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
						::MessageBox( GetHwnd(), (wstring(L"関連付け解除に失敗しました\n") + buf).c_str(), GSTR_APPNAME, MB_OK );
						break;
					}
				}
				m_bExtRMenu[ nIdx ] = checked;
				::EnableWindow( hwndDblClick, checked );
				m_bExtDblClick[ nIdx ] = checked;
				BtnCtl_SetCheck( hwndDblClick, checked );
				ext = _wcstok( NULL, pszSeps );
			}
			return TRUE;
		}
		else if( LOWORD(wParam) == IDC_CHECK_EXT_DBLCLICK && HIWORD(wParam) == BN_CLICKED )
		{
			bool checked = ( BtnCtl_GetCheck( hwndDblClick ) == TRUE ? true : false );
			if( ! AlertFileAssociation() ){		//レジストリ変更確認
				BtnCtl_SetCheck( hwndDblClick, !checked );
				break;
			}
			WCHAR exts[_countof(types.m_szTypeExts)] = {0};
			wcscpy( exts, types.m_szTypeExts );
			const WCHAR	pszSeps[] = L" ;,";	// separator
			WCHAR *ext = _wcstok( exts, pszSeps );
			int nRet;
			while( NULL != ext ){
				if( (nRet = RegistExt( ext, checked )) != 0 )
				{
					WCHAR buf[BUFFER_SIZE] = {0};
					::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, nRet, 0, buf, _countof(buf), NULL ); 
					::MessageBox( GetHwnd(), (wstring(L"関連付けに失敗しました\n") + buf).c_str(), GSTR_APPNAME, MB_OK );
					break;
				}
				m_bExtDblClick[ nIdx ] = checked;
				ext = _wcstok( NULL, pszSeps );
			}
			return TRUE;
		}
	}
	return result;
}


/* ダイアログデータの設定 */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	WCHAR	szText[64 + MAX_TYPES_EXTS + 10];
	int		nExtent = 0;
	HWND	hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	HDC		hDC = ::GetDC( hwndList );
	HFONT	hFont = (HFONT)::SendMessageAny(hwndList, WM_GETFONT, 0, 0);
	HFONT	hFontOld = (HFONT)::SelectObject(hDC, hFont);

	List_ResetContent( hwndList );	/* リストを空にする */
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));
		if( types.m_szTypeExts[0] != L'\0' ){		/* タイプ属性：拡張子リスト */
			auto_sprintf( szText, L"%ts ( %ts )",
				types.m_szTypeName,	/* タイプ属性：名称 */
				types.m_szTypeExts	/* タイプ属性：拡張子リスト */
			);
		}else{
			auto_sprintf( szText, L"%ts",
				types.m_szTypeName	/* タイプ属性：拡称 */
			);
		}
		::List_AddString( hwndList, szText );
		m_bRegistryChecked[ nIdx ] = FALSE;
		m_bExtRMenu[ nIdx ] = FALSE;
		m_bExtDblClick[ nIdx ] = FALSE;

		SIZE sizeExtent;
		if( ::GetTextExtentPoint32( hDC, szText, wcslen(szText), &sizeExtent) && sizeExtent.cx > nExtent ){
			nExtent = sizeExtent.cx;
		}
	}

	::SelectObject(hDC, hFontOld);
	::ReleaseDC( hwndList, hDC );
	List_SetHorizontalExtent( hwndList, nExtent + 8 );
	List_SetCurSel( hwndList, m_nSettingType.GetIndex() );
	::SendMessageAny( GetHwnd(), WM_COMMAND, MAKEWPARAM(IDC_LIST_TYPES, LBN_SELCHANGE), 0 );
	::EnableWindow( GetItemHwnd(IDC_BUTTON_TEMPCHANGE), m_bEnableTempChange );
	return;
}


// タイプ別設定インポート
//		2010/4/12 Uchi
bool CDlgTypeList::Import()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// インポート
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName ) );
	if (!cImpExpType.ImportUI( G_AppInstance(), GetHwnd() )) {
		// インポートをしていない
		return false;
	}

	// リスト再初期化
	SetData();
	List_SetCurSel( hwndList, nIdx );

	return true;
}

// タイプ別設定エクスポート
//		2010/4/12 Uchi
bool CDlgTypeList::Export()
{
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int nIdx = List_GetCurSel( hwndList );
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(nIdx));

	CImpExpType	cImpExpType( nIdx, types, hwndList );

	// エクスポート
	cImpExpType.SetBaseName( to_wchar( types.m_szTypeName) );
	if (!cImpExpType.ExportUI( G_AppInstance(), GetHwnd() )) {
		// エクスポートをしていない
		return false;
	}

	return true;
}

// タイプ別設定初期化
//		2010/4/12 Uchi
bool CDlgTypeList::InitializeType( void )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndList = GetDlgItem( GetHwnd(), IDC_LIST_TYPES );
	int iDocType = List_GetCurSel( hwndList );
	if (iDocType == 0) {
		// 基本の場合には何もしない
		return true;
	}
	STypeConfig& types = CDocTypeManager().GetTypeSetting(CTypeConfig(iDocType));
	int			nRet;
	if ( types.m_szTypeExts[0] != L'\0' ) { 
		nRet = ::MYMESSAGEBOX(
			GetHwnd(),
			MB_YESNO | MB_ICONQUESTION,
			GSTR_APPNAME,
			L"%ts を初期化します。 よろしいですか？",
			types.m_szTypeName );
		if (nRet != IDYES) {
			return false;
		}
	}

//	_DefaultConfig(&types);		//規定値をコピー
	types = CDocTypeManager().GetTypeSetting(CTypeConfig(0));	// 基本をコピー

	types.m_nIdx = iDocType;
	auto_sprintf( types.m_szTypeName, L"設定%d", iDocType+1 );
	wcscpy( types.m_szTypeExts, L"" );

	// リスト再初期化
	SetData();
	List_SetCurSel( hwndList, iDocType );

	InfoMessage( hwndDlg, L"%ts を初期化しました。", types.m_szTypeName );

	return true;
}

/*! 再帰的レジストリコピー */
int CopyRegistry(HKEY srcRoot, const wstring srcPath, HKEY destRoot, const wstring destPath)
{
	int errorCode;
	CRegKey keySrc;
	if((errorCode = keySrc.Open(srcRoot, srcPath.c_str(), KEY_READ)) != 0){ return errorCode; }

	CRegKey keyDest;
	if((errorCode = keyDest.Open(destRoot, destPath.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		if( (errorCode = keyDest.Create(destRoot, destPath.c_str())) != 0 ){ return errorCode; }
	}

	int index = 0;
	for (;;)
	{
		WCHAR szValue[ BUFFER_SIZE ] = {0};
		BYTE data[ BUFFER_SIZE ] = {0};
		DWORD dwDataLen;
		DWORD dwType;

		errorCode = keySrc.EnumValue(index, szValue, _countof(szValue), &dwType, data, _countof(data), &dwDataLen );
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			// 手抜き：データのサイズがBUFFER_SIZE(=1024)を超える場合を考慮していない
			if( (errorCode = keyDest.SetValue(szValue, data, dwDataLen, dwType)) != 0 ){ return errorCode; }
			index++;
		}
	}

	index = 0;
	WCHAR szSubKey[ BUFFER_SIZE ] = {0};
	for (;;)
	{
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			if( errorCode = CopyRegistry(srcRoot, srcPath + L"\\" + szSubKey, destRoot, destPath + L"\\" + szSubKey) ){ return errorCode; }
			index++;
		}
	}

	return errorCode;
}

/*! 再帰的レジストリ削除 */
int DeleteRegistry(HKEY root, const wstring path)
{
	int errorCode;
	CRegKey keySrc;
	if((errorCode = keySrc.Open(root, path.c_str(), KEY_READ | KEY_WRITE)) != 0){ return ERROR_SUCCESS; }

	int index = 0;
	index = 0;
	WCHAR szSubKey[ BUFFER_SIZE ] = {0};
	for (;;)
	{
		errorCode = keySrc.EnumKey(index, szSubKey, _countof(szSubKey));
		if( errorCode == ERROR_NO_MORE_ITEMS ){
			errorCode = 0;
			break;
		}else if( errorCode ){
			return errorCode;
		}else{
			if( (errorCode = DeleteRegistry(root, path + L"\\" + szSubKey)) != 0 ){ return errorCode; }
		}
	}
	keySrc.Close();
	if( (errorCode = CRegKey::DeleteKey(root, path.c_str())) != 0 ){ return errorCode; }

	return errorCode;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を行う
	@param sExt	拡張子
	@param bDefProg [in]既定フラグ（ダブルクリックで起動させるか）
	レジストリアクセス方針
	・管理者権限なしで実施したいため、HKLMは読み込みのみとし、書き込みはHKCUに行う。
	処理の流れ
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ
		・[HKCU\Software\Classes\.(拡張子)]を作成。値は「SakuraEditor_(拡張子)」
	・[HKCU\Software\Classes\.(拡張子)]の値が「SakuraEditor_(拡張子)」以外の場合、
		[HKCU\Software\Classes\.(拡張子)\SakuraEditorBackup]に値をコピーする
		値に「SakuraEditor_(拡張子)」を設定する
	・ProgID <- [HKCR\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)]の存在チェック
		存在しなければ
		・[HKLM\Software\Classes\(HKLMのProgID)]の存在チェック
		存在すれば
			・[HKLM\Software\Classes\(ProgID)]の構造を[HKCU\Software\Classes\(ProgID)]にコピーする
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\command]を作成。値は「"(サクラEXEパス)" "%1"」
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の値を「Sakura &Editor」とする
	・既定フラグ判定
		trueなら
			・[HKCU\Software\Classes\(ProgID)\shell]の値が空でなければ[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]に退避する
			・[HKCU\Software\Classes\(ProgID)\shell]の値を「SakuraEditor」とする
		falseなら
			・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]の存在チェック
				存在すれば、退避した値を[HKCU\Software\Classes\(ProgID)\shell]に設定
				存在しなければ、[HKCU\Software\Classes\(ProgID)\shell]の値を削除
*/
int RegistExt(LPCWSTR sExt, bool bDefProg)
{
	int errorCode = ERROR_SUCCESS;
	wstring sBasePath = wstring( L"Software\\Classes\\" );

	//小文字化
	WCHAR szLowerExt[MAX_PATH] = {0};
	wcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, wcslen(sExt));
	CharLower(szLowerExt);

	wstring sDotExt = sBasePath + L"." + szLowerExt;
	wstring sGenProgID = wstring() + L"SakuraEditor_" + szLowerExt;

	CRegKey keyExt_HKLM;
	WCHAR szProgID_HKLM[ BUFFER_SIZE ] = {0};
	if( ( errorCode = keyExt_HKLM.Open(HKEY_LOCAL_MACHINE, sDotExt.c_str(), KEY_READ) ) == 0 )
	{
		keyExt_HKLM.GetValue(NULL, szProgID_HKLM, _countof(szProgID_HKLM));
	}

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		if( (errorCode = keyExt.Create(HKEY_CURRENT_USER, sDotExt.c_str())) != 0 ){ return errorCode; }
	}

	WCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if(wcscmp( sGenProgID.c_str(), szProgID ) != 0) {
		if( szProgID[0] != L'\0' )
		{
			if( (errorCode = keyExt.SetValue(PROGID_BACKUP_NAME, szProgID)) != 0 ){ return errorCode; }
		} 
		if( (errorCode = keyExt.SetValue(NULL, sGenProgID.c_str())) != 0 ){ return errorCode; }
	}

	wstring sProgIDPath = sBasePath + sGenProgID;
	if( ! CRegKey::ExistsKey(HKEY_CURRENT_USER, sProgIDPath.c_str()) )
	{
		if( szProgID_HKLM[0] != L'\0' )
		{
			if( (errorCode = CopyRegistry(HKEY_LOCAL_MACHINE, (sBasePath + szProgID_HKLM).c_str(), HKEY_CURRENT_USER, sProgIDPath.c_str())) != 0 ){ return errorCode; }
		}
	}

	wstring sShellPath = sProgIDPath + L"\\shell";
	wstring sShellActionPath = sShellPath + L"\\" + ACTION_NAME;
	wstring sShellActionCommandPath = sShellActionPath + L"\\command";
	wstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	CRegKey keyShellActionCommand;
	if( (errorCode = keyShellActionCommand.Open(HKEY_CURRENT_USER, sShellActionCommandPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		if( (errorCode = keyShellActionCommand.Create(HKEY_CURRENT_USER, sShellActionCommandPath.c_str())) != 0 ){ return errorCode; }
	}

	WCHAR sExePath[_MAX_PATH] = {0};
	::GetModuleFileName( NULL, sExePath, _countof(sExePath) );
	wstring sCommandPathArg = wstring() + L"\"" + sExePath + L"\" \"%1\"";
	if( (errorCode = keyShellActionCommand.SetValue(NULL, sCommandPathArg.c_str())) != 0 ){ return errorCode; }

	CRegKey keyShellAction;
	if( (errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) !=0 ){ return errorCode; }
	if( (errorCode = keyShellAction.SetValue(NULL, L"Sakura &Editor")) != 0 ){ return errorCode; }

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0 ){ return errorCode; }
	WCHAR szShellValue[ BUFFER_SIZE ] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if(bDefProg)
	{
		if( wcscmp(szShellValue, ACTION_NAME) != 0 )
		{
			if( szShellValue[0] != '\0')
			{
				CRegKey keyBackup;
				if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
				{
					if( (errorCode = keyBackup.Create(HKEY_CURRENT_USER, sBackupPath.c_str())) != 0 ){ return errorCode; }
				}
				keyBackup.SetValue(NULL, szShellValue);
			}
			keyShell.SetValue(NULL, ACTION_NAME);
		}
	}
	else
	{
		CRegKey keyBackup;
		if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
		{
			keyShell.DeleteValue(L"");
		}
		else
		{
			WCHAR sBackupValue[ BUFFER_SIZE ] = {0};
			keyBackup.GetValue(NULL, sBackupValue, _countof(sBackupValue));
			keyShell.SetValue(NULL, sBackupValue);
		}
	}

	return ERROR_SUCCESS;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を削除する
	@param sExt	[in]拡張子
	処理の流れ
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ終了
	・ProgID <- [HKCU\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の存在チェック
		存在しなければ終了
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor\ShellBackup]の存在チェック
		存在すれば、退避した値を[HKCU\Software\Classes\(ProgID)\shell]に設定
		存在しなければ、[HKCU\Software\Classes\(ProgID)\shell]の値を削除
	・ProgIDの先頭が"SakuraEditor_"か？
		そうなら[HKCU\Software\Classes\(ProgID)]と[HKCU\Software\Classes\.(拡張子)]を削除
	　　そうでなければ[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]を削除
*/
int UnregistExt(LPCWSTR sExt)
{
	int errorCode = ERROR_SUCCESS;
	wstring sBasePath = wstring( L"Software\\Classes\\" );

	//小文字化
	WCHAR szLowerExt[MAX_PATH] = {0};
	wcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, wcslen(sExt));
	CharLower(szLowerExt);

	wstring sDotExt = sBasePath + L"." + szLowerExt;
	wstring sGenProgID = wstring() + szLowerExt + L"file";

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ | KEY_WRITE)) != 0)
	{
		return errorCode;
	}

	WCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if( szProgID[0] == L'\0' )
	{
		return ERROR_SUCCESS;
	}

	wstring sProgIDPath = sBasePath + szProgID;
	wstring sShellPath = sProgIDPath + L"\\shell";
	wstring sShellActionPath = sShellPath + L"\\" + ACTION_NAME;
	wstring sShellActionCommandPath = sShellActionPath + L"\\command";
	wstring sBackupPath = sShellActionPath + ACTION_BACKUP_PATH;

	CRegKey keyShellAction;
	if( (errorCode = keyShellAction.Open(HKEY_CURRENT_USER, sShellActionPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		return ERROR_SUCCESS;
	}

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ | KEY_WRITE)) != 0 ){ return errorCode; }
	CRegKey keyBackup;
	if( (errorCode = keyBackup.Open(HKEY_CURRENT_USER, sBackupPath.c_str(), KEY_READ | KEY_WRITE)) != 0 )
	{
		keyShell.DeleteValue(L"");
	}
	else
	{
		WCHAR szBackupValue[ BUFFER_SIZE ] = {0};
		keyBackup.GetValue(NULL, szBackupValue, _countof(szBackupValue));
		keyShell.SetValue(NULL, szBackupValue);
	}

	keyBackup.Close();
	keyShellAction.Close();
	if( wcsncmp(szProgID, L"SakuraEditor_", 13) == 0)
	{
		if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sProgIDPath)) != 0 ){ return errorCode; }

		WCHAR szBackupValue[ BUFFER_SIZE ] = {0};
		keyExt.GetValue(PROGID_BACKUP_NAME, szBackupValue, _countof(szBackupValue));
		if( szBackupValue[0] != L'\0' ){
			keyExt.SetValue(NULL, szBackupValue);
		}else{
			if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sDotExt)) != 0 ){ return errorCode; }
		}
	}else{
		if( (errorCode = DeleteRegistry(HKEY_CURRENT_USER, sShellActionPath)) != 0 ){ return errorCode; }
	}

	return ERROR_SUCCESS;
}

/*!
	@brief 拡張子ごとの関連付けレジストリ設定を確認する
	@param sExt			[in]拡張子
	@param pbRMenu		[out]関連付け設定
	@param pbDblClick	[out]既定設定
	処理の流れ
	・pbRMenu <- false, pbDblClick <- false
	・[HKCU\Software\Classes\.(拡張子)]の存在チェック
		存在しなければ終了
	・ProgID <- [HKCU\Software\Classes\.(拡張子)]の値
	・[HKCU\Software\Classes\(ProgID)\shell\SakuraEditor]の存在チェック
		存在しなければ終了
	・pbRMenu <- true
	・[HKCU\Software\Classes\(ProgID)\shell]の値をチェック
		「SakuraEditor」なら、pbDblClick <- true
*/
int CheckExt(LPCWSTR sExt, bool *pbRMenu, bool *pbDblClick)
{
	int errorCode = ERROR_SUCCESS;
	wstring sBasePath = wstring( L"Software\\Classes\\" );

	*pbRMenu = false;
	*pbDblClick = false;

	//小文字化
	WCHAR szLowerExt[MAX_PATH] = {0};
	wcsncpy_s(szLowerExt, sizeof(szLowerExt) / sizeof(szLowerExt[0]), sExt, wcslen(sExt));
	CharLower(szLowerExt);

	wstring sDotExt = sBasePath + L"." + szLowerExt;
	wstring sGenProgID = wstring() + szLowerExt + L"file";

	CRegKey keyExt;
	if((errorCode = keyExt.Open(HKEY_CURRENT_USER, sDotExt.c_str(), KEY_READ)) != 0)
	{
		return ERROR_SUCCESS;
	}

	WCHAR szProgID[ BUFFER_SIZE ] = {0};
	keyExt.GetValue(NULL, szProgID, _countof(szProgID));

	if(szProgID[0] == L'\0')
	{
		return ERROR_SUCCESS;
	}

	wstring sShellPath = wstring() + L"Software\\Classes\\" + szProgID + L"\\shell";
	wstring sShellActionPath = sShellPath + L"\\" + ACTION_NAME;
	if( ! CRegKey::ExistsKey(HKEY_CURRENT_USER, sShellActionPath.c_str()) )
	{
		return ERROR_SUCCESS;
	}
	*pbRMenu = true;

	CRegKey keyShell;
	if( (errorCode = keyShell.Open(HKEY_CURRENT_USER, sShellPath.c_str(), KEY_READ)) != 0 ){ return errorCode; }
	WCHAR szShellValue[ BUFFER_SIZE ] = {0};
	keyShell.GetValue(NULL, szShellValue, _countof(szShellValue));
	if( wcscmp( szShellValue, ACTION_NAME ) == 0 )
	{
		*pbDblClick = true;
	}

	return ERROR_SUCCESS;
}

/*!
	@brief レジストリ変更の警告メッセージを表示する
*/
bool CDlgTypeList::AlertFileAssociation()
{
	if( m_bAlertFileAssociation ){
		if( IDYES == ::MYMESSAGEBOX( 
						NULL, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
						GSTR_APPNAME,
						L"Windowsの関連付け設定を変更しようとしています。\nこの操作は同じ設定を利用する他のソフトにも影響を与える可能性があります。\n実施しますか？")
					)
		{
			m_bAlertFileAssociation = false;	//「はい」なら最初の一度だけ確認する
			return true;
		}else{
			return false;
		}
	}
	return true;
}