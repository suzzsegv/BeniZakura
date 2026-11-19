/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002-2003, MIK
	Copyright (C) 2006, ryoji

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
#include "dlg/CDlgCtrlCode.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "sakura_rc.h"


struct ctrl_info_t {
	wchar_t			code;		//入力する文字コード
	unsigned int	vKey;		//表記
	WCHAR			name[4];	//名前
	const WCHAR		*jname;		//説明
} static const p_ctrl_list[] = {
	{ 0x0000, 0x00c0, L"NUL", L"空文字"    }, //NULL
	{ 0x0001, 'A', L"SOH", L"ヘッダ開始"   }, //START OF HEADING
	{ 0x0002, 'B', L"STX", L"テキスト開始" }, //START OF TEXT
	{ 0x0003, 'C', L"ETX", L"テキスト終了" }, //END OF TEXT
	{ 0x0004, 'D', L"EOT", L"転送終了"     }, //END OF TRANSMISSION
	{ 0x0005, 'E', L"ENQ", L"照会"         }, //ENQUIRY
	{ 0x0006, 'F', L"ACK", L"受信OK"       }, //ACKNOWLEDGE
	{ 0x0007, 'G', L"BEL", L"警告(ベル)"   }, //BELL
	{ 0x0008, 'H', L"BS",  L"後退"         }, //BACKSPACE
	{ 0x0009, 'I', L"HT",  L"タブ"         }, //horizontal tabulation (HT)
	{ 0x000a, 'J', L"LF",  L"改行"         },	//LINE FEED (LF); new line (NL); end of line(EOL)
	{ 0x000b, 'K', L"VT",  L"垂直タブ"     }, //vertical tabulation (VT)
	{ 0x000c, 'L', L"FF",  L"改ページ"     },	//FORM FEED (FF)
	{ 0x000d, 'M', L"CR",  L"復帰"         }, //CARRIAGE RETURN
	{ 0x000e, 'N', L"SO",  L"シフトアウト" }, //SHIFT OUT
	{ 0x000f, 'O', L"SI",  L"シフトイン"   }, //SHIFT IN
	{ 0x0010, 'P', L"DLE", L"データリンクエスケープ" }, //DATA LINK ESCAPE
	{ 0x0011, 'Q', L"DC1", L"装置制御1"    }, //DEVICE CONTROL ONE
	{ 0x0012, 'R', L"DC2", L"装置制御2"    }, //DEVICE CONTROL TWO
	{ 0x0013, 'S', L"DC3", L"装置制御3"    }, //DEVICE CONTROL THREE
	{ 0x0014, 'T', L"DC4", L"装置制御4"    }, //DEVICE CONTROL FOUR
	{ 0x0015, 'U', L"NAK", L"受信失敗"     }, //NEGATIVE ACKNOWLEDGE
	{ 0x0016, 'V', L"SYN", L"同期"         }, //SYNCHRONOUS IDLE
	{ 0x0017, 'W', L"ETB", L"転送ブロック終了" }, //END OF TRANSMISSION BLOCK
	{ 0x0018, 'X', L"CAN", L"キャンセル"   }, //CANCEL
	{ 0x0019, 'Y', L"EM",  L"メディア終了" }, //END OF MEDIUM
	{ 0x001a, 'Z', L"SUB", L"置換"         }, //SUBSTITUTE
	{ 0x001b, 0x00db, L"ESC", L"エスケープ"   }, //ESCAPE
	{ 0x001c, 0x00dc, L"FS",  L"フォーム区切" }, //file separator (FS)
	{ 0x001d, 0x00dd, L"GS",  L"グループ区切" }, //group separator (GS)
	{ 0x001e, 0x00de, L"RS",  L"レコード区切" }, //record separator (RS)
	{ 0x001f, 0x00e2, L"US",  L"ユニット区切" }, //unit separator (US)
	{ 0x007f, 0x00bf, L"DEL", L"削除"         }, //DELETE

	//internal data
	{ 0x001f, 0x00df, L"US",  NULL }	//PC98 "_"  //unit separator (US)
};
// Feb. 12, 2003 MIK longが抜けていた

CDlgCtrlCode::CDlgCtrlCode()
{
	m_nCode = L'\0';
}

