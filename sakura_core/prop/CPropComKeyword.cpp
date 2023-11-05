/*!	@file
	@brief 共通設定ダイアログボックス、「強調キーワード」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, KEITA
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "dlg/CDlgInput1.h"
#include "util/shell.h"
#include "sakura_rc.h"


//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropKeyword::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropKeyword::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta
INT_PTR CALLBACK CPropKeyword::DlgProc_dialog(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc2( reinterpret_cast<pDispatchPage>(&CPropKeyword::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/* Keyword メッセージ処理 */
INT_PTR CPropKeyword::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	int					nIndex1;
	int					i;
	static int			nListItemHeight;
	LV_COLUMN			lvc;
	LV_ITEM*			plvi;
	static HWND			hwndCOMBO_SET;
	static HWND			hwndLIST_KEYWORD;
	RECT				rc;
	CDlgInput1			cDlgInput1;
	wchar_t				szKeyWord[MAX_KEYWORDLEN + 1];
	DWORD				dwStyle;
	LV_DISPINFO*		plvdi;
	LV_KEYDOWN*			pnkd;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 Keyword */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		if( wParam == IDOK ){ // 独立ウィンドウ
			hwndCtl = ::GetDlgItem( hwndDlg, IDOK );
			GetWindowRect( hwndCtl, &rc );
			i = rc.bottom; // OK,CANCELボタンの下端

			GetWindowRect( hwndDlg, &rc );
			SetWindowPos( hwndDlg, NULL, 0, 0, rc.right-rc.left, i-rc.top+10, SWP_NOZORDER|SWP_NOMOVE );
			SetWindowText( hwndDlg, _T("共通設定 - 強調キーワード") );

			hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
			Combo_SetCurSel( hwndCOMBO_SET, m_nKeywordSet1 );
		}
		else{
			hwndCtl = ::GetDlgItem( hwndDlg, IDOK );
			ShowWindow( hwndCtl, SW_HIDE );
			hwndCtl = ::GetDlgItem( hwndDlg, IDCANCEL );
			ShowWindow( hwndCtl, SW_HIDE );
		}

		/* コントロールのハンドルを取得 */
		hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
		hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
		::GetWindowRect( hwndLIST_KEYWORD, &rc );
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = rc.right - rc.left;
		lvc.pszText = _T("");
		lvc.iSubItem = 0;
		ListView_InsertColumn( hwndLIST_KEYWORD, 0, &lvc );

		dwStyle = (DWORD)::GetWindowLong( hwndLIST_KEYWORD, GWL_STYLE );
		::SetWindowLong( hwndLIST_KEYWORD, GWL_STYLE, dwStyle | LVS_SHOWSELALWAYS );
