/*!	@file
	共通設定ダイアログボックス、「メインメニュー」ページ

	@author Uchi
*/
/*
	Copyright (C) 2010, Uchi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "env/cShareData_IO.h"
#include "typeprop/CImpExpManager.h"
#include "dlg/CDlgInput1.h"
#include "util/shell.h"
#include "sakura_rc.h"

using std::wstring;

// TreeView 表示固定初期値
const WCHAR	DEFAULT_NODE[]	= L"編集してください";
const WCHAR	DEFAULT_SEPA[]	= L"――――――――――";

// 内部使用変数
// 機能格納(Work)
struct SMainMenuWork {
	wstring			m_sName;		// 名前
	EFunctionCode	m_nFunc;		// Function
	WCHAR			m_sKey[2];		// アクセスキー
	bool			m_bDupErr;		// アクセスキー重複エラー
};

static	std::map<int, SMainMenuWork>	msMenu;	// 一時データ
static	int		nMenuCnt = 0;					// 一時データ番号


// ローカル関数定義
static HTREEITEM TreeCopy( HWND, HTREEITEM, HTREEITEM, bool, bool );
static void TreeView_ExpandAll( HWND, bool );
static const TCHAR * MakeDispLabel( SMainMenuWork* );

// 特別機能
struct SSpecialFunc	{
	EFunctionCode	m_nFunc;		// Function
	const WCHAR* 	m_sName;		// 名前
};

static	const	TCHAR*	NAME_SPECIAL_TOP	= _T("特別機能");
static	SSpecialFunc	sSpecialFuncs[] = {
		{F_WINDOW_LIST,				L"ウィンドウリスト",},
		{F_FILE_USED_RECENTLY,		L"最近使ったファイル",},
		{F_FOLDER_USED_RECENTLY,	L"最近使ったフォルダ",},
		{F_CUSTMENU_LIST,			L"カスタムメニューリスト",},
		{F_USERMACRO_LIST,			L"登録済みマクロリスト",},
};

static	int 	nSpecialFuncsNum;		// 特別機能のコンボボックス内での番号

//  TreeViewキー入力時のメッセージ処理用
static WNDPROC	m_wpTreeView = NULL;
static HWND		m_hwndDlg;

// TreeViewラベル編集時のメッセージ処理用
static WNDPROC	m_wpEdit = NULL;


/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropMainMenu::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropMainMenu::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}



// TreeViewキー入力時のメッセージ処理
static LRESULT CALLBACK TreeViewProc(
	HWND	hwndTree,		// handle to dialog box
	UINT	uMsg,			// message
	WPARAM	wParam,			// first message parameter
	LPARAM	lParam 			// second message parameter
)
{
	HTREEITEM		htiItem;
	TV_ITEM			tvi;		// 取得用
	WCHAR			cKey;
	SMainMenuWork*	pFuncWk;	// 機能
	short	AltKey;

	switch (uMsg) {
	case WM_GETDLGCODE:
		MSG*	pMsg;
		if (lParam == 0) {
			break;
		}
		pMsg = (MSG*)lParam;
		if (pMsg->wParam == wParam && ( wParam == VK_RETURN|| wParam == VK_ESCAPE || wParam == VK_TAB )) {
			break;
		}
		return DLGC_WANTALLKEYS;
	case WM_KEYDOWN:
		htiItem = TreeView_GetSelection( hwndTree );
		cKey = (WCHAR)MapVirtualKey( wParam, 2 );
		if (cKey > ' ') {
			// アクセスキー設定
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			if (!TreeView_GetItem( hwndTree, &tvi )) {
				break;
			}
			pFuncWk = &msMenu[tvi.lParam];
			if (pFuncWk->m_nFunc == F_SEPARATOR) {
				return 0;
			}
			pFuncWk->m_sKey[0] = cKey;
			pFuncWk->m_sKey[1] = L'\0';
			pFuncWk->m_bDupErr = false;
			tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
			tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
			TreeView_SetItem( hwndTree , &tvi );		//	キー設定結果を反映
			return 0;
		}

		AltKey = GetKeyState(VK_MENU);

		switch (wParam) {
		case VK_BACK:
		case VK_DELETE:	//	DELキーが押されたらダイアログボックスにメッセージを送信
			::SendMessage( m_hwndDlg, WM_COMMAND, IDC_BUTTON_DELETE, (LPARAM)::GetDlgItem( m_hwndDlg, IDC_BUTTON_DELETE ) );
			return 0;
		case VK_F2:						// F2で編集
			if (htiItem != NULL) {
				TreeView_EditLabel( hwndTree, htiItem );
			}
			return 0;
		}
		break;
	case WM_CHAR:
		return 0;
	}
	return  CallWindowProc( m_wpTreeView, hwndTree, uMsg, wParam, lParam);
}

// TreeViewラベル編集時のメッセージ処理
static LRESULT CALLBACK WindowProcEdit(
	HWND	hwndEdit,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	switch (uMsg) {
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}
	return CallWindowProc( m_wpEdit, hwndEdit, uMsg, wParam, lParam );
}

/* Menu メッセージ処理 */
INT_PTR CPropMainMenu::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	int			idCtrl;
	static HWND	hwndComboFunkKind;
	static HWND	hwndListFunk;
	static HWND	hwndTreeRes;

	int			i;

	HTREEITEM	nIdxMenu;
	int			nIdxFIdx;
	int			nIdxFunc;
	WCHAR		szLabel[MAX_MAIN_MENU_NAME_LEN+10];

	EFunctionCode	eFuncCode;
	SMainMenuWork*	pFuncWk;	// 機能
	TCHAR			szKey[2];

	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	HTREEITEM		htiItem;
	HTREEITEM		htiParent;
	HTREEITEM		htiTemp;
	HTREEITEM		htiTemp2;
	TV_DISPINFO*	ptdi;

	CDlgInput1		cDlgInput1;

	static	bool	bInMove;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Menu */
		SetData( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* コントロールのハンドルを取得 */
		hwndComboFunkKind = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndListFunk = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );

		/* キー選択時の処理 */
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndComboFunkKind );

		// TreeViewのメッセージ処理（アクセスキー入力用）
		m_hwndDlg = hwndDlg;
		m_wpTreeView = (WNDPROC)SetWindowLongPtr( hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)TreeViewProc );

		::SetTimer( hwndDlg, 1, 300, NULL );

		bInMove = false;

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		ptdi = (TV_DISPINFO*)lParam;

		switch( pNMHDR->code ){
		case PSN_KILLACTIVE:
			/* ダイアログデータの取得 Menu */
			GetData( hwndDlg );
			return TRUE;
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_MAINMENU;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）
			nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );
			nIdxFunc = List_GetCurSel( hwndListFunk );
			if( nIdxFIdx != CB_ERR ){
				::SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndComboFunkKind );
				if( nIdxFunc != LB_ERR ){
					List_SetCurSel( hwndListFunk, nIdxFunc );
				}
			}
			return TRUE;
		case TVN_BEGINLABELEDIT:	//	アイテムの編集開始
			if (pNMHDR->hwndFrom == hwndTreeRes) { 
				HWND hEdit = TreeView_GetEditControl( hwndTreeRes );
				if (msMenu[ptdi->item.lParam].m_nFunc == F_NODE) {
					// ノードのみ有効
					SetWindowText( hEdit, to_tchar( msMenu[ptdi->item.lParam].m_sName.c_str() ) ) ;
					EditCtl_LimitText( hEdit, MAX_MAIN_MENU_NAME_LEN );
					// 編集時のメッセージ処理
					m_wpEdit = (WNDPROC)SetWindowLongPtr( hEdit, GWLP_WNDPROC, (LONG_PTR)WindowProcEdit );
				}
				else {
					// ノード以外編集不可
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
				}
			}
			return TRUE;
		case TVN_ENDLABELEDIT:		//	アイテムの編集が終了
 			if (pNMHDR->hwndFrom == hwndTreeRes 
			  && (msMenu[ptdi->item.lParam].m_nFunc == F_NODE)) {
				// ノード有効
				pFuncWk = &msMenu[ptdi->item.lParam];
				if (ptdi->item.pszText == NULL) {
					// Esc
					//	何も設定しない（元のまま）
				}
				else if (auto_strcmp(ptdi->item.pszText, _T("")) == 0) {
					// 空
					pFuncWk->m_sName = DEFAULT_NODE;
				}
				else {
					pFuncWk->m_sName = to_wchar(ptdi->item.pszText);
				}
				ptdi->item.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
				TreeView_SetItem( hwndTreeRes , &ptdi->item );	//	編集結果を反映

				// 編集時のメッセージ処理を戻す
				SetWindowLongPtr( TreeView_GetEditControl( hwndTreeRes ), GWLP_WNDPROC, (LONG_PTR)m_wpEdit );
				m_wpEdit = NULL;
			}
			return TRUE;
		case TVN_DELETEITEM:
			if (!bInMove && !msMenu.empty()
			  && pNMHDR->hwndFrom == hwndTreeRes
			  && (htiItem = TreeView_GetSelection( hwndTreeRes )) != NULL) {
				//付属情報を削除
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (TreeView_GetItem( hwndTreeRes, &tvi )) {
					msMenu.erase( tvi.lParam );
				}
				return 0;
			}
			break;
		case NM_DBLCLK:
			// ダブルクリック時の処理
			if (pNMHDR->hwndFrom == hwndTreeRes) {
				htiItem = TreeView_GetSelection( hwndTreeRes );
				if (htiItem == NULL) {
					break;
				}
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if (!TreeView_GetItem( hwndTreeRes, &tvi )) {
					break;
				}
				pFuncWk = &msMenu[tvi.lParam];
				if (pFuncWk->m_nFunc != F_SEPARATOR) {
					auto_sprintf( szKey, _T("%ls"), pFuncWk->m_sKey);

					if (!cDlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							_T("メニューアイテムのアクセスキー設定"),
							_T("キーを入力してください。"),
							1,
							szKey)) {
						return TRUE;
					}
					auto_sprintf( pFuncWk->m_sKey, L"%ts", szKey);
					pFuncWk->m_bDupErr = false;

					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
					TreeView_SetItem( hwndTreeRes, &tvi );
				}
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);			/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;		/* コントロールのハンドル */

		if (hwndComboFunkKind == hwndCtl) {
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );

				if (nIdxFIdx == nSpecialFuncsNum) {
					// 機能一覧に特殊機能をセット
					List_ResetContent( hwndListFunk );
					for (i = 0; i <_countof(sSpecialFuncs); i++) {
						List_AddString( hwndListFunk, sSpecialFuncs[i].m_sName );
					}
				}
				else {
					/* 機能一覧に文字列をセット（リストボックス）*/
					m_cLookup.SetListItem( hwndListFunk, nIdxFIdx );
				}

				return TRUE;
			}
		}
		else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_IMPORT:	/* インポート */
					/* カスタムメニュー設定をインポートする */
					Import( hwndDlg );
					return TRUE;
				case IDC_BUTTON_EXPORT:	/* エクスポート */
					/* カスタムメニュー設定をエクスポートする */
					Export( hwndDlg );
					return TRUE;

				case IDC_BUTTON_CLEAR:
					if (IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("メニューの設定をクリアします。\nよろしいですか？")) ) {
						return TRUE;
					}
					// 内部データ初期化
					msMenu.clear();
					nMenuCnt = 0;
					// TreeView初期化
					TreeView_DeleteAllItems( hwndTreeRes );
					return TRUE;

				case IDC_BUTTON_INITIALIZE:
					if (IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("メニューの設定を初期状態に戻します。\nよろしいですか？"))) {
						return TRUE;
					}
					// 初期状態に戻す
					{
						CDataProfile	cProfile;
						cProfile.SetReadingMode();
						cProfile.ReadProfileRes( MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI) );

						CShareData_IO::IO_MainMenu( cProfile, m_Common.m_sMainMenu, false );
						
						SetData( hwndDlg ); 
					}
					return TRUE;

				case IDC_BUTTON_DELETE:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem != NULL) {
						if (TreeView_GetChild( hwndTreeRes, htiItem ) != NULL
						  && IDCANCEL == ::MYMESSAGEBOX( hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
							_T("選択している項目を下位項目ごと削除します。\nよろしいですか？"))) {
							return TRUE;
						}
						htiTemp = TreeView_GetNextSibling( hwndTreeRes, htiItem );
						if (htiTemp == NULL) {
							// 末尾ならば、前を取る
							htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
						}
						TreeView_DeleteItem( hwndTreeRes, htiItem );
						if (htiTemp != NULL) {
							TreeView_SelectItem( hwndTreeRes, htiTemp );
						}
					}
					break;


				case IDC_BUTTON_INSERT_NODE:			// ノード挿入
				case IDC_BUTTON_INSERTSEPARATOR:		// 区切線挿入
				case IDC_BUTTON_INSERT:					// 挿入(上)
				case IDC_BUTTON_INSERT_A:				// 挿入(下)
				case IDC_BUTTON_ADD:					// 追加
					eFuncCode = F_INVALID;
					switch (wID) {
					case IDC_BUTTON_INSERT_NODE:		// ノード挿入
						eFuncCode = F_NODE;
						auto_strcpy( szLabel , DEFAULT_NODE );
						break;
					case IDC_BUTTON_INSERTSEPARATOR:	// 区切線挿入
						eFuncCode = F_SEPARATOR;
						auto_strcpy( szLabel , DEFAULT_SEPA );
						break;
					case IDC_BUTTON_INSERT:				// 挿入
					case IDC_BUTTON_INSERT_A:			// 挿入
					case IDC_BUTTON_ADD:				// 追加
						// Function 取得
						if (CB_ERR == (nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind ))) {
							break;
						}
						if (LB_ERR == (nIdxFunc = List_GetCurSel( hwndListFunk ))) {
							break;
						}
						if (nIdxFIdx == nSpecialFuncsNum) {
							// 特殊機能
							auto_strcpy( szLabel, sSpecialFuncs[nIdxFunc].m_sName );
							eFuncCode = sSpecialFuncs[nIdxFunc].m_nFunc;
						}
						else if (m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc ) != 0) {
							List_GetText( hwndListFunk, nIdxFunc, szLabel );
							eFuncCode = m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc );
						}
						else {
							auto_strcpy( szLabel, L"?" );
							eFuncCode = F_SEPARATOR;
						}
						break;
					}

					// 挿入位置検索
					htiTemp = TreeView_GetSelection( hwndTreeRes );
					if (htiTemp == NULL) {
						// 取れなかったらRootの末尾
						htiParent = TVI_ROOT;
						htiTemp = TVI_LAST;
					}
					else {
						if (wID == IDC_BUTTON_ADD) {
							// 追加
							tvi.mask = TVIF_HANDLE | TVIF_PARAM;
							tvi.hItem = htiTemp;
							if (!TreeView_GetItem( hwndTreeRes, &tvi )) {
								// 取れなかったらRootの末尾
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}
							else {
								if (msMenu[tvi.lParam].m_nFunc == F_NODE) {
									// ノード
									htiParent = htiTemp;
									htiTemp = TVI_LAST;
								}
								else {
									// 子を付けられないので親に付ける（選択アイテムの下に付く）
									htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
									htiTemp = TVI_LAST;
									if (htiParent == NULL) {
										// 取れなかったらRootの末尾
										htiParent = TVI_ROOT;
									}
								}
							}
						}
						else if (wID == IDC_BUTTON_INSERT_NODE || wID == IDC_BUTTON_INSERT_A) {
							// ノード挿入、挿入(下)
							// 追加先を探る
							htiTemp = TreeView_GetSelection( hwndTreeRes );
							if (htiTemp == NULL) {
								htiParent = TVI_ROOT;
								htiTemp = TVI_LAST;
							}
							else {
								tvi.mask = TVIF_HANDLE | TVIF_PARAM;
								tvi.hItem = htiTemp;
								if (TreeView_GetItem( hwndTreeRes, &tvi )) {
									if (msMenu[tvi.lParam].m_nFunc == F_NODE) {
										// ノード
										htiParent = htiTemp;
										htiTemp = TVI_FIRST;
									}
									else {
										// 子を付けられないので親に付ける（選択アイテムの下に付く）
										htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
										if (htiParent == NULL) {
											// 取れなかったらRoot
											htiParent = TVI_ROOT;
										}
									}
								}
								else {
									// 取れなかったらRoot
									htiParent = TVI_ROOT;
									htiTemp = TVI_LAST;
								}
							}
						}
						else {
							// 挿入(上)、区切線
							// 挿入先を探る
							htiParent = TreeView_GetParent( hwndTreeRes, htiTemp );
							if (htiParent == NULL) {
								// 取れなかったらRootのトップ
								htiParent = TVI_ROOT;
								htiTemp = TVI_FIRST;
							}
							else {
								// 一つ手前
								htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiTemp );
								if (htiTemp == NULL) {
									// 取れなかったら親の最初
									htiTemp = TVI_FIRST;
								}
							}
						}
					}

					// TreeViewに挿入
					pFuncWk = &msMenu[nMenuCnt];
					pFuncWk->m_nFunc = (EFunctionCode)eFuncCode;
					pFuncWk->m_sName = szLabel;
					pFuncWk->m_bDupErr = false;
					auto_strcpy(pFuncWk->m_sKey, L"");
					tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
					tvis.hParent = htiParent;
					tvis.hInsertAfter = htiTemp;
					tvis.item.pszText = const_cast<TCHAR*>(to_tchar(szLabel));
					tvis.item.lParam = nMenuCnt++;
					tvis.item.cChildren = ( wID == IDC_BUTTON_INSERT_NODE );
					htiItem = TreeView_InsertItem( hwndTreeRes, &tvis );
					// 展開
					if (htiParent != TVI_ROOT) {
						TreeView_Expand( hwndTreeRes, htiParent, TVE_EXPAND );
					}
					TreeView_SelectItem( hwndTreeRes, htiItem );

					// リストを1つ進める
					switch (wID) {
					case IDC_BUTTON_INSERT:				// 挿入
					case IDC_BUTTON_INSERT_A:			// 挿入
					case IDC_BUTTON_ADD:				// 追加
						List_SetCurSel( hwndListFunk, nIdxFunc+1 );
						break;
					}
					break;


				case IDC_BUTTON_UP:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// そのエリアで最初
						break;
					}

					// コピー
					bInMove = true;
					TreeCopy(hwndTreeRes, htiItem, htiTemp, false, true);

					// 削除
					TreeView_DeleteItem( hwndTreeRes, htiTemp );
					bInMove = false;
					break;

				case IDC_BUTTON_DOWN:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetNextSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// そのエリアで最後
						break;
					}

					// コピー
					bInMove = true;
					TreeCopy(hwndTreeRes, htiTemp, htiItem, false, true);

					// 削除
					TreeView_DeleteItem( hwndTreeRes, htiItem );
					bInMove = false;

					// 選択
					htiItem = TreeView_GetNextSibling( hwndTreeRes, htiTemp );
					if (htiItem != NULL) {
						TreeView_SelectItem( hwndTreeRes, htiItem );
					}
					break;

				case IDC_BUTTON_RIGHT:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiTemp = TreeView_GetPrevSibling( hwndTreeRes, htiItem );
					if (htiTemp == NULL) {
						// そのエリアで最初
						break;
					}
					// ノード確認
					tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
					tvi.hItem = htiTemp;
					i = TreeView_GetItem( hwndTreeRes, &tvi );
					if (!TreeView_GetItem( hwndTreeRes, &tvi )) {
						// エラー
						break;
					}
					if (tvi.cChildren) {
						// 直前がノード
						HTREEITEM		htiTemp2;
						// コピー
						bInMove = true;
						htiTemp2 = TreeCopy(hwndTreeRes, htiTemp, htiItem, true, true);

						// 削除
						TreeView_DeleteItem( hwndTreeRes, htiItem );
						bInMove = false;

						// 選択
						TreeView_SelectItem( hwndTreeRes, htiTemp2 );
					}
					else {
						// ノードが無い
						break;
					}
					break;

				case IDC_BUTTON_LEFT:
					htiItem = TreeView_GetSelection( hwndTreeRes );
					if (htiItem == NULL) {
						break;
					}
					htiParent = TreeView_GetParent( hwndTreeRes, htiItem );
					if (htiParent == NULL) {
						// Root
						break;
					}
					// コピー
					bInMove = true;
					htiTemp2 = TreeCopy(hwndTreeRes, htiParent, htiItem, false, true);

					// 削除
					TreeView_DeleteItem( hwndTreeRes, htiItem );
					bInMove = false;

					// 選択
					TreeView_SelectItem( hwndTreeRes, htiTemp2 );
					break;


				case IDC_BUTTON_CHECK:		// メニューの検査
					{
						wstring sErrMsg;
						if (Check_MainMenu( hwndTreeRes, sErrMsg )) {
							InfoMessage( hwndDlg, _T("問題ありませんでした。"));
						}
						else {
							WarningMessage( hwndDlg, to_tchar(sErrMsg.c_str()) );
						}
					}
					break;


				case IDC_BUTTON_EXPAND:		// ツリー全開
					TreeView_ExpandAll( hwndTreeRes, true );
					break;

				case IDC_BUTTON_COLLAPSE:	// ツリー全閉
					TreeView_ExpandAll( hwndTreeRes, false );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdxMenu = TreeView_GetSelection( hwndTreeRes );
		nIdxFIdx = Combo_GetCurSel( hwndComboFunkKind );
		nIdxFunc = List_GetCurSel( hwndListFunk );
		i = List_GetCount( hwndTreeRes );
		if (nIdxMenu == NULL) {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ),     FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ),   FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RIGHT ),  FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_LEFT ),   FALSE );
		}
		else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ),     NULL != TreeView_GetPrevSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ),   NULL != TreeView_GetNextSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RIGHT ),  NULL != TreeView_GetPrevSibling( hwndTreeRes, nIdxMenu ) );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_LEFT ),   NULL != TreeView_GetParent( hwndTreeRes, nIdxMenu ) );
		}
		if (LB_ERR == nIdxFunc ||
		  ( CB_ERR != nIdxFIdx && LB_ERR != nIdxFunc &&
		    (m_cLookup.Pos2FuncCode( nIdxFIdx, nIdxFunc ) == 0 && nIdxFIdx != nSpecialFuncsNum))) {
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT_A ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), NULL != nIdxMenu );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT_A ), NULL != nIdxMenu );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		break;
	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );

		// 編集時のメッセージ処理を戻す
		SetWindowLongPtr( hwndTreeRes, GWLP_WNDPROC, (LONG_PTR)m_wpTreeView );
		m_wpTreeView = NULL;

		// ワークのクリア
		msMenu.clear();
		nMenuCnt = 0;
		break;
	}
	return FALSE;
}