/* モーダルダイアログの表示 */
int CDlgCtrlCode::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_CTRLCODE, lParam );
}

/* ダイアログデータの設定 */
void CDlgCtrlCode::SetData( void )
{
	HWND	hwndWork;
	int		i, count;
	long	lngStyle;
	LV_ITEM	lvi;

	/* リスト */
	hwndWork = ::GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	ListView_DeleteAllItems( hwndWork );  /* リストを空にする */

	/* 行選択 */
	lngStyle = ListView_GetExtendedListViewStyle( hwndWork );
	lngStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, lngStyle );

	/* データ表示 */
	WCHAR	tmp[10];
	count = 0;
	for( i = 0; i < _countof(p_ctrl_list); i++ )
	{
		if( p_ctrl_list[i].jname == NULL ) continue;
		
		auto_sprintf( tmp, L"0x%02X", p_ctrl_list[i].code );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = count;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndWork, &lvi );
		
		if( p_ctrl_list[i].code <= 0x1f )
			auto_sprintf( tmp, L"^%tc", L'@' + p_ctrl_list[i].code );
		else if( p_ctrl_list[i].code == 0x7f )
			wcscpy( tmp, L"^?" );
		else
			wcscpy( tmp, L"･" );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 2;
		lvi.pszText  = const_cast<WCHAR*>(p_ctrl_list[i].name);
		ListView_SetItem( hwndWork, &lvi );
		
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = count;
		lvi.iSubItem = 3;
		lvi.pszText  = const_cast<WCHAR*>(p_ctrl_list[i].jname);
		ListView_SetItem( hwndWork, &lvi );
		
		count++;
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgCtrlCode::GetData( void )
{
	int		nIndex;
	HWND	hwndList;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
	//選択中のキー番号を探す。
	nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
	if( nIndex == -1 ) return FALSE;

	m_nCode = p_ctrl_list[nIndex].code;

	return TRUE;
}

BOOL CDlgCtrlCode::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	LV_COLUMN	col;
	RECT		rc;

	_SetHwnd( hwndDlg );

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_CTRLCODE );
	::GetWindowRect( hwndList, &rc );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = L"コード";
	col.iSubItem = 0;
	ListView_InsertColumn( hwndList, 0, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = L"表記";
	col.iSubItem = 1;
	ListView_InsertColumn( hwndList, 1, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 15 / 100;
	col.pszText  = L"名前";
	col.iSubItem = 2;
	ListView_InsertColumn( hwndList, 2, &col );

	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = (rc.right - rc.left) * 46 / 100;
	col.pszText  = L"説明";
	col.iSubItem = 3;
	ListView_InsertColumn( hwndList, 3, &col );

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgCtrlCode::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDOK:			/* 左右に表示 */
		/* ダイアログデータの取得 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

#ifdef __MINGW32__
typedef struct tagNMKEY {
  NMHDR hdr;
  UINT  nVKey;
  UINT  uFlags;
} NMKEY, *LPNMKEY;
#endif

BOOL CDlgCtrlCode::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*	pNMHDR;
	HWND	hwndList;

	pNMHDR = (NMHDR*)lParam;

	hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );

	if( hwndList == pNMHDR->hwndFrom )
	{
		switch( pNMHDR->code )
		{
		case NM_DBLCLK:
			::EndDialog( GetHwnd(), GetData() );
			return TRUE;

		case LVN_KEYDOWN:
			{
				HWND	hwndList;
				NMKEY	*p = (NMKEY*)lParam;
				int		i, j;
				unsigned int	c;
				for( i = 0; i < _countof(p_ctrl_list); i++ )
				{
					c = p_ctrl_list[i].vKey;
					if( c == (p->nVKey & 0xffff) )
					{
						for( j = 0; j < _countof(p_ctrl_list); j++ )
						{
							if( p_ctrl_list[i].code == p_ctrl_list[j].code )
							{
								hwndList = GetDlgItem( GetHwnd(), IDC_LIST_CTRLCODE );
								ListView_SetItemState( hwndList, j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
								ListView_EnsureVisible( hwndList, j, FALSE );
						
								/* ダイアログデータの取得 */
								::EndDialog( GetHwnd(), GetData() );
								return TRUE;
							}
						}
						break;
					}
				}
			}
			break;
		}
	}

	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

