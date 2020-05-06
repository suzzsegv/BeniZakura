/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date	1998/3/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta, jepro
	Copyright (C) 2002, MIK, genta, aroka
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/CDlgAbout.h"
#include "uiparts/HandCursor.h"
#include "util/file.h"
#include "util/module.h"
#include "gitrev.h"
#include "sakura_rc.h" // 2002/2/10 aroka 復帰


//	From Here Feb. 7, 2002 genta
// 2006.01.17 Moca COMPILER_VERを追加
// 2010.04.15 Moca icc/dmcを追加しCPUを分離
#if defined(_M_IA64)
#  define TARGET_M_SUFFIX "_I64"
#elif defined(_M_AMD64)
#  define TARGET_M_SUFFIX "_A64"
#else
#  define TARGET_M_SUFFIX ""
#endif

#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__INTEL_COMPILER)
#  define COMPILER_TYPE "I"
#  define COMPILER_VER __INTEL_COMPILER
#elif defined(__DMC__)
#  define COMPILER_TYPE "D"
#  define COMPILER_VER __DMC__
#elif defined(_MSC_VER)
#  define COMPILER_TYPE "V"
#  define COMPILER_VER _MSC_VER
#else
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
#endif
//	To Here Feb. 7, 2002 genta

#ifdef _UNICODE
	#define TARGET_STRING_MODEL "W"
#else
	#define TARGET_STRING_MODEL "A"
#endif

#ifdef _DEBUG
	#define TARGET_DEBUG_MODE "D"
#else
	#define TARGET_DEBUG_MODE "R"
#endif

#define TSTR_TARGET_MODE _T(TARGET_STRING_MODEL) _T(TARGET_DEBUG_MODE)

#ifdef _WIN32_WINDOWS
	#define MY_WIN32_WINDOWS _WIN32_WINDOWS
#else
	#define MY_WIN32_WINDOWS 0
#endif

#ifdef _WIN32_WINNT
	#define MY_WIN32_WINNT _WIN32_WINNT
#else
	#define MY_WIN32_WINNT 0
#endif

//	From Here Nov. 7, 2000 genta
/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR CDlgAbout::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		// EDITも READONLY か DISABLEの場合 WM_CTLCOLORSTATIC になります
		if( (HWND)lParam == GetDlgItem(hWnd, IDC_EDIT_ABOUT) ){
			::SetTextColor( (HDC)wParam, RGB( 102, 102, 102 ) );
		} else {
			::SetTextColor( (HDC)wParam, RGB( 0, 0, 0 ) );
        }
		return (INT_PTR)GetStockObject( WHITE_BRUSH );
	}
	return result;
}
//	To Here Nov. 7, 2000 genta

/* モーダルダイアログの表示 */
int CDlgAbout::DoModal( HINSTANCE hInstance, HWND hwndParent )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_ABOUT, NULL );
}

/*! 初期化処理
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
BOOL CDlgAbout::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	TCHAR			szMsg[2048];
	wchar_t	vcVersion[64];

	/* バージョン情報 */

	CNativeT cmemMsg;
	cmemMsg.AppendString(_T("\r\n紅桜  "));

	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );

	auto_sprintf(szMsg, _T("%s\r\n"), _T(GIT_REV));
	cmemMsg.AppendString( szMsg );
	cmemMsg.AppendString( _T("\r\n") );

	switch(_MSC_VER){
	case 1500:
		wcscpy( vcVersion, L"Visual Studio 2008" );
		break;
	case 1600:
		wcscpy( vcVersion, L"Visual Studio 2010" );
		break;
	case 1700:
		wcscpy( vcVersion, L"Visual Studio 2012" );
		break;
	case 1800:
		wcscpy( vcVersion, L"Visual Studio 2013" );
		break;
	case 1900:
		wcscpy( vcVersion, L"Visual Studio 2015" );
		break;
	default:
		auto_sprintf(vcVersion, L"MSC Ver.%d.%02d", _MSC_VER / 100, _MSC_VER % 100);
		break;
	}
	auto_sprintf( szMsg, L"  %s %s  Built with %s\r\n", _T(__DATE__), _T(__TIME__), vcVersion );
	cmemMsg.AppendString( szMsg );
	cmemMsg.AppendString( _T("\r\n") );
	cmemMsg.AppendString( _T("  Copyright (C) 2012-2020 by Satoshi Suzuki\r\n") );
	cmemMsg.AppendString( _T("\r\n") );
	cmemMsg.AppendString( _T("\r\n") );

	cmemMsg.AppendString( _T("Based on サクラエディタ  Ver.2.0.8.1+\r\n") );
	cmemMsg.AppendString( _T("\r\n") );
	cmemMsg.AppendString( _T("  Copyright (C) 1998-2014 by Norio Nakatani & Collaborators\r\n") );
	cmemMsg.AppendString( _T("\r\n") );

// パッチ(かリビジョン)の情報をコンパイル時に渡せるようにする
#ifdef SKR_PATCH_INFO
	cmemMsg.AppendString( _T("      ") );
	const TCHAR* ptszPatchInfo = to_tchar(SKR_PATCH_INFO);
	int patchInfoLen = auto_strlen(ptszPatchInfo);
	cmemMsg.AppendString( ptszPatchInfo, t_min(80, patchInfoLen) );
#endif
	cmemMsg.AppendString( _T("\r\n"));

	::DlgItem_SetText( GetHwnd(), IDC_EDIT_VER, cmemMsg.GetStringPtr() );

	//	From Here Jun. 8, 2001 genta
	//	Edit Boxにメッセージを追加する．
	int desclen = ::LoadString( m_hInstance, IDS_ABOUT_DESCRIPTION, szMsg, _countof( szMsg ) );
	if( desclen > 0 ){
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_ABOUT, szMsg );
	}
	//	To Here Jun. 8, 2001 genta

	//	From Here Dec. 2, 2002 genta
	//	アイコンをカスタマイズアイコンに合わせる
	HICON hIcon = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false );
	HWND hIconWnd = GetDlgItem( GetHwnd(), IDC_STATIC_MYICON );

	if( hIconWnd != NULL && hIcon != NULL ){
		StCtl_SetIcon( hIconWnd, hIcon );
	}
	//	To Here Dec. 2, 2002 genta

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgAbout::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_COPY:
		{
			HWND hwndEditVer = GetDlgItem( GetHwnd(), IDC_EDIT_VER );
	 		EditCtl_SetSel( hwndEditVer, 0, -1); 
	 		SendMessage( hwndEditVer, WM_COPY, 0, 0 );
	 		EditCtl_SetSel( hwndEditVer, -1, 0); 
 		}
		return TRUE;
	}
	return CDialog::OnBnClicked( wID );
}


