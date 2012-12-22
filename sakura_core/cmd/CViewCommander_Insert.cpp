/*!	@file
@brief CViewCommanderクラスのコマンド(挿入系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
#include "dlg/CDlgCtrlCode.h"	//コントロールコードの入力(ダイアログ)


//日付挿入
void CViewCommander::Command_INS_DATE( void )
{
	// 日付をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
}



//時刻挿入
void CViewCommander::Command_INS_TIME( void )
{
	// 時刻をフォーマット
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );

	// テキストを貼り付け ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
}



//	from CViewCommander_New.cpp
/*!	コントロールコードの入力(ダイアログ)
	@author	MIK
	@date	2002/06/02
*/
void CViewCommander::Command_CtrlCode_Dialog( void )
{
	CDlgCtrlCode	cDlgCtrlCode;

	//コントロールコード入力ダイアログを表示する
	if( cDlgCtrlCode.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() ) )
	{
		//コントロールコードを入力する
		Command_WCHAR( cDlgCtrlCode.GetCharCode() );
	}
}
