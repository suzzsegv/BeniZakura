/*!	@file
	@brief 編集ウィンドウの管理

	編集ウィンドウ (エディタプロセス) の起動・終了・切り替え

	@date 2026/11/14 CControlTray.cpp から分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, Stonee, jepro, genta, aroka, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, YAZAKI, towest
	Copyright (C) 2003, MIK, Moca, KEITA, genta, aroka
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"

#include "CEditorManager.h"
#include "CEditApp.h"
#include "window/CEditWnd.h"
#include "io/CTextStream.h"
#include "util/module.h"
#include "util/window.h"
#include "env/CShareData.h"
#include "env/SessionStore.h"
#include "doc/CDocListener.h" // SLoadInfo,EditInfo
#include "dlg/CDlgGrep.h"
#include "sakura_rc.h"

void CEditorManager::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, CDlgGrep& cDlgGrep)
{

	/*======= Grepの実行 =============*/
	/* Grep結果ウィンドウの表示 */

	CNativeW		cmWork1;
	CNativeW		cmWork2;
	CNativeW		cmWork3;
	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrep.m_szFile );
	cmWork3.SetString( cDlgGrep.m_szFolder );
	cmWork1.Replace( L"\"", L"\"\"" );
	cmWork2.Replace( L"\"", L"\"\"" );
	cmWork3.Replace( L"\"", L"\"\"" );

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	CNativeW cCmdLine;
	WCHAR szTemp[20];
	cCmdLine.AppendString(L"-GREPMODE -GKEY=\"");
	cCmdLine.AppendStringW(cmWork1.GetStringPtr());
	cCmdLine.AppendString(L"\" -GFILE=\"");
	cCmdLine.AppendString(cmWork2.GetStringPtr());
	cCmdLine.AppendString(L"\" -GFOLDER=\"");
	cCmdLine.AppendString(cmWork3.GetStringPtr());
	cCmdLine.AppendString(L"\" -GCODE=");
	auto_sprintf( szTemp, L"%d", cDlgGrep.m_nGrepCharSet );
	cCmdLine.AppendString(szTemp);

	//GOPTオプション
	WCHAR pOpt[64] = L"";
	if( cDlgGrep.m_bSubFolder					)wcscat( pOpt, L"S" );	// サブフォルダからも検索する
	if( cDlgGrep.m_bUseGitIgnore				)wcscat( pOpt, L"G" );	// .gitignore の除外指定に従う
	if( cDlgGrep.m_bUseHgIgnore				)wcscat( pOpt, L"H" );	// .hgignore の除外指定に従う
	if( cDlgGrep.m_sSearchOption.bLoHiCase		)wcscat( pOpt, L"L" );	// 英大文字と英小文字を区別する
	if( cDlgGrep.m_sSearchOption.bRegularExp	)wcscat( pOpt, L"R" );	// 正規表現
	if( cDlgGrep.m_bGrepOutputLine				)wcscat( pOpt, L"P" );	// 行を出力するか該当部分だけ出力するか
	if( cDlgGrep.m_sSearchOption.bWordOnly		)wcscat( pOpt, L"W" );	// 単語単位で探す
	if( 1 == cDlgGrep.m_nGrepOutputStyle		)wcscat( pOpt, L"1" );	// Grep: 出力形式
	if( 2 == cDlgGrep.m_nGrepOutputStyle		)wcscat( pOpt, L"2" );	// Grep: 出力形式
	if( pOpt[0] != L'\0' ){
		cCmdLine.AppendString( L" -GOPT=" );
		cCmdLine.AppendString( pOpt );
	}

	/* 新規編集ウィンドウの追加 ver 0 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = L"";
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	OpenNewEditor( hinst, msgParent, sLoadInfo, cCmdLine.GetStringPtr(),
		false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
}



/*!
	新規編集ウィンドウの追加 ver 0

	@date 2000.10.24 genta WinExec -> CreateProcess．同期機能を付加
	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2003.05.30 genta 外部プロセス起動時のカレントディレクトリ指定を可能に．
	@date 2007.06.26 ryoji 新規編集ウィンドウは hWndParent と同じグループを指定して起動する
	@date 2008.04.19 ryoji MYWM_FIRST_IDLE 待ちを追加
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
bool CEditorManager::OpenNewEditor(
	HINSTANCE			hInstance,			//!< [in] インスタンスID (実は未使用)
	HWND				hWndParent,			//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
	const SLoadInfo&	sLoadInfo,			//!< [in]
	const WCHAR*		szCmdLineOption,	//!< [in] 追加のコマンドラインオプション
	bool				sync,				//!< [in] trueなら新規エディタの起動まで待機する
	const WCHAR*		szCurDir,			//!< [in] 新規エディタのカレントディレクトリ
	bool				bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	/* 共有データ構造体のアドレスを返す */
	DLLSHAREDATA*	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, L"編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
		return false;
	}

	// -- -- -- -- コマンドライン文字列を生成 -- -- -- -- //
	CCommandLineString cCmdLineBuf;

	//アプリケーションパス
	WCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName( NULL, szEXE, _countof( szEXE ) );
	cCmdLineBuf.AppendF( L"\"%ts\"", szEXE );

	// ファイル名
	if( sLoadInfo.cFilePath.c_str()[0] != L'\0' )	cCmdLineBuf.AppendF( L" \"%ts\"", sLoadInfo.cFilePath.c_str() );

	// コード指定
	if( IsValidCodeType(sLoadInfo.eCharCode) )cCmdLineBuf.AppendF( L" -CODE=%d", sLoadInfo.eCharCode );

	// ビューモード指定
	if( sLoadInfo.bViewMode )cCmdLineBuf.AppendF( L" -R" );

	// グループID
	if( false == bNewWindow ){	// 新規エディタをウインドウで開く
		// グループIDを親ウィンドウから取得
		HWND hwndAncestor = MyGetAncestor( hWndParent, GA_ROOTOWNER2 );	// 2007.10.22 ryoji GA_ROOTOWNER -> GA_ROOTOWNER2
		int nGroup = CAppNodeManager::getInstance()->GetEditNode( hwndAncestor )->GetGroup();
		if( nGroup > 0 ){
			cCmdLineBuf.AppendF( L" -GROUP=%d", nGroup );
		}
	}else{
		// 空いているグループIDを使用する
		cCmdLineBuf.AppendF( L" -GROUP=%d", CAppNodeManager::getInstance()->GetFreeGroupId() );
	}

	// 追加のコマンドラインオプション
	WCHAR szResponseFile[_MAX_PATH] = L"";
	struct CResponsefileDeleter{
		LPCWSTR fileName;
		CResponsefileDeleter(): fileName(NULL){}
		~CResponsefileDeleter(){
			if( fileName && fileName[0] ){
				::DeleteFile( fileName );
				fileName = NULL;
			}
		}
	};
	CResponsefileDeleter respDeleter;
	if( szCmdLineOption ){
		// Grepなどで入りきらない場合はレスポンスファイルを利用する
		if( cCmdLineBuf.max_size() < cCmdLineBuf.size() + auto_strlen(szCmdLineOption) ){
			WCHAR szIniDir[_MAX_PATH];
			GetInidir(szIniDir);
			LPWSTR pszTempFile = _wtempnam(szIniDir, L"skr_resp");
			if( !pszTempFile ){
				ErrorMessage(hWndParent, L"レスポンスファイルの作成に失敗しました。");
				return false;
			}
			auto_strcpy(szResponseFile, pszTempFile);
			free(pszTempFile);
			CTextOutputStream output(szResponseFile);
			if( !output ){
				ErrorMessage(hWndParent, L"レスポンスファイルの作成に失敗しました。");
				return false;
			}
			respDeleter.fileName = szResponseFile;
			// 出力
			output.WriteString(to_wchar(szCmdLineOption));
			output.Close();
			sync = true;
			cCmdLineBuf.AppendF(L" -@=\"%ts\"", szResponseFile);
		}else{
			cCmdLineBuf.AppendF(L" %ts", szCmdLineOption);
		}
	}
	// -- -- -- -- プロセス生成 -- -- -- -- //

	//	プロセスの起動
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof_raw( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	//	May 30, 2003 genta カレントディレクトリ指定を可能に
	//エディタプロセスを起動
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS; //2007.09.22 kobake デバッグ用フラグ
#endif
	WCHAR szCmdLine[1024]; wcscpy_s(szCmdLine, _countof(szCmdLine), cCmdLineBuf.c_str());
	BOOL bCreateResult = CreateProcess(
		szEXE,					// 実行可能モジュールの名前
		szCmdLine,				// コマンドラインの文字列
		NULL,					// セキュリティ記述子
		NULL,					// セキュリティ記述子
		FALSE,					// ハンドルの継承オプション
		dwCreationFlag,			// 作成のフラグ
		NULL,					// 新しい環境ブロック
		szCurDir,				// カレントディレクトリの名前
		&s,						// スタートアップ情報
		&p						// プロセス情報
	);
	if( !bCreateResult ){
		//	失敗
		WCHAR* pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			L"\'%ts\'\nプロセスの起動に失敗しました。\n%ts",
			szEXE,
			pMsg
		);
		::LocalFree( (HLOCAL)pMsg );	//	エラーメッセージバッファを解放
		return false;
	}

	bool bRet = true;
	if( sync ){
		//	起動したプロセスが完全に立ち上がるまで一定時間待つ．
		int nResult = WaitForInputIdle( p.hProcess, 60 * 1000 );	//	最大60秒間待つ
		if( nResult != 0 ){
			ErrorMessage(
				hWndParent,
				L"\'%ts\'\nプロセスの起動に失敗しました。",
				szEXE
			);
			bRet = false;
		}
	}
	else{
		// タブまとめ時は起動したプロセスが立ち上がるまでしばらくタイトルバーをアクティブに保つ	// 2007.02.03 ryoji
		if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
			WaitForInputIdle( p.hProcess, 3000 );
			sync = true;
		}
	}

	// MYWM_FIRST_IDLE が届くまでちょっとだけ余分に待つ	// 2008.04.19 ryoji
	// Note. 起動先プロセスが初期化処理中に COM 関数（SHGetFileInfo API なども含む）を実行すると、
	//       その時点で COM の同期機構が動いて WaitForInputIdle は終了してしまう可能性がある（らしい）。
	if( sync && bRet )
	{
		int i;
		for( i = 0; i < 200; i++ ){
			MSG msg;
			DWORD dwExitCode;
			if( ::PeekMessage( &msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE ) ){
				if( msg.message == WM_QUIT ){	// 指定範囲外でも WM_QUIT は取り出される
					::PostQuitMessage( msg.wParam );
					break;
				}
				// 監視対象プロセスからのメッセージなら抜ける
				// そうでなければ破棄して次を取り出す
				if( msg.wParam == p.dwProcessId ){
					break;
				}
			}
			if( ::GetExitCodeProcess( p.hProcess, &dwExitCode ) && dwExitCode != STILL_ACTIVE ){
				break;	// 監視対象プロセスが終了した
			}
			::Sleep(10);
		}
	}

	CloseHandle( p.hThread );
	CloseHandle( p.hProcess );

	return bRet;
}


