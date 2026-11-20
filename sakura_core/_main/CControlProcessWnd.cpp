/*!	@file
	@brief 常駐部

	キー割り当て、共有データの保存、Windows 終了時の処理など (編集ウィンドウの起動・終了は CEditorManager)

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

#include "CControlProcessWnd.h"
#include "CEditorManager.h"
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
static LRESULT CALLBACK CControlProcessWndProc( HWND, UINT, WPARAM, LPARAM );

static CControlProcessWnd*	g_pcControlProcessWnd;

/* ウィンドウプロシージャじゃ */
static LRESULT CALLBACK CControlProcessWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	CControlProcessWnd* pSApp;
	switch( uMsg ){
	case WM_CREATE:
		pSApp = ( CControlProcessWnd* )g_pcControlProcessWnd;
		return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		// Modified by KEITA for WIN64 2003.9.6
		//RELPRINT( L"dispatch\n" );
		pSApp = ( CControlProcessWnd* )::GetWindowLongPtr( hwnd, GWLP_USERDATA );
		if( NULL != pSApp ){
			return pSApp->DispatchEvent( hwnd, uMsg, wParam, lParam );
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




/////////////////////////////////////////////////////////////////////////////
// CControlProcessWnd
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CControlProcessWnd::CControlProcessWnd()
: m_hInstance( NULL )
, m_hWnd( NULL )
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	// アクセラレータテーブル作成
	CreateAccelTbl();

	return;
}


CControlProcessWnd::~CControlProcessWnd()
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CControlProcessWnd メンバ関数




/* 作成 */
HWND CControlProcessWnd::Create( HINSTANCE hInstance )
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcessWnd::Create" );

	//同名同クラスのウィンドウが既に存在していたら、失敗
	m_hInstance = hInstance;
	HWND hwndWork = ::FindWindow( GSTR_CONTROL_PROCESS_WND, GSTR_CONTROL_PROCESS_WND );
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
		wc.lpfnWndProc		= CControlProcessWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= GSTR_CONTROL_PROCESS_WND;
		ATOM	atom = RegisterClass( &wc );
		if( 0 == atom ){
			ErrorMessage( NULL, L"CControlProcessWnd::Create()\nウィンドウクラスを登録できませんでした。" );
		}
	}
	g_pcControlProcessWnd = this;

	// ウィンドウ作成 (WM_CREATEで、GetHwnd() に HWND が格納される)
	::CreateWindow(
		GSTR_CONTROL_PROCESS_WND,						// pointer to registered class name
		GSTR_CONTROL_PROCESS_WND,						// pointer to window name
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
	::SetWindowPos( GetHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	return GetHwnd();
}




/* メッセージループ */
void CControlProcessWnd::MessageLoop( void )
{
//複数プロセス版
	MSG	msg;
	int ret;
	
	//2004.02.17 Moca GetMessageのエラーチェック
	while ( GetHwnd() != NULL && (ret = ::GetMessage(&msg, NULL, 0, 0 )) != 0 ){
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
LRESULT CControlProcessWnd::DispatchEvent(
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
		/* エディタ－コントロールプロセス間でのUI特権分離の確認メッセージ */	// 2007.06.07 ryoji
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
		::SetWindowLongPtr( GetHwnd(), GWLP_USERDATA, (LONG_PTR)this );

		// 2006.07.09 ryoji 最後の方でシャットダウンするアプリケーションにする
		::SetProcessShutdownParameters( 0x180, 0 );
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
			if( CEditorManager::CloseAllEditor( FALSE, GetHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
				//	Jan. 31, 2000 genta
				//	この時点ではWindowsの終了が確定していないので常駐解除すべきではない．
				//	::DestroyWindow( hwnd );
				return TRUE;
			}else{
				return FALSE;
			}
		case WM_CLOSE:
			/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
			if( CEditorManager::CloseAllEditor( FALSE, GetHwnd(), TRUE, 0 ) ){	// 2006.12.25, 2007.02.13 ryoji 引数追加
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
void CControlProcessWnd::OnCommand( WORD wNotifyCode, WORD wID , HWND hwndCtl )
{
	switch( wNotifyCode ){
	/* メニューからのメッセージ */
	case 0:
		break;
	}
	return;
}

/*! アクセラレータテーブル作成
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlProcessWnd::CreateAccelTbl( void )
{
	m_pShareData->m_sHandles.m_hAccel = CKeyBind::CreateAccerelator(
		m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
		m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr
	);

	if( NULL == m_pShareData->m_sHandles.m_hAccel ){
		ErrorMessage(
			NULL,
			L"CControlProcessWnd::CreateAccelTbl()\n"
			L"アクセラレータ テーブルが作成できません。\n"
			L"システムリソースが不足しています。"
		);
	}
}

/*! アクセラレータテーブル破棄
	@date 2013.04.20 novice 共通処理を関数化
*/
void CControlProcessWnd::DeleteAccelTbl( void )
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
void CControlProcessWnd::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (GetHwnd() == NULL)
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
			GetHwnd()/*::GetDesktopWindow()*/,
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
INT_PTR CALLBACK CControlProcessWnd::ExitingDlgProc(
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