//				(dwStyle & ~LVS_TYPEMASK) | dwView);


		/* コントロール更新のタイミング用のタイマーを起動 */
		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pnkd = (LV_KEYDOWN *)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		plvdi = (LV_DISPINFO*)lParam;
		plvi = &plvdi->item;

		if( hwndLIST_KEYWORD == pNMHDR->hwndFrom ){
			switch( pNMHDR->code ){
			case NM_DBLCLK:
//				MYTRACE( _T("NM_DBLCLK     \n") );
				/* リスト中で選択されているキーワードを編集する */
				Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
				return TRUE;
			case LVN_BEGINLABELEDIT:
#ifdef _DEBUG
				MYTRACE( _T("LVN_BEGINLABELEDIT\n") );
												MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
												MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
												MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
												MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText );
												MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
				return TRUE;
			case LVN_ENDLABELEDIT:
#ifdef _DEBUG
				MYTRACE( _T("LVN_ENDLABELEDIT\n") );
												MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
												MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
												MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
												MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText  );
												MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
				if( NULL == plvi->pszText ){
					return TRUE;
				}
				if( plvi->pszText[0] != _T('\0') ){
					if( MAX_KEYWORDLEN < _tcslen( plvi->pszText ) ){
						InfoMessage( hwndDlg, _T("キーワードの長さは%dバイトまでです。"), MAX_KEYWORDLEN );
						return TRUE;
					}
					/* ｎ番目のセットにキーワードを編集 */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
						plvi->lParam,
						to_wchar(plvi->pszText)
					);
				}else{
					/* ｎ番目のセットのｍ番目のキーワードを削除 */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam );
				}
				/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
				SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

				ListView_SetItemState( hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

				return TRUE;
			case LVN_KEYDOWN:
//				MYTRACE( _T("LVN_KEYDOWN\n") );
				switch( pnkd->wVKey ){
				case VK_DELETE:
					/* リスト中で選択されているキーワードを削除する */
					Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					break;
				case VK_SPACE:
					/* リスト中で選択されているキーワードを編集する */
					Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					break;
				}
				return TRUE;
			}
		}else{
			switch( pNMHDR->code ){
			case PSN_KILLACTIVE:
				DEBUG_TRACE( _T("Keyword PSN_KILLACTIVE\n") );
				/* ダイアログデータの取得 Keyword */
				GetData( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_KEYWORD;
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);			/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;		/* コントロールのハンドル */
		if( hwndCOMBO_SET == hwndCtl){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex1 = Combo_GetCurSel( hwndCOMBO_SET );
				/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
				SetKeyWordSet( hwndDlg, nIndex1 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_ADDSET:	/* セット追加 */
					if( MAX_SETNUM <= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
						InfoMessage( hwndDlg, _T("セットは%d個までしか登録できません。\n"), MAX_SETNUM );
						return TRUE;
					}
					/* モードレスダイアログの表示 */
					wcscpy( szKeyWord, L"" );
					//	Oct. 5, 2002 genta 長さ制限の設定を修正．バッファオーバーランしていた．
					if( !cDlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						_T("キーワードのセット追加"),
						_T("セット名を入力してください。"),
						MAX_SETNAMELEN,
						szKeyWord
						)
					){
						return TRUE;
					}
					if( szKeyWord[0] != L'\0' ){
						/* セットの追加 */
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( szKeyWord, false );

						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum - 1;

						/* ダイアログデータの設定 Keyword */
						SetData( hwndDlg );
					}
					return TRUE;
				case IDC_BUTTON_DELSET:	/* セット削除 */
					nIndex1 = Combo_GetCurSel( hwndCOMBO_SET );
					if( CB_ERR == nIndex1 ){
						return TRUE;
					}
					/* 削除対象のセットを使用しているファイルタイプを列挙 */
					static TCHAR		pszLabel[1024];
					_tcscpy( pszLabel, _T("") );
					for( i = 0; i < MAX_TYPES; ++i ){
						CTypeConfig type(i);
						// 2002/04/25 YAZAKI STypeConfig全体を保持する必要はないし、m_pShareDataを直接見ても問題ない。
						if( nIndex1 == m_Types_nKeyWordSetIdx[i][0]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][1]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][2]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][3]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][4]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][5]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][6]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][7]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][8]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][9] ){
							_tcscat( pszLabel, _T("・") );
							_tcscat( pszLabel, type->m_szTypeName );
							_tcscat( pszLabel, _T("（") );
							_tcscat( pszLabel, type->m_szTypeExts );
							_tcscat( pszLabel, _T("）") );
							_tcscat( pszLabel, _T("\n") );
						}
					}
					if( IDCANCEL == ::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("「%ls」のセットを削除します。\nよろしいですか？\n削除しようとするセットは、以下のファイルタイプに割り当てられています。\n削除したセットは無効になります。\n\n%ts"),
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIndex1 ),
						pszLabel
					) ){
						return TRUE;
					}
					/* 削除対象のセットを使用しているファイルタイプのセットをクリア */
					for( i = 0; i < MAX_TYPES; ++i ){
						// 2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
						for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
							if( nIndex1 == m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j] = -1;
							}
							else if( nIndex1 < m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j]--;
							}
						}
					}
					/* ｎ番目のセットを削除 */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
					/* ダイアログデータの設定 Keyword */
					SetData( hwndDlg );
					return TRUE;
				case IDC_BUTTON_KEYSETRENAME: // キーワードセットの名称変更
					// モードレスダイアログの表示
					wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
					{
						BOOL bDlgInputResult = cDlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							_T("セットの名称変更"),
							_T("セット名を入力してください。"),
							MAX_SETNAMELEN,
							szKeyWord
						);
						if( !bDlgInputResult ){
							return TRUE;
						}
					}
					if( szKeyWord[0] != L'\0' ){
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord );

						// ダイアログデータの設定 Keyword
						SetData( hwndDlg );
					}
					return TRUE;
				case IDC_CHECK_KEYWORDCASE:	/* キーワードの英大文字小文字区別 */