/*!	新規編集ウィンドウの追加 ver 2:

	@date Oct. 24, 2000 genta create.
	@date Feb. 25, 2012 novice -CODE/-RはOpenNewEditor側で処理するので削除
*/
bool CEditorManager::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo*	pfi,
	bool			bViewMode,
	bool			sync,
	bool			bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	DLLSHAREDATA*	pShareData;

	/* 共有データ構造体のアドレスを返す */
	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, L"編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。", MAX_EDITWINDOWS );
		return false;
	}

	// 追加のコマンドラインオプション
	CCommandLineString cCmdLine;
	if( pfi != NULL ){
		if( pfi->m_ptCursor.x >= 0					)cCmdLine.AppendF( L" -X=%d", pfi->m_ptCursor.x +1 );
		if( pfi->m_ptCursor.y >= 0					)cCmdLine.AppendF( L" -Y=%d", pfi->m_ptCursor.y +1 );
		if( pfi->m_nViewLeftCol >= CLayoutInt(0)	)cCmdLine.AppendF( L" -VX=%d", (Int)pfi->m_nViewLeftCol + 1 );
		if( pfi->m_nViewTopLine >= CLayoutInt(0)	)cCmdLine.AppendF( L" -VY=%d", (Int)pfi->m_nViewTopLine + 1 );
	}
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = pfi ? pfi->m_szPath : L"";
	sLoadInfo.eCharCode = pfi ? pfi->m_nCharCode : CODE_NONE;
	sLoadInfo.bViewMode = bViewMode;
	return OpenNewEditor( hInstance, hWndParent, sLoadInfo, cCmdLine.c_str(), sync, NULL, bNewWindow );
}
//	To Here Oct. 24, 2000 genta



