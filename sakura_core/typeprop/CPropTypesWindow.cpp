/*
	タイプ別設定 - ウィンドウ

	2008.04.12 kobake CPropTypes.cppから分離
	2009.02.22 ryoji
*/
#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "CDlgSameColor.h"
#include "CDlgKeywordSelect.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"

using namespace std;


TYPE_NAME<int> ImeSwitchArr[] = {
	{ 0, _T("そのまま") },
	{ 1, _T("常にON") },
	{ 2, _T("常にOFF") },
};

//	Nov. 20, 2000 genta
TYPE_NAME<int> ImeStateArr[] = {
	{ 0, _T("標準設定") },
	{ 1, _T("全角") },
	{ 2, _T("全角ひらがな") },
	{ 3, _T("全角カタカナ") },
	{ 4, _T("無変換") }
};

static const wchar_t* aszEolStr[] = {
	L"CR+LF",
	L"LF (UNIX)",
	L"CR (Mac)",
};
static const EEolType aeEolType[] = {
	EOL_CRLF,
	EOL_LF,
	EOL_CR,
};

/* window メッセージ処理 */
INT_PTR CPropTypesWindow::DispatchEvent(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* ダイアログデータの設定 color */
		SetData( hwndDlg );

		/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), 1 );

		return TRUE;

	case WM_COMMAND:
		wNotifyCode	= HIWORD( wParam );	/* 通知コード */
		wID			= LOWORD( wParam );	/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */

		switch( wNotifyCode ){
		case CBN_SELCHANGE:
			{
				int i;
				switch( wID ){
				case IDC_COMBO_DEFAULT_CODETYPE:
					// 文字コードの変更をBOMチェックボックスに反映
					i = Combo_GetCurSel( (HWND) lParam );
					if( CB_ERR != i ){
						CCodeTypeName	cCodeTypeName( Combo_GetItemData( (HWND)lParam, i ) );
						::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (cCodeTypeName.IsBomDefOn() ?  BST_CHECKED : BST_UNCHECKED) );
						::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), cCodeTypeName.UseBom() );
					}
					break;
				}
			}
			break;

		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_BACKIMG_PATH_SEL:
				{
					CDialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH),
						_T("*.bmp;*.jpg;*.jpeg"), true );
				}
				return TRUE;
			//	From Here Sept. 10, 2000 JEPRO
			//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
			case IDC_RADIO_LINETERMTYPE0: /* 行番号区切り 0=なし 1=縦線 2=任意 */
			case IDC_RADIO_LINETERMTYPE1:
			case IDC_RADIO_LINETERMTYPE2:
				EnableTypesPropInput( hwndDlg );
				return TRUE;
			//	To Here Sept. 10, 2000

			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_KILLACTIVE:
//			MYTRACE( _T("color PSN_KILLACTIVE\n") );
			/* ダイアログデータの取得 window */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = 2;
			return TRUE;
		}
		break;	/* WM_NOTIFY */

	}
	return FALSE;
}


void CPropTypesWindow::SetCombobox(HWND hwndWork, const TCHAR** pszLabels, int nCount, int select)
{
	Combo_ResetContent(hwndWork);
	for(int i = 0; i < nCount; ++i ){
		Combo_AddString( hwndWork, pszLabels[i] );
	}
	Combo_SetCurSel(hwndWork, select);
}