// & の補完
static wstring	SupplementAmpersand( wstring sLavel)
{
	size_t	nPos =0;
	while ((nPos = sLavel.find( L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos+1] != L'&') {
			// &&でない
			sLavel.replace( nPos, 1, L"&&");
		}
		nPos +=2;
	}
	return sLavel;
}

// & の削除
static wstring	RemoveAmpersand( wstring sLavel)
{
	size_t	nPos =0;
	while ((nPos = sLavel.find( L"&", nPos)) != wstring::npos) {
		if (sLavel[nPos+1] == L'&') {
			// &&
			sLavel.replace( nPos, 1, L"");
		}
		nPos ++;
	}
	return sLavel;
}

/* ダイアログデータの設定 MainMenu */
void CPropMainMenu::SetData( HWND hwndDlg )
{
	CMainMenu*		pcMenuTBL = m_Common.m_sMainMenu.m_cMainMenuTbl;
	CMainMenu*		pcFunc;
	HWND			hwndCombo;
	HWND			hwndCheck;
	HWND			hwndTreeRes;
	WCHAR			szLabel[MAX_MAIN_MENU_NAME_LEN+10];
	int				nCurLevel;
	HTREEITEM		htiItem;
	HTREEITEM		htiParent;
	TV_INSERTSTRUCT	tvis;			// 挿入用
	SMainMenuWork*	pFuncWk;		// 機能(work)
	int 			i;
	int 			j;

	/* 機能種別一覧に文字列をセット（コンボボックス） */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );

	// 特別機能追加
	nSpecialFuncsNum = Combo_AddString( hwndCombo, NAME_SPECIAL_TOP );

	/* 種別の先頭の項目を選択（コンボボックス）*/
	Combo_SetCurSel( hwndCombo, 0 );

	// ワーク、TreeViewの初期化
	msMenu.clear();
	nMenuCnt = 0;

	hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );
	TreeView_DeleteAllItems( hwndTreeRes );

	// アクセスキーを( )付で表示
	hwndCheck = ::GetDlgItem( hwndDlg, IDC_CHECK_KEY_PARENTHESES );
	BtnCtl_SetCheck( hwndCheck, m_Common.m_sMainMenu.m_bMainMenuKeyParentheses );

	/* メニュー項目一覧と内部データをセット（TreeView）*/
	nCurLevel = 0;
	htiParent = TVI_ROOT;
	htiItem = TreeView_GetRoot( hwndTreeRes );
	for (i = 0; i < m_Common.m_sMainMenu.m_nMainMenuNum; i++) {
		pcFunc = &pcMenuTBL[i];
		if (pcFunc->m_nLevel < nCurLevel) {
			// Level Up
			for (; pcFunc->m_nLevel < nCurLevel; nCurLevel--) {
				htiParent = (htiParent == TVI_ROOT) ? TVI_ROOT : TreeView_GetParent( hwndTreeRes, htiParent );
				if (htiParent == NULL)		htiParent = TVI_ROOT;
			}
		}
		else if (pcFunc->m_nLevel > nCurLevel) {
			// Level Down
			for ( htiParent = htiItem, nCurLevel++; pcFunc->m_nLevel < nCurLevel; nCurLevel++) {
				// 実行されることは無いはず（データが正常ならば）
				htiParent = TreeView_GetChild( hwndTreeRes, htiItem );
				if (htiParent == NULL)		htiParent = htiItem;
			}
		}

		// 内部データを作成
		pFuncWk = &msMenu[nMenuCnt];
		pFuncWk->m_nFunc = pcFunc->m_nFunc;
		switch (pcFunc->m_nType) {
			case T_LEAF:
				m_cLookup.Funccode2Name( pcFunc->m_nFunc, szLabel, MAX_MAIN_MENU_NAME_LEN );
				pFuncWk->m_sName = szLabel;
				break;
			case T_SEPARATOR:
				pFuncWk->m_sName = DEFAULT_SEPA;
				break;
			case T_SPECIAL:
				pFuncWk->m_sName = pcFunc->m_sName;
				if (pFuncWk->m_sName.empty()) {
					for (j = 0; j < _countof(sSpecialFuncs); j++) {
						if (pcFunc->m_nFunc == sSpecialFuncs[j].m_nFunc) {
							pFuncWk->m_sName = RemoveAmpersand( sSpecialFuncs[j].m_sName );
							break;
						}
					}
				}
				break;
			default:
				pFuncWk->m_sName = RemoveAmpersand( pcFunc->m_sName );
				break;
		}
		auto_strcpy(pFuncWk->m_sKey, pcFunc->m_sKey);
		pFuncWk->m_bDupErr = false;
		// TreeViewに挿入
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.hParent = htiParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
		tvis.item.lParam = nMenuCnt++;								// 内部データインデックスのインクリメント
		tvis.item.cChildren = ( pcFunc->m_nType == T_NODE );
		htiItem = TreeView_InsertItem( hwndTreeRes, &tvis );
	}
}

