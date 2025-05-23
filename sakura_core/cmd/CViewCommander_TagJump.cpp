/*!	@file
@brief CViewCommanderクラスのコマンド(タグジャンプ)関数群

	2012/12/17	CViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, MIK
	Copyright (C) 2003, MIK, genta, じゅうじ
	Copyright (C) 2004, Moca, novice
	Copyright (C) 2005, MIK
	Copyright (C) 2006, genta
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, MIK
	Copyright (C) 2010, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "uiparts/CWaitCursor.h"
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "CEditApp.h"
#include "_os/COsVersionInfo.h"
#include "util/window.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "env/CSakuraEnvironment.h"
#include "CGrepAgent.h"
#include "sakura_rc.h"


/*! タグジャンプ

	@param bClose [in] true:元ウィンドウを閉じる

	@date 2003.04.03 genta 元ウィンドウを閉じるかどうかの引数を追加
	@date 2004.05.13 Moca 行桁位置の指定が無い場合は、行桁を移動しない
*/
bool CViewCommander::Command_TAGJUMP( bool bClose )
{
	//	2004.05.13 Moca 初期値を1ではなく元の位置を継承するように
	// 0以下は未指定扱い。(1開始)
	int			nJumpToLine;
	int			nJumpToColumn;
	nJumpToLine = 0;
	nJumpToColumn = 0;

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint ptXY;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&ptXY
	);
	/* 現在行のデータを取得 */
	CLogicInt		nLineLen;
	const wchar_t*	pLine;
	pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}

	//ファイル名バッファ
	wchar_t		szJumpToFile[1024];
	int			nBgn;
	int			nPathLen;
	wmemset( szJumpToFile, 0, _countof(szJumpToFile) );

	/* WZ風のタグリストか */
	if( 0 == wmemcmp( pLine, L"■\"", 2 ) ){
		if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
			wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
			GetLineColumn( &pLine[2] + nPathLen, &nJumpToLine, &nJumpToColumn );
		}
		else{
			goto can_not_tagjump;
		}
	}
	else if(0 == wmemcmp( pLine, L"・", 1 )){
		GetLineColumn( &pLine[1], &nJumpToLine, &nJumpToColumn );
		ptXY.y--;

		for( ; 0 <= ptXY.y; ptXY.y-- ){
			pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
			if( NULL == pLine ){
				goto can_not_tagjump;
			}
			if( 0 == wmemcmp( pLine, L"・", 1 ) ){
				continue;
			}
			else if( 0 == wmemcmp( pLine, L"■\"", 2 ) ){
				if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
					wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
					break;
				}
				else{
					goto can_not_tagjump;
				}
			}
			else{
				goto can_not_tagjump;
			}
		}
	}
	else{
		//@@@ 2001.12.31 YAZAKI
		const wchar_t *p = pLine;
		const wchar_t *p_end = p + nLineLen;

		//	From Here Aug. 27, 2001 genta
		//	Borland 形式のメッセージからのTAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == L' ' || *p == L'\t' || *p == L'\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				wmemcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColumn( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColumn );
				break;
			}
			//	Jan. 04, 2001 genta Directoryを対象外にしたので文字列には柔軟に対応
			//	break;	//@@@ 2001.12.31 YAZAKI 「working ...」問題に対処
			//	skip non-space
			for( ; p < p_end && ( *p != L' ' && *p != L'\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == L'\0' ){
			if( !Command_TagJumpByTagsFile(bClose) )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose追加
	if( !m_pCommanderView->TagJumpSub( to_tchar(szJumpToFile), CMyPoint(nJumpToColumn, nJumpToLine), bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
	m_pCommanderView->SendStatusMessage(_T("タグジャンプできません"));	//@@@ 2003.04.13
	return false;
}



/* タグジャンプバック */
void CViewCommander::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice タグジャンプ機能追加
	TagJump tagJump;

	/* タグジャンプ情報の参照 */
	if( !CTagJumpManager().PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer) ){
		m_pCommanderView->SendStatusMessage(_T("タグジャンプバックできません"));
		// 2004.07.10 Moca m_TagJumpNumを0にしなくてもいいと思う
		// GetDllShareData().m_TagJumpNum = 0;
		return;
	}

	/* アクティブにする */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* カーソルを移動させる */
	memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &(tagJump.point), sizeof( tagJump.point ) );
	::SendMessageAny( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

	return;
}



/*
	タグファイルを作成する。

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	ダイアログ表示でフォルダ等を細かく指定できるようにした。
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CViewCommander::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() )
	{
		TCHAR vcsDirectoryPathName[MAX_PATH];
		bool vcsRepositoryDetected;

		_tcscpy( szTargetPath, GetDocument()->m_cDocFile.GetFilePath() );
		szTargetPath[ _tcslen( szTargetPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');

		vcsRepositoryDetected = GetVcsRepositoryRootDir( vcsDirectoryPathName, szTargetPath );
		if( vcsRepositoryDetected == true ){
			wcscpy( szTargetPath, vcsDirectoryPathName );
		}
	}
	else
	{
		// 20100722 Moca サクラのフォルダからカレントディレクトリに変更
		::GetCurrentDirectory( _countof(szTargetPath), szTargetPath );
	}

	//ダイアログを表示する
	CDlgTagsMake	cDlgTagsMake;
	if( !cDlgTagsMake.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0, szTargetPath ) ) return false;

	TCHAR	cmdline[1024];
	/* exeのあるフォルダ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, CTAGS_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//ctags.exeの存在チェック
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T( "タグ作成コマンド実行は失敗しました。\n\nCTAGS.EXE が見つかりません。" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_pCommanderView->GetHwnd() );

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(sa) );
	sa.nLength              = sizeof(sa);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hStdOutRead = hStdOutWrite = 0;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE )
	{
		//エラー
		return false;
	}

	//継承不能にする
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//	To Here Dec. 28, 2002 MIK

	TCHAR	options[1024];
	_tcscpy( options, _T("--excmd=n --extras=+f") );	//デフォルトのオプション
	if( cDlgTagsMake.m_nTagsOpt & 0x0001 ) _tcscat( options, _T(" -R") );	//サブフォルダも対象
	if( cDlgTagsMake.m_nTagsOpt & 0x0002 ) _tcscat( options, _T(" --kinds-C=+px") );
	if( cDlgTagsMake.m_nTagsOpt & 0x0004 ) _tcscat( options, _T(" --input-encoding=cp932") );
	if( cDlgTagsMake.m_szTagsCmdLine[0] != _T('\0') )	//個別指定のコマンドライン
	{
		_tcscat( options, _T(" ") );
		_tcscat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	_tcscat( options, _T(" *") );	//配下のすべてのファイル

	//コマンドライン文字列作成(MAX:1024)
	if (IsWin32NT())
	{
		// 2010.08.28 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		//	2006.08.04 genta add /D to disable autorun
		auto_sprintf( cmdline, _T("\"%ts\\cmd.exe\" /D /C \"\"%ts\\%ts\" %ts\""),
				szCmdDir,
				szExeFolder,	//sakura.exeパス
				CTAGS_COMMAND,	//ctags.exe
				options			//ctagsオプション
			);
	}
	else
	{
		// 2010.08.28 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		auto_sprintf( cmdline, _T("\"%ts\\command.com\" /C \"%ts\\%ts\" %ts"),
				szCmdDir,
				szExeFolder,	//sakura.exeパス
				CTAGS_COMMAND,	//ctags.exe
				options			//ctagsオプション
			);
	}

	//コマンドライン実行
	BOOL bProcessResult = CreateProcess(
		NULL, cmdline, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, cDlgTagsMake.m_szPath, &sui, &pi
	);
	if( !bProcessResult )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T("タグ作成コマンド実行は失敗しました。\n\n%ts"), cmdline );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		bool	bLoopFlag = true;

		//中断ダイアログ表示
		HWND	hwndCancel;
		HWND	hwndMsg;
		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)L"タグファイルを作成中です。" );

		//実行結果の取り込み
		do {
			// Jun. 04, 2003 genta CPU消費を減らすために200msec待つ
			// その間メッセージ処理が滞らないように待ち方をWaitForSingleObjectから
			// MsgWaitForMultipleObjectに変更
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 200, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//終了していればループフラグをFALSEとする
					//ただしループの終了条件は プロセス終了 && パイプが空
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//処理中のユーザー操作を可能にする
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}

			//中断ボタン押下チェック
			if( cDlgCancel.IsCanceled() )
			{
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess( pi.hProcess, 0 );
				break;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//パイプの中の読み出し待機中の文字数を取得
			{
				if( new_cnt > 0 )												//待機中のものがある
				{
					if( new_cnt >= _countof(work) - 2 )							//パイプから読み出す量を調整
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					if( read_cnt == 0 )
					{
						continue;
					}
					// 2003.11.09 じゅうじ
					//	正常終了の時はメッセージが出力されないので
					//	何か出力されたらエラーメッセージと見なす．
					else {
						//終了処理
						CloseHandle( hStdOutWrite );
						CloseHandle( hStdOutRead  );
						if( pi.hProcess ) CloseHandle( pi.hProcess );
						if( pi.hThread  ) CloseHandle( pi.hThread  );

						cDlgCancel.CloseDialog( TRUE );

						work[ read_cnt ] = L'\0';	// Nov. 15, 2003 genta 表示用に0終端する
						WarningMessage( m_pCommanderView->GetHwnd(), _T("タグ作成コマンド実行は失敗しました。\n\n%hs"), work ); // 2003.11.09 じゅうじ

						return true;
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

	}


finish:
	//終了処理
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	cDlgCancel.CloseDialog( TRUE );

	m_pCommanderView->SendStatusMessage(_T("タグファイルの作成が終了しました。"));

	return true;
}



/*
	ダイレクトタグジャンプ(メッセージ付)

	@date	2010.07.22	新規作成
*/
bool CViewCommander::Command_TagJumpByTagsFileMsg( bool bMsg )
{
	bool ret = Command_TagJumpByTagsFile(false);
	if( false == ret && bMsg ){
		m_pCommanderView->SendStatusMessage(_T("タグジャンプできません"));
	}
	return ret;
}



/*
	ダイレクトタグジャンプ

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.05.12	フォルダ階層も考慮して探す
	@date	
*/
bool CViewCommander::Command_TagJumpByTagsFile( bool bClose )
{
	CNativeW	cmemKeyW;
	m_pCommanderView->GetCurrentTextForSearch( cmemKeyW, true, true );
	if( 0 == cmemKeyW.GetStringLength() ){
		return false;
	}
	
	TCHAR	szDirFile[1024];
	if( false == Sub_PreProcTagJumpByTagsFile( szDirFile, _countof(szDirFile) ) ){
		return false;
	}
	CDlgTagJumpList	cDlgTagJumpList(true);	//タグジャンプリスト
	
	cDlgTagJumpList.SetFileName( szDirFile );
	cDlgTagJumpList.SetKeyword(cmemKeyW.GetStringPtr());

	int nMatchAll = cDlgTagJumpList.FindDirectTagJump();

	//複数あれば選択してもらう。
	if( 1 < nMatchAll ){
		if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0 ) ){
			nMatchAll = 0;
			return true;	//キャンセル
		}
	}

	//タグジャンプする。
	if( 0 < nMatchAll ){
		//	@@ 2005.03.31 MIK 階層パラメータ追加
		TCHAR fileName[1024];
		int   fileLine;

		if( false == cDlgTagJumpList.GetSelectedFullPathAndLine( fileName, _countof(fileName), &fileLine , NULL ) ){
			return false;
		}
		return m_pCommanderView->TagJumpSub( fileName, CMyPoint(0, fileLine), bClose );
	}

	return false;
}