/* ダイアログデータの設定 window */
void CPropTypesWindow::SetData( HWND hwndDlg )
{
	{
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		::CheckDlgButton( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon  ? TRUE : FALSE );
	}

	//起動時のIME(日本語入力変換)	//Nov. 20, 2000 genta
	{
		int ime;
		// ON/OFF状態
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState & 3;
		int		nSelPos = 0;
		for( int i = 0; i < _countof( ImeSwitchArr ); ++i ){
			Combo_InsertString( hwndCombo, i, ImeSwitchArr[i].pszName );
			if( ImeSwitchArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		// 入力モード
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		Combo_ResetContent( hwndCombo );
		ime = m_Types.m_nImeState >> 2;
		nSelPos = 0;
		for( int i = 0; i < _countof( ImeStateArr ); ++i ){
			Combo_InsertString( hwndCombo, i, ImeStateArr[i].pszName );
			if( ImeStateArr[i].nMethod == ime ){	/* IME状態 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );
	}

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// 「自動認識時にCESU-8を優先」m_Types.m_encoding.m_bPriorCesu8 をチェック
		::CheckDlgButton( hwndDlg, IDC_CHECK_PRIOR_CESU8, m_Types.m_encoding.m_bPriorCesu8 );

		// デフォルトコードタイプのコンボボックス設定
		int		nSel= 0;
		int		j = 0;
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		CCodeTypesForCombobox cCodeTypes;
		for (i = 0; i < cCodeTypes.GetCount(); i++) {
			if (CCodeTypeName( cCodeTypes.GetCode(i) ).CanDefault()) {
				int idx = Combo_AddString( hCombo, cCodeTypes.GetName(i) );
				Combo_SetItemData( hCombo, idx, cCodeTypes.GetCode(i) );
				if (m_Types.m_encoding.m_eDefaultCodetype == cCodeTypes.GetCode(i)) {
					nSel = j;
				}
				j++;
			}
		}
		Combo_SetCurSel( hCombo, nSel );

		// BOM チェックボックス設定
		CCodeTypeName	cCodeTypeName(m_Types.m_encoding.m_eDefaultCodetype);
		if( !cCodeTypeName.UseBom() )
			m_Types.m_encoding.m_bDefaultBom = false;
		::CheckDlgButton( hwndDlg, IDC_CHECK_DEFAULT_BOM, (m_Types.m_encoding.m_bDefaultBom ? BST_CHECKED : BST_UNCHECKED) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DEFAULT_BOM ), (int)cCodeTypeName.UseBom() );

		// デフォルト改行タイプのコンボボックス設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		for( i = 0; i < _countof(aszEolStr); ++i ){
			ApiWrap::Combo_AddString( hCombo, aszEolStr[i] );
		}
		for( i = 0; i < _countof(aeEolType); ++i ){
			if( m_Types.m_encoding.m_eDefaultEoltype == aeEolType[i] ){
				break;
			}
		}
		if( i == _countof(aeEolType) ){
			i = 0;
		}
		Combo_SetCurSel( hCombo, i );
	}

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( !m_Types.m_bLineNumIsCRLF ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, FALSE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINENUM_CRLF, TRUE );
	}

	// 背景画像
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_PATH), _countof2(m_Types.m_szBackImgPath));
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X), 5);
	EditCtl_LimitText(GetDlgItem(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y), 5);

	DlgItem_SetText( hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath );
	{
		const TCHAR* posNames[] ={
			_T("左上"),
			_T("右上"),
			_T("左下"),
			_T("右下"),
			_T("中央"),
			_T("中央上"),
			_T("中央下"),
			_T("中央左"),
			_T("中央右"),
		};
		/*BGIMAGE_TOP_LEFT .. */
		int nCount = _countof(posNames);
		SetCombobox( ::GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS), posNames, nCount, m_Types.m_backImgPos);
	}
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X, m_Types.m_backImgRepeatX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y, m_Types.m_backImgRepeatY);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X, m_Types.m_backImgScrollX);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y, m_Types.m_backImgScrollY);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, m_Types.m_backImgPosOffset.x, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, m_Types.m_backImgPosOffset.y, TRUE);

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( 0 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 1 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, TRUE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, FALSE );
	}else
	if( 2 == m_Types.m_nLineTermType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE0, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE1, FALSE );
		::CheckDlgButton( hwndDlg, IDC_RADIO_LINETERMTYPE2, TRUE );
	}

	/* 行番号区切り文字 */
	wchar_t	szLineTermChar[2];
	auto_sprintf( szLineTermChar, L"%lc", m_Types.m_cLineTermChar );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar );

	//	From Here Sept. 10, 2000 JEPRO
	//	行番号区切りを任意の半角文字にするときだけ指定文字入力をEnableに設定
	EnableTypesPropInput( hwndDlg );
	//	To Here Sept. 10, 2000


	return;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



/* ダイアログデータの取得 color */
int CPropTypesWindow::GetData( HWND hwndDlg )
{
	{
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		m_Types.m_bUseDocumentIcon = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DOCICON ) ? true : false;
	}

	//起動時のIME(日本語入力変換)	Nov. 20, 2000 genta
	{
		//入力モード
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESTATE );
		int		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState = ImeStateArr[nSelPos].nMethod << 2;	//	IME入力モード

		//ON/OFF状態
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_IMESWITCH );
		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nImeState |= ImeSwitchArr[nSelPos].nMethod;	//	IME ON/OFF
	}

	/* 「文字コード」グループの設定 */
	{
		int i;
		HWND hCombo;

		// m_Types.m_bPriorCesu8 を設定
		m_Types.m_encoding.m_bPriorCesu8 = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_PRIOR_CESU8 ) != 0;

		// m_Types.eDefaultCodetype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_CODETYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultCodetype = ECodeType( Combo_GetItemData( hCombo, i ) );
		}

		// m_Types.m_bDefaultBom を設定
		m_Types.m_encoding.m_bDefaultBom = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DEFAULT_BOM ) != 0;

		// m_Types.eDefaultEoltype を設定
		hCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_DEFAULT_EOLTYPE );
		i = Combo_GetCurSel( hCombo );
		if( CB_ERR != i ){
			m_Types.m_encoding.m_eDefaultEoltype = aeEolType[i];
		}
	}

	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_Types.m_bLineNumIsCRLF = false;
	}else{
		m_Types.m_bLineNumIsCRLF = true;
	}

	DlgItem_GetText(hwndDlg, IDC_EDIT_BACKIMG_PATH, m_Types.m_szBackImgPath, _countof2(m_Types.m_szBackImgPath));
	m_Types.m_backImgPos = static_cast<EBackgroundImagePos>(Combo_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO_BACKIMG_POS)));
	m_Types.m_backImgRepeatX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_X);
	m_Types.m_backImgRepeatY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_REP_Y);
	m_Types.m_backImgScrollX = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_X);
	m_Types.m_backImgScrollY = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_BACKIMG_SCR_Y);
	m_Types.m_backImgPosOffset.x = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_X, NULL, TRUE);
	m_Types.m_backImgPosOffset.y = GetDlgItemInt(hwndDlg, IDC_EDIT_BACKIMG_OFFSET_Y, NULL, TRUE);

	/* 行番号区切り  0=なし 1=縦線 2=任意 */
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE0 ) ){
		m_Types.m_nLineTermType = 0;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE1 ) ){
		m_Types.m_nLineTermType = 1;
	}else
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		m_Types.m_nLineTermType = 2;
	}
	
	/* 行番号区切り文字 */
	wchar_t	szLineTermChar[2];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_LINETERMCHAR, szLineTermChar, 2 );
	m_Types.m_cLineTermChar = szLineTermChar[0];

	return TRUE;
}




//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CPropTypesWindow::EnableTypesPropInput( HWND hwndDlg )
{
	//	行番号区切りを任意の半角文字にするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_LINETERMTYPE2 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_LINETERMCHAR ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_LINETERMCHAR ), FALSE );
	}
}
//	To Here Sept. 10, 2000