/* ダイアログデータの取得 MainMenu */
int CPropMainMenu::GetData( HWND hwndDlg )
{
	HWND			hwndTreeRes;
	HWND			hwndCheck;
	HTREEITEM		htiItem;

	// アクセスキーを( )付で表示
	hwndCheck = ::GetDlgItem( hwndDlg, IDC_CHECK_KEY_PARENTHESES );
	m_Common.m_sMainMenu.m_bMainMenuKeyParentheses = (BtnCtl_GetCheck( hwndCheck ) != 0);

	// メニュートップ項目をセット
	m_Common.m_sMainMenu.m_nMainMenuNum = 0;
	memset( m_Common.m_sMainMenu.m_nMenuTopIdx, -1, sizeof(m_Common.m_sMainMenu.m_nMenuTopIdx) );

	hwndTreeRes = ::GetDlgItem( hwndDlg, IDC_TREE_RES );

	htiItem = TreeView_GetRoot( hwndTreeRes );
	GetDataTree( hwndTreeRes, htiItem, 0);

	return TRUE;
}

/* ダイアログデータの取得 TreeViewの 1 level */
bool CPropMainMenu::GetDataTree( HWND hwndTree, HTREEITEM htiTrg, int nLevel )
{
	static	bool	bOptionOk;
	CMainMenu*		pcMenuTBL = m_Common.m_sMainMenu.m_cMainMenuTbl;
	CMainMenu*		pcFunc;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;			// 取得用
	SMainMenuWork*	pFuncWk;		// 機能(work)
	int 			nTopCount = 0;

	if (nLevel == 0) {
		// 共通設定フラグ
		bOptionOk = false;
	}

	for (s = htiTrg; s != NULL; s = TreeView_GetNextSibling( hwndTree, s )) {
		if (m_Common.m_sMainMenu.m_nMainMenuNum >= MAX_MAINMENU) {
			// 登録数 over
			return false;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (!TreeView_GetItem( hwndTree, &tvi )) {
			// Error
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];

		if (nLevel == 0) {
			if (nTopCount >= MAX_MAINMENU_TOP) {
				continue;
			}
			// Top Levelの記録
			m_Common.m_sMainMenu.m_nMenuTopIdx[nTopCount++] = m_Common.m_sMainMenu.m_nMainMenuNum;
		}
		pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];

		switch(pFuncWk->m_nFunc) {
		case F_NODE:
			pcFunc->m_nType = T_NODE;
			auto_strcpy_s( pcFunc->m_sName, MAX_MAIN_MENU_NAME_LEN+1, SupplementAmpersand( pFuncWk->m_sName ).c_str() );
			break;
		case F_SEPARATOR:
			pcFunc->m_nType = T_SEPARATOR;
			pcFunc->m_sName[0] = L'\0';
			break;
		default:
			if (pFuncWk->m_nFunc >= F_SPECIAL_FIRST && pFuncWk->m_nFunc <= F_SPECIAL_LAST) {
				pcFunc->m_nType = T_SPECIAL;
				if (nLevel == 0) {
					auto_strcpy_s( pcFunc->m_sName, MAX_MAIN_MENU_NAME_LEN+1, SupplementAmpersand( pFuncWk->m_sName ).c_str() );
				}
				else {
					pcFunc->m_sName[0] = L'\0';
				}
			}
			else {
				if (pFuncWk->m_nFunc == F_OPTION) {
					bOptionOk = true;
				}
				pcFunc->m_nType = T_LEAF;
				pcFunc->m_sName[0] = L'\0';
			}
			break;
		}
		pcFunc->m_nFunc = pFuncWk->m_nFunc;
		auto_strcpy( pcFunc->m_sKey, pFuncWk->m_sKey );
		pcFunc->m_nLevel = nLevel;

		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	子の取得
			if (ts != NULL) {
				if (!GetDataTree( hwndTree, ts, nLevel+1 )) {
					return false;
				}
			}
		}
	}

	if (nLevel == 0 && !bOptionOk) {
		// 共通設定が無い
		if (nTopCount < MAX_MAINMENU_TOP && m_Common.m_sMainMenu.m_nMainMenuNum+1 < MAX_MAINMENU) {
			// Top Levelの記録
			m_Common.m_sMainMenu.m_nMenuTopIdx[nTopCount++] = m_Common.m_sMainMenu.m_nMainMenuNum;
			// Top Levelの追加（ダミー）
			pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];
			pcFunc->m_nType = T_NODE;
			pcFunc->m_nFunc = F_NODE;
			auto_strcpy( pcFunc->m_sName, L"auto_add" );
			pcFunc->m_sKey[0] = L'\0';
			pcFunc->m_nLevel = nLevel++;
		}
		else {
			// 末尾に追加を指定
			nLevel = 1;
		}
		if (m_Common.m_sMainMenu.m_nMainMenuNum < MAX_MAINMENU) {
			// 共通設定
			pcFunc = &pcMenuTBL[m_Common.m_sMainMenu.m_nMainMenuNum++];
			pcFunc->m_nType = T_LEAF;
			pcFunc->m_nFunc = F_OPTION;
			pcFunc->m_sName[0] = L'\0';
			pcFunc->m_sKey[0] = L'\0';
			pcFunc->m_nLevel = nLevel;
		}
		else {
			// 登録数 over
			return false;
		}
	}

	return true;
}