void CEditorManager::ActiveNextWindow()
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( CEditWnd::getInstance()->GetHwnd() == pEditNodeArr[i].GetHwnd() )
			{
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 前のウィンドウ
			int		j;
			for( j = i - 1; j >= 0; --j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j < 0 ){
				for( j = nRowNum - 1; j > i; --j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 前のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最後のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
		}
		delete [] pEditNodeArr;
	}
}

void CEditorManager::ActivePrevWindow()
{
	/* 現在開いている編集窓のリストを得る */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if(  nRowNum > 0 ){
		/* 自分のウィンドウを調べる */
		int				nGroup = 0;
		int				i;
		for( i = 0; i < nRowNum; ++i ){
			if( CEditWnd::getInstance()->GetHwnd() == pEditNodeArr[i].GetHwnd() ){
				nGroup = pEditNodeArr[i].m_nGroup;
				break;
			}
		}
		if( i < nRowNum ){
			// 次のウィンドウ
			int		j;
			for( j = i + 1; j < nRowNum; ++j ){
				if( nGroup == pEditNodeArr[j].m_nGroup )
					break;
			}
			if( j >= nRowNum ){
				for( j = 0; j < i; ++j ){
					if( nGroup == pEditNodeArr[j].m_nGroup )
						break;
				}
			}
			/* 次のウィンドウをアクティブにする */
			HWND	hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow( hwndWork );
			/* 最初のペインをアクティブにする */
			::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
		}
		delete [] pEditNodeArr;
	}
}