/*!
	シンボル検索(キーワード指定タグジャンプ) ダイアログ
	@param keyword NULL許容
	@author MIK
	@date 2005.03.31 新規作成
	@date 2010.04.02 Moca 無題でも使えるように
*/
bool CViewCommander::Command_TagJumpByTagsFileKeyword( const wchar_t* keyword )
{
	CDlgTagJumpList	cDlgTagJumpList(false);
	TCHAR	fileName[1024];
	int		fileLine;	// 行番号
	TCHAR	szCurrentPath[1024];

	if( false == Sub_PreProcTagJumpByTagsFile( szCurrentPath, _countof(szCurrentPath) ) ){
		return false;
	}

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ) 
	{
		return true;	//キャンセル
	}

	//タグジャンプする。
	if( false == cDlgTagJumpList.GetSelectedFullPathAndLine( fileName, _countof(fileName), &fileLine, NULL ) )
	{
		return false;
	}

	return m_pCommanderView->TagJumpSub( fileName, CMyPoint(0, fileLine) );
}



/*!
	タグジャンプの前処理
	実行可能確認と、基準ファイル名の設定
*/
bool CViewCommander::Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, int count )
{
	if( count ) szCurrentPath[0] = _T('\0');

	// 実行可能確認
	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		// 2010.04.02 (無題)でもタグジャンプできるように
		// Grep、アウトプットは行番号タグジャンプがあるので無効にする(要検討)
		if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode ||
		    CAppMode::getInstance()->IsDebugMode() ){
		    return false;
		}
	}
	
	// 基準ファイル名の設定
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		auto_strcpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );
	}else{
		if( 0 == ::GetCurrentDirectory( count - _countof(_T("\\dmy")) - MAX_TYPES_EXTS, szCurrentPath ) ){
			return false;
		}
		// (無題)でもファイル名を要求してくるのでダミーをつける
		// 現在のタイプ別の1番目の拡張子を拝借
		TCHAR szExts[MAX_TYPES_EXTS];
		TCHAR* pszExt = szExts;
		auto_strcpy( szExts, GetDocument()->m_cDocType.GetDocumentAttribute().m_szTypeExts );
		if( szExts[0] != '\0' ){
			// strtok 注意
			pszExt = _tcstok( szExts, _T(" ;,") );
		}
		int nExtLen = 0;
		if( NULL != pszExt ){
			nExtLen = auto_strlen( pszExt );
		}
		_tcscat( szCurrentPath, _T("\\dmy") );
		if( nExtLen ){
			_tcscat( szCurrentPath, _T(".") );
			_tcscat( szCurrentPath, pszExt );
		}
	}
	return true;
}
