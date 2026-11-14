/*!	@file
	@brief 編集ウィンドウの管理

	編集ウィンドウ (エディタプロセス) の起動・終了・切り替え

	@date 2026/11/14 CControlTray から分離
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

#ifndef _CEDITORMANAGER_H_
#define _CEDITORMANAGER_H_

#include <Windows.h>

struct SLoadInfo;
struct EditInfo;
class CDlgGrep;

//!	編集ウィンドウの管理
/*!
	編集ウィンドウ (エディタプロセス) の起動・終了・切り替えを行う。
	静的関数のみのクラスで、インスタンスは生成しない。
	どのプロセス (主に編集ウィンドウのプロセス) からも呼び出せる。

	@date 2026/11/14 CControlTray の静的関数を分離
*/
class CEditorManager
{
public:
	//編集ウィンドウの起動
	static bool OpenNewEditor(							//!< 新規編集ウィンドウの追加 ver 0
		HINSTANCE			hInstance,					//!< [in] インスタンスID (実は未使用)
		HWND				hWndParent,					//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
		const SLoadInfo&	sLoadInfo,					//!< [in]
		const TCHAR*		szCmdLineOption	= NULL,		//!< [in] 追加のコマンドラインオプション
		bool				sync			= false,	//!< [in] trueなら新規エディタの起動まで待機する
		const TCHAR*		szCurDir		= NULL,		//!< [in] 新規エディタのカレントディレクトリ
		bool				bNewWindow		= false		//!< [in] 新規エディタをウインドウで開く
	);
	static bool OpenNewEditor2(						//!< 新規編集ウィンドウの追加 ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo*	pfi,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, CDlgGrep& cDlgGrep);	//!< Grep結果ウィンドウの起動

	//編集ウィンドウの切り替え
	static void ActiveNextWindow();
	static void ActivePrevWindow();

	//編集ウィンドウの終了
	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom, BOOL bExit, int nGroup );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更	// 2006.12.25, 2007.02.13 ryoji 引数追加
	static void TerminateApplication( HWND hWndFrom );	/* サクラエディタの全終了 */	// 2006.12.25 ryoji 引数追加

private:
	CEditorManager();	// インスタンスは生成しない
};

#endif /* _CEDITORMANAGER_H_ */