/*!	サクラエディタの全終了

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
*/
void CEditorManager::TerminateApplication(
	HWND hWndFrom	//!< [in] 呼び出し元のウィンドウハンドル
)
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();	/* 共有データ構造体のアドレスを返す */

	/* 現在の編集ウィンドウの数を調べる */
	if( pShareData->m_Common.m_sGeneral.m_bExitConfirm ){	//終了時の確認
		if( 0 < CAppNodeGroupHandle(0).GetEditorWindowsNum() ){
			if( IDYES != ::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				L"現在開いている編集用のウィンドウをすべて閉じて終了しますか?"
			) ){
				return;
			}
		}
	}
	/* 「すべてのウィンドウを閉じる」要求 */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	BOOL bCheckConfirm = (pShareData->m_Common.m_sGeneral.m_bExitConfirm)? FALSE: TRUE;	// 2006.12.25 ryoji 終了確認済みならそれ以上は確認しない
	if( CloseAllEditor( bCheckConfirm, hWndFrom, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
		::PostMessageAny( pShareData->m_sHandles.m_hwndControlProcessWnd, WM_CLOSE, 0, 0 );
	}
	return;
}




/*!	すべてのウィンドウを閉じる

	@date Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2006.12.25 ryoji 複数の編集ウィンドウを閉じるときの確認（引数追加）
	@date 2007.02.13 ryoji 「編集の全終了」を示す引数(bExit)を追加
	@date 2007.06.20 ryoji nGroup引数を追加
*/
BOOL CEditorManager::CloseAllEditor(
	BOOL	bCheckConfirm,	//!< [in] [すべて閉じる]確認オプションに従って問い合わせをするかどうか
	HWND	hWndFrom,		//!< [in] 呼び出し元のウィンドウハンドル
	BOOL	bExit,			//!< [in] TRUE: 編集の全終了 / FALSE: すべて閉じる
	int		nGroup			//!< [in] グループID
)
{
	EditNode*	pWndArr;
	int		n;

	n = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pWndArr, FALSE );
	if( 0 == n ){
		return TRUE;
	}

	int groupNum = CAppNodeManager::getInstance()->GetGroupNumber();
	if (groupNum == 1) {
		SessionStore::getInstance()->save();
	}

	/* 全編集ウィンドウへ終了要求を出す */
	BOOL	bRes = CAppNodeGroupHandle(nGroup).RequestCloseEditor( pWndArr, n, bExit, bCheckConfirm, hWndFrom );	// 2007.02.13 ryoji bExitを引き継ぐ
	delete []pWndArr;
	return bRes;
}




