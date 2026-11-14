/*!	@file
	@brief 常駐部

	キー割り当て、共有データの保存、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	@date 2026/11/14 タスクトレイのアイコンとメニュー (および、そのホットキー) を削除
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

#include "CControlTray.h"
#include "CEditApp.h"
#include "debug/CRunningTimer.h"
#include "window/CEditWnd.h"		//Nov. 21, 2000 JEPROtest
#include "io/CTextStream.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "env/SessionStore.h"
#include "doc/CDocListener.h" // SLoadInfo,EditInfo
#include "sakura_rc.h"

/////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK CControlTrayWndProc( HWND, UINT, WPARAM, LPARAM );

static CControlTray*	g_m_pCControlTray;

void CControlTray::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, CDlgGrep& cDlgGrep)
{

	/*======= Grepの実行 =============*/
	/* Grep結果ウィンドウの表示 */

	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	cmWork1.SetString( cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrep.m_szFile );
	cmWork3.SetString( cDlgGrep.m_szFolder );
	cmWork1.Replace( L"\"", L"\"\"" );
	cmWork2.Replace( _T("\""), _T("\"\"") );
	cmWork3.Replace( _T("\""), _T("\"\"") );

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	CNativeT cCmdLine;
	TCHAR szTemp[20];
	cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
	cCmdLine.AppendStringW(cmWork1.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GFILE=\""));
	cCmdLine.AppendString(cmWork2.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GFOLDER=\""));
	cCmdLine.AppendString(cmWork3.GetStringPtr());
	cCmdLine.AppendString(_T("\" -GCODE="));
	auto_sprintf( szTemp, _T("%d"), cDlgGrep.m_nGrepCharSet );
	cCmdLine.AppendString(szTemp);

	//GOPTオプション
	TCHAR pOpt[64] = _T("");
	if( cDlgGrep.m_bSubFolder					)_tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
	if( cDlgGrep.m_sSearchOption.bLoHiCase		)_tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
	if( cDlgGrep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// 正規表現
	if( cDlgGrep.m_bGrepOutputLine				)_tcscat( pOpt, _T("P") );	// 行を出力するか該当部分だけ出力するか
	if( cDlgGrep.m_sSearchOption.bWordOnly		)_tcscat( pOpt, _T("W") );	// 単語単位で探す
	if( 1 == cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: 出力形式
	if( 2 == cDlgGrep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: 出力形式
	if( pOpt[0] != _T('\0') ){
		cCmdLine.AppendString( _T(" -GOPT=") );
		cCmdLine.AppendString( pOpt );
	}

	/* 新規編集ウィンドウの追加 ver 0 */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	OpenNewEditor( hinst, msgParent, sLoadInfo, cCmdLine.GetStringPtr(),
		false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
}


/* ウィンドウプロシージャじゃ */
static LRESULT CALLBACK CControlTrayWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CControlTray* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CControlTray* )g_m_pCControlTray;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		// Modified by KEITA for WIN64 2003.9.6
		//RELPRINT( _T("dispatch\n") );
		pSApp = ( CControlTray* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CControlTray
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CControlTray::CControlTray()
: m_hInstance( NULL )
, m_hWnd( NULL )
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	// アクセラレータテーブル作成
	CreateAccelTbl();

	return;
}


CControlTray::~CControlTray()
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CControlTray メンバ関数




/* 作成 */
HWND CControlTray::Create( HINSTANCE hInstance )
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlTray::Create" );

	//同名同クラスのウィンドウが既に存在していたら、失敗
	m_hInstance = hInstance;
	HWND hwndWork = ::FindWindow( GSTR_CEDITAPP, GSTR_CEDITAPP );
	if( NULL != hwndWork ){
		return NULL;
	}

	//ウィンドウクラス登録
	WNDCLASS	wc;
	{
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= CControlTrayWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= GSTR_CEDITAPP;
		ATOM	atom = RegisterClass( &wc );
		if( 0 == atom ){
			ErrorMessage( NULL, _T("CControlTray::Create()\nウィンドウクラスを登録できませんでした。") );
		}
	}
	g_m_pCControlTray = this;

	// ウィンドウ作成 (WM_CREATEで、GetHwnd() に HWND が格納される)
	::CreateWindow(
		GSTR_CEDITAPP,						// pointer to registered class name
		GSTR_CEDITAPP,						// pointer to window name
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		m_hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);

	// 最前面にする
	// (このウィンドウを親とするダイアログやメッセージボックス (Windows 終了時の保存確認、
	//  終了中ダイアログなど) が最前面に表示されるように)
	::SetWindowPos( GetTrayHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	return GetTrayHwnd();
}




/* メッセージループ */
void CControlTray::MessageLoop( void )
{
//複数プロセス版
	MSG	msg;
	int ret;
	
	//2004.02.17 Moca GetMessageのエラーチェック
	while ( GetTrayHwnd() != NULL && (ret = ::GetMessage(&msg, NULL, 0, 0 )) != 0 ){
		if( ret == -1 ){
			break;
		}
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return;

}




/* メッセージ処理 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
LRESULT CControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	int			nRowNum;
	EditNode*	pEditNodeArr;

	switch ( uMsg ){
	case MYWM_UIPI_CHECK:
		/* エディタ－トレイ間でのUI特権分離の確認メッセージ */	// 2007.06.07 ryoji
		::SendMessage( (HWND)lParam, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)0 );	// 返事を返す
		return 0L;

	/* 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
	case MYWM_DELETE_ME:
		// 現在開いている編集窓のリスト
		nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
		if( 0 < nRowNum ){
			delete [] pEditNodeArr;
		}
		// 編集ウィンドウの数が0になったら終了
		if( 0 == nRowNum ){
			::SendMessage( hwnd, WM_CLOSE, 0, 0 );
		}
		return 0;

	case WM_CREATE:
		m_hWnd = hwnd;
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( GetTrayHwnd(), GWLP_USERDATA, (LONG_PTR)this );

		// 2006.07.09 ryoji 最後の方でシャットダウンするアプリケーションにする
		BOOL (WINAPI *pfnSetProcessShutdownParameters)( DWORD dwLevel, DWORD dwFlags );
		HINSTANCE hDll;
		hDll = ::GetModuleHandle(_T("KERNEL32"));
		if( NULL != hDll ){
			*(FARPROC*)&pfnSetProcessShutdownParameters = ::GetProcAddress( hDll, "SetProcessShutdownParameters" );
			if( NULL != pfnSetProcessShutdownParameters ){
				pfnSetProcessShutdownParameters( 0x180, 0 );
			}
		}
		return 0L;

//	case WM_QUERYENDSESSION:
		case WM_COMMAND:
			OnCommand( HIWORD(wParam), LOWORD(wParam), (HWND) lParam );
			return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
		case MYWM_CHANGESETTING:
			switch( (e_PM_CHANGESETTING_SELECT)lParam ){
			case PM_CHANGESETTING_ALL:
//@@			/* 共有データの保存 */
//@@			m_cShareData.SaveShareData();

				/* アクセラレータテーブルの再作成 */
				// アクセラレータテーブル破棄
				DeleteAccelTbl();
				// アクセラレータテーブル作成
				CreateAccelTbl();
				break;
			default:
				break;
			}
			return 0L;

		case WM_QUERYENDSESSION:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor( FALSE, GetTrayHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
				//	Jan. 31, 2000 genta
				//	この時点ではWindowsの終了が確定していないので常駐解除すべきではない．
				//	::DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CloseAllEditor( FALSE, GetTrayHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
				::DestroyWindow( hwnd );
			}
			return 0L;

		//	From Here Jan. 31, 2000 genta	Windows終了時の後処理．
		//	Windows終了時はWM_CLOSEが呼ばれない上，DestroyWindowを
		//	呼び出す必要もない．また，メッセージループに戻らないので
		//	メッセージループの後ろの処理をここで完了させる必要がある．
		case WM_ENDSESSION:
			//	もしWindowsの終了が中断されたのなら何もしない
			if( wParam == TRUE )
				OnDestroy();	// 2006.07.09 ryoji WM_DESTROY と同じ処理をする（共有データの保存など）

			return 0;	//	もうこのプロセスに制御が戻ることはない
		//	To Here Jan. 31, 2000 genta
		case WM_DESTROY:
			OnDestroy();

			/* Windows にスレッドの終了を要求します。*/
			::PostQuitMessage( 0 );
			return 0L;
		case MYWM_ALLOWACTIVATE:
			::AllowSetForegroundWindow(wParam);
			return 0L;
		default:
			break;	/* default */
	}
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}