/* メインメニュー設定をインポートする */
void CPropMainMenu::Import( HWND hwndDlg )
{
	CImpExpMainMenu	cImpExp( m_Common );

	// インポート
	if (!cImpExp.ImportUI( G_AppInstance(), hwndDlg )) {
		// インポートをしていない
		return;
	}
	SetData( hwndDlg );
}

/* メインメニュー設定をエクスポートする */
void CPropMainMenu::Export( HWND hwndDlg )
{
	CImpExpMainMenu	cImpExp( m_Common );

	GetData( hwndDlg );

	// エクスポート
	if (!cImpExp.ExportUI( G_AppInstance(), hwndDlg )) {
		// エクスポートをしていない
		return;
	}
}



// ツリーのコピー
//		fChildがtrueの時はdstの子としてコピー, そうでなければdstの兄弟としてdstの後ろにコピー
//		fOnryOneがtrueの時は1つだけコピー（子があったらコピー）
static HTREEITEM TreeCopy( HWND hwndTree, HTREEITEM dst, HTREEITEM src, bool fChild, bool fOnryOne )
{
	HTREEITEM		s;
	HTREEITEM		ts;
	HTREEITEM		td = NULL;
	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	int				n = 0;
	TCHAR			szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	for (s = src; s != NULL; s = fOnryOne ? NULL:TreeView_GetNextSibling( hwndTree, s )) {
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		tvi.pszText = szLabel;
		tvi.cchTextMax = MAX_MAIN_MENU_NAME_LEN;
		if (!TreeView_GetItem( hwndTree, &tvi )) {
			// Error
			break;
		}
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		if (fChild || n != 0) {
			// dstの子供として作成
			tvis.hParent = dst;
			tvis.hInsertAfter = TVI_LAST;
		}
		else {
			//	dstの兄弟として作成
			tvis.hParent = TreeView_GetParent( hwndTree, dst );
			tvis.hInsertAfter = dst;
		}
		tvis.item.pszText = szLabel;
		tvis.item.lParam = tvi.lParam;
		tvis.item.cChildren = tvi.cChildren;
		td = TreeView_InsertItem( hwndTree, &tvis );	//	Itemの作成

		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	子の取得
			if (ts != NULL) {
				TreeCopy( hwndTree, td, ts, true, false );
			}
			// 展開
			if (tvi.state & TVIS_EXPANDEDONCE) {
				TreeView_Expand( hwndTree, td, TVE_EXPAND );
			}
		}
		n++;
	}

	return td;
}