//					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_bKEYWORDCASEArr[ m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ] = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );	//MIK 2000.12.01 case sense
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE ));			//MIK 2000.12.01 case sense
					return TRUE;
				case IDC_BUTTON_ADDKEYWORD:	/* キーワード追加 */
					/* ｎ番目のセットのキーワードの数を返す */
					if( !m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CanAddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
						InfoMessage( hwndDlg, _T("登録できるキーワード数が上限に達しています。\n") );
						return TRUE;
					}
					/* モードレスダイアログの表示 */
					wcscpy( szKeyWord, L"" );
					if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, _T("キーワード追加"), _T("キーワードを入力してください。"), MAX_KEYWORDLEN, szKeyWord ) ){
						return TRUE;
					}
					if( szKeyWord[0] != L'\0' ){
						/* ｎ番目のセットにキーワードを追加 */
						if( 0 == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord ) ){
							// ダイアログデータの設定 Keyword 指定キーワードセットの設定
							SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
						}
					}
					return TRUE;
				case IDC_BUTTON_EDITKEYWORD:	/* キーワード編集 */
					/* リスト中で選択されているキーワードを編集する */
					Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_DELKEYWORD:	/* キーワード削除 */
					/* リスト中で選択されているキーワードを削除する */
					Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// From Here 2005.01.26 Moca
				case IDC_BUTTON_KEYCLEAN:
					Clean_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// To Here 2005.01.26 Moca
				case IDC_BUTTON_IMPORT:	/* インポート */
					/* リスト中のキーワードをインポートする */
					Import_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_EXPORT:	/* エクスポート */
					/* リスト中のキーワードをエクスポートする */
					Export_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// 独立ウィンドウで使用する
				case IDOK:
					EndDialog( hwndDlg, IDOK );
					break;
				case IDCANCEL:
					EndDialog( hwndDlg, IDCANCEL );
					break;
				}
				break;	/* BN_CLICKED */
			}
		}
		break;	/* WM_COMMAND */

	case WM_TIMER:
		nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nIndex1 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), TRUE );
		}
		break;

	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
		break;

	}
	return FALSE;
}

/* リスト中で選択されているキーワードを編集する */
void CPropKeyword::Edit_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	int			nIndex1;
	LV_ITEM		lvi;
	wchar_t		szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1	cDlgInput1;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );

	/* ｎ番目のセットのｍ番目のキーワードを返す */
	wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam ) );

	/* モードレスダイアログの表示 */
	if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, _T("キーワード編集"), _T("キーワードを編集してください。"), MAX_KEYWORDLEN, szKeyWord ) ){
		return;
	}
	if( szKeyWord[0] != L'\0' ){
		/* ｎ番目のセットにキーワードを編集 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
			lvi.lParam,
			szKeyWord
		);
	}else{
		/* ｎ番目のセットのｍ番目のキーワードを削除 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	}
	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}



/* リスト中で選択されているキーワードを削除する */
void CPropKeyword::Delete_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	int			nIndex1;
	LV_ITEM		lvi;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );
	/* ｎ番目のセットのｍ番目のキーワードを削除 */
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	//キーワード数を表示する。
	DispKeywordCount( hwndDlg );

	return;
}


