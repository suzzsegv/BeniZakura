/*!	@file
	@brief 常駐部

	キー割り当て、共有データの保存、Windows 終了時の処理など (編集ウィンドウの起動・終了は CEditorManager)

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	@date 2007/10/23 kobake     クラス名、ファイル名変更: CEditApp→CControlTray
	@date 2026/11/14 タスクトレイのアイコンとメニュー (および、そのホットキー) を削除
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCONTROLPROCESSWND_H_
#define _CCONTROLPROCESSWND_H_

#include <Windows.h>

struct DLLSHAREDATA;

//!	常駐部の管理
/*!
	キー割り当て、共有データの保存、Windows 終了時の処理など (編集ウィンドウの起動・終了は CEditorManager)
	（タスクトレイのアイコンとメニューは 2026/11/14 に削除）
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CControlProcessWnd
{
public:
	/*
	||  Constructors
	*/
	CControlProcessWnd();
	~CControlProcessWnd();

	/*
	|| メンバ関数
	*/
	HWND Create( HINSTANCE );	/* 作成 */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */
	void MessageLoop( void );	/* メッセージループ */
	void OnDestroy( void );		/* WM_DESTROY 処理 */	// 2006.07.09 ryoji
	void CreateAccelTbl( void ); // アクセラレータテーブル作成
	void DeleteAccelTbl( void ); // アクセラレータテーブル破棄

public:
	HWND GetHwnd() const{ return m_hWnd; }

protected:
	void OnCommand( WORD , WORD  , HWND );	/*!< WM_COMMANDメッセージ処理 */

	static INT_PTR CALLBACK ExitingDlgProc(	/*!< 終了ダイアログ用プロシージャ */	// 2006.07.02 ryoji CControlProcess から移動
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);


	/*
	|| メンバ変数
	*/
private:
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;

	DLLSHAREDATA*	m_pShareData;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CCONTROLPROCESSWND_H_ */