// TreeView 全開･全閉
static void TreeView_ExpandAll( HWND hwndTree, bool bExpand )
{
	std::map<int, HTREEITEM>	htiStack;
	HTREEITEM	htiCur;
	HTREEITEM	htiItem;
	HTREEITEM	htiNext;
	int			nLevel;

	nLevel = 0;
	htiCur = htiItem = TreeView_GetSelection( hwndTree );
	if (!bExpand && htiCur != NULL) {
		// 閉じる時はトップに変更
		for (htiNext = htiCur; htiNext !=  NULL; ) {
			htiItem = htiNext;
			htiNext = TreeView_GetParent( hwndTree, htiItem );
		}
		if (htiCur != htiItem) {
			htiCur = htiItem;
			TreeView_SelectItem( hwndTree, htiCur );
		}
	}


	for (htiItem = TreeView_GetRoot( hwndTree ); htiItem != NULL; htiItem = htiNext) {
		htiNext = TreeView_GetChild( hwndTree, htiItem);
		if (htiNext != NULL) {
			TreeView_Expand( hwndTree, htiItem, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
			// 子の開閉
			htiStack[nLevel++] = htiItem;
		}
		else {
			htiNext = TreeView_GetNextSibling( hwndTree, htiItem);
			while (htiNext == NULL && nLevel > 0) {
				htiItem = htiStack[--nLevel];
				htiNext = TreeView_GetNextSibling( hwndTree, htiItem);
			}
		}
	}
	// 選択位置を戻す
	if (htiCur == NULL) {
		if (bExpand ) {
			htiItem = TreeView_GetRoot( hwndTree );
			TreeView_SelectSetFirstVisible( hwndTree, htiItem );
		}
		TreeView_SelectItem( hwndTree, NULL );
	}
	else {
		TreeView_SelectSetFirstVisible( hwndTree, htiCur );
	}
}



// 表示用データの作成（アクセスキー付加）
static const TCHAR* MakeDispLabel( SMainMenuWork* pFunc )
{
	static	WCHAR	szLabel[MAX_MAIN_MENU_NAME_LEN + 10];

	if (pFunc->m_sKey[0]) {
		auto_sprintf_s( szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls(%ls)",
			pFunc->m_bDupErr ? L">" : L"",
			pFunc->m_sName.substr(0, MAX_MAIN_MENU_NAME_LEN).c_str(), pFunc->m_sKey);
	}
	else {
		auto_sprintf_s( szLabel, MAX_MAIN_MENU_NAME_LEN + 10, L"%ls%ls",
			pFunc->m_bDupErr ? L">" : L"",
			pFunc->m_sName.substr(0, MAX_MAIN_MENU_NAME_LEN).c_str() );
	}

	return to_tchar( szLabel );
}



// メニューの検査
bool CPropMainMenu::Check_MainMenu( 
	HWND	hwndTree,		// handle to TreeView
	wstring&	sErrMsg			// エラーメッセージ
)
{
	bool			bRet = false;
	HTREEITEM		htiItem;
	
	sErrMsg = L"";
	
	htiItem = TreeView_GetRoot( hwndTree );

	bRet = Check_MainMenu_Sub( hwndTree, htiItem, 0, sErrMsg );
	return bRet;
}

// メニューの検査 TreeViewの 1 level
bool CPropMainMenu::Check_MainMenu_Sub(
	HWND		hwndTree,		// handle to dialog box
	HTREEITEM 	htiTrg,			// ターゲット
	int 		nLevel,
	wstring&	sErrMsg )
{
	// 検査用
	static	bool		bOptionOk;		// 「共通設定」
	static	int 		nMenuNum;		// メニュー項目数		最大 MAX_MAINMENU
	static	int 		nTopNum;		// トップレベル項目数	最大 MAX_MAINMENU_TOP
	static	int 		nDupErrNum;		// 重複エラー個数
	static	int 		nNoSetErrNum;	// 未設定エラー個数
	static	HTREEITEM	htiErr;
	//
	bool			bRet = true;
	EMainMenuType	nType;
	HTREEITEM		s;
	HTREEITEM		ts;
	TV_ITEM			tvi;							// 取得用
	SMainMenuWork*	pFuncWk;						// 機能(work)
	std::map< WCHAR, HTREEITEM >	mKey;			// 重複エラー検出用
	std::map< WCHAR, HTREEITEM >::iterator itKey;	// 同上

	if (nLevel == 0) {
		bOptionOk = false;
		nMenuNum = nTopNum = nDupErrNum = nNoSetErrNum = 0;
		htiErr = NULL;
	}
	mKey.clear();

	for (s = htiTrg; s != NULL; s = TreeView_GetNextSibling( hwndTree, s )) {
		// メニュー数のカウント
		nMenuNum++;
		if (nLevel == 0) {
			nTopNum++;
		}
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if (!TreeView_GetItem( hwndTree, &tvi )) {
			// Error
			sErrMsg = L"メニュー設定の取得に失敗しました";
			return false;
		}
		pFuncWk = &msMenu[tvi.lParam];
		switch (pFuncWk->m_nFunc) {
		case F_NODE:
			nType = T_NODE;
			break;
		case F_SEPARATOR:
			nType = T_SEPARATOR;
			break;
		default:
			if (pFuncWk->m_nFunc >= F_SPECIAL_FIRST && pFuncWk->m_nFunc <= F_SPECIAL_LAST) {
				nType = T_SPECIAL;
			}
			else {
				if (pFuncWk->m_nFunc == F_OPTION) {
					bOptionOk = true;
				}
				nType = T_LEAF;
			}
			break;
		}
		if (pFuncWk->m_sKey[0] == '\0') {
			if (nType == T_NODE || nType == T_LEAF) {
				// 未設定
				if (nNoSetErrNum == 0) {
					if (htiErr == NULL) {
						htiErr = s;
					}
				}
				TreeView_SelectItem( hwndTree, s );
				nNoSetErrNum++;
			}
		}
		else {
			itKey = mKey.find( pFuncWk->m_sKey[0] );
			if (itKey == mKey.end()) {
				mKey[pFuncWk->m_sKey[0]] = s;

				if (pFuncWk->m_bDupErr) {
					// 目印クリア
					pFuncWk->m_bDupErr = false;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText =  const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
					TreeView_SetItem( hwndTree , &tvi );		//	キー設定結果を反映
				}
			}
			else {
				// 重複エラー
				if (nDupErrNum == 0) {
					if (htiErr == NULL) {
						htiErr = mKey[pFuncWk->m_sKey[0]];
					}
				}
				TreeView_SelectItem( hwndTree, mKey[pFuncWk->m_sKey[0]] );

				nDupErrNum++;

				// 目印設定
				pFuncWk->m_bDupErr = true;
				tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
				tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( pFuncWk ) );
				TreeView_SetItem( hwndTree , &tvi );		//	キー設定結果を反映

				// 目印設定（元分）
				tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
				tvi.hItem = mKey[pFuncWk->m_sKey[0]];
				if (!TreeView_GetItem( hwndTree, &tvi )) {
					// Error
					sErrMsg = L"メニュー設定の取得に失敗しました";
					return false;
				}
				if (!msMenu[tvi.lParam].m_bDupErr) {
					msMenu[tvi.lParam].m_bDupErr = true;
					tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
					tvi.pszText = const_cast<TCHAR*>( MakeDispLabel( &msMenu[tvi.lParam] ) );
					TreeView_SetItem( hwndTree , &tvi );		//	キー設定結果を反映
				}
			}
		}
		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	子の取得
			if (ts != NULL) {
				if (!Check_MainMenu_Sub( hwndTree, ts, nLevel+1, sErrMsg )) {
					// 内部エラー
					return false;
				}
			}
		}
	}

	if (nLevel == 0) {
		sErrMsg = L"";
		if (!bOptionOk) {
			sErrMsg += L"『共通設定』が有りません。\n";
			bRet = false;
		}
		if (nTopNum > MAX_MAINMENU_TOP) {
			sErrMsg += L"トップレベルの項目数が多すぎます。\n";
			bRet = false;
		}
		if (nMenuNum > MAX_MAINMENU) {
			sErrMsg += L"登録項目数が多すぎます。\n";
			bRet = false;
		}
		if (nDupErrNum > 0) {
			sErrMsg += L"重複したアクセスキーがあります。\n";
			bRet = false;
		}
		if (nNoSetErrNum > 0) {
			sErrMsg += L"未設定のアクセスキーがあります。\n";
			bRet = false;
		}
		if (htiErr != NULL) {
			TreeView_SelectItem( hwndTree, htiErr );
			TreeView_SelectSetFirstVisible( hwndTree, htiErr );
		}
	}
	return bRet;
}