/* リスト中のキーワードをインポートする */
void CPropKeyword::Import_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	bool	bCase = false;
	int		nIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase( nIdx, bCase );
	CImpExpKeyWord	cImpExpKeyWord( m_Common, nIdx, bCase );

	// インポート
	if (!cImpExpKeyWord.ImportUI( G_AppInstance(), hwndDlg )) {
		// インポートをしていない
		return;
	}

	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	return;
}


/* リスト中のキーワードをエクスポートする */
void CPropKeyword::Export_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	bool	bCase;
	CImpExpKeyWord	cImpExpKeyWord( m_Common, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, bCase );

	// エクスポート
	if (!cImpExpKeyWord.ExportUI( G_AppInstance(), hwndDlg )) {
		// エクスポートをしていない
		return;
	}
}


//! キーワードを整頓する
void CPropKeyword::Clean_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	if( IDYES == ::MessageBox( hwndDlg, _T("現在の設定では強調キーワードとして表示できないキーワードを削除しますか？"),
			GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){	// 2009.03.26 ryoji MB_ICONSTOP->MB_ICONQUESTION
		if( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CleanKeyWords( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
		}
		SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}
}

/* ダイアログデータの設定 Keyword */
void CPropKeyword::SetData( HWND hwndDlg )
{
	int		i;
	HWND	hwndWork;


	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	Combo_ResetContent( hwndWork );  /* コンボボックスを空にする */
	if( 0 < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		/* セット名コンボボックスのデフォルト選択 */
		Combo_SetCurSel( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

		/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
		SetKeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}else{
		/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
		SetKeyWordSet( hwndDlg, -1 );
	}

	return;
}


/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
void CPropKeyword::SetKeyWordSet( HWND hwndDlg, int nIdx )
{
	int		i;
	int		nNum;
	HWND	hwndList;
	LV_ITEM	lvi;

	ListView_DeleteAllItems( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ) );
	if( 0 <= nIdx ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), TRUE );
		//	Jan. 29, 2005 genta キーワードセット切り替え直後はキーワードは未選択
		//	そのため有効にしてすぐにタイマーで無効になる．
		//	なのでここで無効にしておく．
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), FALSE );
		return;
	}

	/* キーワードの英大文字小文字区別 */
	if( true == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase(nIdx) ){		//MIK 2000.12.01 case sense
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );
	}

	/* ｎ番目のセットのキーワードの数を返す */
	nNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( nIdx );
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	// 2005.01.25 Moca/genta リスト追加中は再描画を抑制してすばやく表示
	::SendMessageAny( hwndList, WM_SETREDRAW, FALSE, 0 );

	for( i = 0; i < nNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		const TCHAR* pszKeyWord = to_tchar(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( nIdx, i ));

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = const_cast<TCHAR*>(pszKeyWord);
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem( hwndList, &lvi );

	}
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nIdx;

	// 2005.01.25 Moca/genta リスト追加完了のため再描画許可
	::SendMessageAny( hwndList, WM_SETREDRAW, TRUE, 0 );

	//キーワード数を表示する。
	DispKeywordCount( hwndDlg );

	return;
}



/* ダイアログデータの取得 Keyword */
int CPropKeyword::GetData( HWND hwndDlg )
{
//	HWND	hwndResList;
//	int		i;
//	int		j;
//	int		k;

//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_KEYWORD;


	return TRUE;
}

/* ダイアログデータの取得 Keyword 指定キーワードセットの取得 */
void CPropKeyword::GetKeyWordSet( HWND hwndDlg, int nIdx )
{
}

//キーワード数を表示する。
void CPropKeyword::DispKeywordCount( HWND hwndDlg )
{
	HWND	hwndList;
	int		n;
	TCHAR szCount[ 256 ];

	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	n = ListView_GetItemCount( hwndList );
	if( n < 0 ) n = 0;

	int		nAlloc;
	nAlloc = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetAllocSize( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc -= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc += m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetFreeSize();
	
	auto_sprintf( szCount, _T("(最大 %d 文字, 登録数 %d, 空き %d 個)"), MAX_KEYWORDLEN, n, nAlloc );
	::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_KEYWORD_COUNT ), szCount );
}