/* WM_COMMANDメッセージ処理 */
void CControlTray::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
		break;
	}
	return;
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
bool CControlTray::OpenNewEditor(
	HINSTANCE			hInstance,			//!< [in] インスタンスID (実は未使用)
	HWND				hWndParent,			//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
	const SLoadInfo&	sLoadInfo,			//!< [in]
	const TCHAR*		szCmdLineOption,	//!< [in] 追加のコマンドラインオプション
	bool				sync,				//!< [in] trueなら新規エディタの起動まで待機する
	const TCHAR*		szCurDir,			//!< [in] 新規エディタのカレントディレクトリ
	bool				bNewWindow			//!< [in] 新規エディタを新しいウインドウで開く
)
{
	/* 共有データ構造体のアドレスを返す */
	DLLSHAREDATA*	pShareData = CShareData::getInstance()->GetShareData();

	/* 編集ウィンドウの上限チェック */
	if( pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
		OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return false;
	}

	// -- -- -- -- コマンドライン文字列を生成 -- -- -- -- //
	CCommandLineString cCmdLineBuf;

	//アプリケーションパス
	TCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName( NULL, szEXE, _countof( szEXE ) );
	cCmdLineBuf.AppendF( _T("\"%ts\""), szEXE );

	// ファイル名
	if( sLoadInfo.cFilePath.c_str()[0] != _T('\0') )	cCmdLineBuf.AppendF( _T(" \"%ts\""), sLoadInfo.cFilePath.c_str() );

	// コード指定
	if( IsValidCodeType(sLoadInfo.eCharCode) )cCmdLineBuf.AppendF( _T(" -CODE=%d"), sLoadInfo.eCharCode );

	// ビューモード指定
	if( sLoadInfo.bViewMode )cCmdLineBuf.AppendF( _T(" -R") );

	// グループID
	if( false == bNewWindow ){	// 新規エディタをウインドウで開く
		// グループIDを親ウィンドウから取得
		HWND hwndAncestor = MyGetAncestor( hWndParent, GA_ROOTOWNER2 );	// 2007.10.22 ryoji GA_ROOTOWNER -> GA_ROOTOWNER2
		int nGroup = CAppNodeManager::getInstance()->GetEditNode( hwndAncestor )->GetGroup();
		if( nGroup > 0 ){
			cCmdLineBuf.AppendF( _T(" -GROUP=%d"), nGroup );
		}
	}else{
		// 空いているグループIDを使用する
		cCmdLineBuf.AppendF( _T(" -GROUP=%d"), CAppNodeManager::getInstance()->GetFreeGroupId() );
	}

	// 追加のコマンドラインオプション
	TCHAR szResponseFile[_MAX_PATH] = _T("");
	struct CResponsefileDeleter{
		LPCTSTR fileName;
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
			TCHAR szIniDir[_MAX_PATH];
			GetInidir(szIniDir);
			LPTSTR pszTempFile = _ttempnam(szIniDir, _T("skr_resp"));
			if( !pszTempFile ){
				ErrorMessage(hWndParent, _T("レスポンスファイルの作成に失敗しました。"));
				return false;
			}
			auto_strcpy(szResponseFile, pszTempFile);
			free(pszTempFile);
			CTextOutputStream output(szResponseFile);
			if( !output ){
				ErrorMessage(hWndParent, _T("レスポンスファイルの作成に失敗しました。"));
				return false;
			}
			respDeleter.fileName = szResponseFile;
			// 出力
			output.WriteString(to_wchar(szCmdLineOption));
			output.Close();
			sync = true;
			cCmdLineBuf.AppendF(_T(" -@=\"%ts\""), szResponseFile);
		}else{
			cCmdLineBuf.AppendF(_T(" %ts"), szCmdLineOption);
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
	TCHAR szCmdLine[1024]; _tcscpy_s(szCmdLine, _countof(szCmdLine), cCmdLineBuf.c_str());
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
		TCHAR* pMsg;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			_T("\'%ts\'\nプロセスの起動に失敗しました。\n%ts"),
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
				_T("\'%ts\'\nプロセスの起動に失敗しました。"),
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
bool CControlTray::OpenNewEditor2(
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
		OkMessage( NULL, _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
		return false;
	}

	// 追加のコマンドラインオプション
	CCommandLineString cCmdLine;
	if( pfi != NULL ){
		if( pfi->m_ptCursor.x >= 0					)cCmdLine.AppendF( _T(" -X=%d"), pfi->m_ptCursor.x +1 );
		if( pfi->m_ptCursor.y >= 0					)cCmdLine.AppendF( _T(" -Y=%d"), pfi->m_ptCursor.y +1 );
		if( pfi->m_nViewLeftCol >= CLayoutInt(0)	)cCmdLine.AppendF( _T(" -VX=%d"), (Int)pfi->m_nViewLeftCol + 1 );
		if( pfi->m_nViewTopLine >= CLayoutInt(0)	)cCmdLine.AppendF( _T(" -VY=%d"), (Int)pfi->m_nViewTopLine + 1 );
	}
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = pfi ? pfi->m_szPath : _T("");
	sLoadInfo.eCharCode = pfi ? pfi->m_nCharCode : CODE_NONE;
	sLoadInfo.bViewMode = bViewMode;
	return OpenNewEditor( hInstance, hWndParent, sLoadInfo, cCmdLine.c_str(), sync, NULL, bNewWindow );
}
//	To Here Oct. 24, 2000 genta



void CControlTray::ActiveNextWindow()
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

void CControlTray::ActivePrevWindow()
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
void CControlTray::TerminateApplication(
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
				_T("現在開いている編集用のウィンドウをすべて閉じて終了しますか?")
			) ){
				return;
			}
		}
	}
	/* 「すべてのウィンドウを閉じる」要求 */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	BOOL bCheckConfirm = (pShareData->m_Common.m_sGeneral.m_bExitConfirm)? FALSE: TRUE;	// 2006.12.25 ryoji 終了確認済みならそれ以上は確認しない
	if( CloseAllEditor( bCheckConfirm, hWndFrom, TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
		::PostMessageAny( pShareData->m_sHandles.m_hwndTray, WM_CLOSE, 0, 0 );
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
BOOL CControlTray::CloseAllEditor(
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




/*! アクセラレータテーブル作成
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlTray::CreateAccelTbl( void )
{
	m_pShareData->m_sHandles.m_hAccel = CKeyBind::CreateAccerelator(
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);

	if( NULL == m_pShareData->m_sHandles.m_hAccel ){
		ErrorMessage(
			NULL,
			_T("CControlTray::CreateAccelTbl()\n")
			_T("アクセラレータ テーブルが作成できません。\n")
			_T("システムリソースが不足しています。")
		);
	}
}

/*! アクセラレータテーブル破棄
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlTray::DeleteAccelTbl( void )
{
	if( m_pShareData->m_sHandles.m_hAccel ){
		::DestroyAcceleratorTable( m_pShareData->m_sHandles.m_hAccel );
		m_pShareData->m_sHandles.m_hAccel = NULL;
	}
}

/*!
	@brief WM_DESTROY 処理
	@date 2006.07.09 ryoji 新規作成
*/
void CControlTray::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (GetTrayHwnd() == NULL)
		return;	// 既に破棄されている

	// 2006.07.09 ryoji 共有データ保存を CControlProcess::Terminate() から移動
	//
	// エディタ画面（Normal Process）を立ち上げたままセッション終了するような場合でも
	// 共有データ保存が行われなかったり中断されることが無いよう、
	// ここでウィンドウが破棄される前に保存する
	//

	/* 終了ダイアログを表示する */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* 終了中ダイアログの表示 */
		hwndExitingDlg = ::CreateDialog(
			m_hInstance,
			MAKEINTRESOURCE( IDD_EXITING ),
			GetTrayHwnd()/*::GetDesktopWindow()*/,
			ExitingDlgProc
		);
		::ShowWindow( hwndExitingDlg, SW_SHOW );
	}

	/* 共有データの保存 */
	CShareData_IO::SaveShareData();

	/* 終了ダイアログを表示する */
	if( m_pShareData->m_Common.m_sGeneral.m_bDispExitingDialog ){
		/* 終了中ダイアログの破棄 */
		::DestroyWindow( hwndExitingDlg );
	}

	// アクセラレータテーブルの削除
	DeleteAccelTbl();

	SessionStore::destroyUniqInstance();

	m_hWnd = NULL;
}

/*!
	@brief 終了ダイアログ用プロシージャ
	@date 2006.07.02 ryoji CControlProcess から移動
*/
INT_PTR CALLBACK CControlTray::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
)
{
	switch( uMsg ){
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}
