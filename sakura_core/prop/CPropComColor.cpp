/*!
 *	@file
 *	@brief	共通設定ダイアログボックス、「テーマ」ページ (2026.11.21 「カラー」から名前を変更)
 *
 * カラーテーマ (全タイプ共通) の選択と、カスタムテーマの色・色以外の表示の設定 (TAB 表示、矢印表示、カーソル形状) の編集を行う。
 * プリセットのテーマを選んでいる間は配色を確認するだけで、変更はカスタムを選んでいる間だけできる。
 * カスタムの色は、プリセットの配色からのコピーと、色設定ファイル (.col) のインポートでも設定できる。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/18 新規作成
 */
/*!
 *	Copyright (C) 2026, Suzuki Satoshi
 *
 *	This software is provided 'as-is', without any express or implied
 *	warranty. In no event will the authors be held liable for any damages
 *	arising from the use of this software.
 *
 *	Permission is granted to anyone to use this software for any purpose,
 *	including commercial applications, and to alter it and redistribute it
 *	freely, subject to the following restrictions:
 *
 *		1. The origin of this software must not be misrepresented;
 *		   you must not claim that you wrote the original software.
 *		   If you use this software in a product, an acknowledgment
 *		   in the product documentation would be appreciated but is
 *		   not required.
 *
 *		2. Altered source versions must be plainly marked as such,
 *		   and must not be misrepresented as being the original software.
 *
 *		3. This notice may not be removed or altered from any source
 *		   distribution.
 */

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/DLLSHAREDATA.h"
#include "typeprop/CDlgSameColor.h"
#include "typeprop/CImpExpManager.h"
#include "view/colors/CColorStrategy.h"	// g_ColorAttributeArr
#include "util/window.h"
#include "util/MessageBoxF.h"
#include "sakura_rc.h"


/*!
 * CPropColor::DlgProc_page - 「テーマ」ページのダイアログプロシージャ
 *
 *	@return メッセージの処理結果
 */
INT_PTR CALLBACK CPropColor::DlgProc_page(
	HWND	hwndDlg,	//!< [in] ダイアログ
	UINT	uMsg,		//!< [in] メッセージ
	WPARAM	wParam,		//!< [in] パラメータ
	LPARAM	lParam		//!< [in] パラメータ
)
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropColor::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}

/*!
 * CPropColor::DispatchEvent - 「テーマ」ページのメッセージ処理
 *
 *	@return TRUE: 処理した / FALSE: 処理していない
 */
INT_PTR CPropColor::DispatchEvent(
	HWND	hwndDlg,	//!< [in] ダイアログ
	UINT	uMsg,		//!< [in] メッセージ
	WPARAM	wParam,		//!< [in] パラメータ
	LPARAM	lParam		//!< [in] パラメータ
)
{
	HWND hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );

	switch( uMsg ){
	case WM_INITDIALOG:
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		// 色選択ダイアログのカスタムパレット (タイプ別設定と同じく、共有データの値を使う)
		memcpy_raw( m_dwColorCustColors, m_pShareData->m_dwCustColors, sizeof(m_dwColorCustColors) );

		{
			HWND hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_THEME );
			for( int i = 0; i < COLOR_THEME_SEL_COUNT; i++ ){
				Combo_AddString( hwndCombo, GetColorThemeDispName( GetColorThemeBySelIndex( i ) ) );
			}
		}
		EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_THEME_TABVIEWSTRING ), _countof(m_Common.m_sColorTheme.m_sCustomView.m_szTabViewString) - 1 );	// 2026.11.21

		SetData( hwndDlg );
		m_cColorListUi.Attach( hwndList, m_dwColorCustColors, CanEditColor, this );
		return TRUE;

	case WM_COMMAND:
		{
			const WORD wNotifyCode = HIWORD( wParam );
			const WORD wID = LOWORD( wParam );
			HWND hwndCtl = (HWND)lParam;

			if( hwndCtl == hwndList && wNotifyCode == LBN_SELCHANGE ){
				const int nIndex = List_GetCurSel( hwndList );
				if( 0 <= nIndex && nIndex < COLORIDX_LAST ){
					m_nCurrentColorIdx = nIndex;
				}
				UpdateControls( hwndDlg );
				return TRUE;
			}
			if( wID == IDC_COMBO_THEME && wNotifyCode == CBN_SELCHANGE ){
				const int nSel = Combo_GetCurSel( hwndCtl );
				m_Common.m_sColorTheme.m_nColorTheme = GetColorThemeBySelIndex( nSel );
				SetColorList( hwndDlg );
				SetViewControls( hwndDlg );	// 2026.11.21
				return TRUE;
			}
			if( wID == IDC_EDIT_THEME_TABVIEWSTRING && wNotifyCode == EN_CHANGE ){	// 2026.11.21 TAB 表示
				GetTabViewString( hwndDlg );
				return TRUE;
			}
			if( wNotifyCode != BN_CLICKED ){
				break;
			}

			// 色以外の表示の設定 (カスタム選択中だけ有効なコントロール)	// 2026.11.21
			ColorThemeView& sCustomView = m_Common.m_sColorTheme.m_sCustomView;
			switch( wID ){
			case IDC_CHECK_THEME_TAB_ARROW:	/* 矢印表示 */
				if( IsCustomTheme() ){
					sCustomView.m_bTabArrow = ( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_THEME_TAB_ARROW ) != BST_UNCHECKED );
					SetViewControls( hwndDlg );
				}
				return TRUE;
			case IDC_RADIO_THEME_CARETTYPE0:	/* カーソル形状 */
			case IDC_RADIO_THEME_CARETTYPE1:
				if( IsCustomTheme() ){
					sCustomView.m_nCaretType = ( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_THEME_CARETTYPE1 ) != BST_UNCHECKED ) ? 1 : 0;
				}
				return TRUE;
			}

			ColorInfo* pColorInfoArr = GetDispColorInfoArr();
			ColorInfo& cColorInfo = pColorInfoArr[m_nCurrentColorIdx];
			switch( wID ){
			case IDC_CHECK_DISP:	/* 色分け/表示 をする */
				cColorInfo.m_bDisp = ( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP ) != BST_UNCHECKED );
				::InvalidateRect( hwndList, NULL, TRUE );
				return TRUE;
			case IDC_CHECK_BOLD:	/* 太字か */
				cColorInfo.m_bBoldFont = ( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_BOLD ) != BST_UNCHECKED );
				::InvalidateRect( hwndList, NULL, TRUE );
				return TRUE;
			case IDC_CHECK_UNDERLINE:	/* 下線を表示 */
				cColorInfo.m_bUnderLine = ( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_UNDERLINE ) != BST_UNCHECKED );
				::InvalidateRect( hwndList, NULL, TRUE );
				return TRUE;
			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
				if( CColorListUi::SelectColor( hwndDlg, &cColorInfo.m_colTEXT, m_dwColorCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
					::InvalidateRect( hwndList, NULL, TRUE );
				}
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
				if( CColorListUi::SelectColor( hwndDlg, &cColorInfo.m_colBACK, m_dwColorCustColors ) ){
					::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
					::InvalidateRect( hwndList, NULL, TRUE );
				}
				return TRUE;
			case IDC_BUTTON_SAMETEXTCOLOR:	/* 文字色統一 */
			case IDC_BUTTON_SAMEBKCOLOR:	/* 背景色統一 */
				{
					CDlgSameColor cDlgSameColor;
					const COLORREF cr = ( wID == IDC_BUTTON_SAMETEXTCOLOR ) ? cColorInfo.m_colTEXT : cColorInfo.m_colBACK;
					cDlgSameColor.DoModal( ::GetModuleHandle(NULL), hwndDlg, wID, pColorInfoArr, cr );
				}
				::InvalidateRect( hwndList, NULL, TRUE );
				return TRUE;
			case IDC_BUTTON_COPY_PRESET:	/* プリセットからコピー */
				CopyFromPreset( hwndDlg );
				return TRUE;
			case IDC_BUTTON_IMPORT:	/* 色の設定をインポート */
				Import( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* 色の設定をエクスポート */
				Export( hwndDlg );
				return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			switch( pNMHDR->code ){
			case PSN_KILLACTIVE:
				GetData( hwndDlg );
				return TRUE;
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_COLOR;
				return TRUE;
			}
		}
		break;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT* pDis = (DRAWITEMSTRUCT*)lParam;
			const ColorInfo& cColorInfo = GetDispColorInfoArr()[m_nCurrentColorIdx];
			switch( (UINT)wParam ){
			case IDC_BUTTON_TEXTCOLOR:	/* テキスト色 */
				CColorListUi::DrawColorButton( pDis, cColorInfo.m_colTEXT );
				return TRUE;
			case IDC_BUTTON_BACKCOLOR:	/* 背景色 */
				CColorListUi::DrawColorButton( pDis, cColorInfo.m_colBACK );
				return TRUE;
			case IDC_LIST_COLORS:		/* 色種別リスト */
				m_cColorListUi.DrawItem( pDis );
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

/*!
 * CPropColor::SetData - ダイアログデータの設定
 */
void CPropColor::SetData(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	Combo_SetCurSel( ::GetDlgItem( hwndDlg, IDC_COMBO_THEME ), GetColorThemeSelIndex( m_Common.m_sColorTheme.m_nColorTheme ) );
	SetColorList( hwndDlg );
	SetViewControls( hwndDlg );	// 2026.11.21

	// 画面の配色	// 2026.11.20
	int nRadioId;
	switch( m_Common.m_sColorTheme.m_nUiColorMode ){
	case UI_COLOR_MODE_DARK:	nRadioId = IDC_RADIO_UICOLOR_DARK;		break;
	case UI_COLOR_MODE_SYSTEM:	nRadioId = IDC_RADIO_UICOLOR_SYSTEM;	break;
	default:					nRadioId = IDC_RADIO_UICOLOR_LIGHT;		break;
	}
	::CheckRadioButton( hwndDlg, IDC_RADIO_UICOLOR_LIGHT, IDC_RADIO_UICOLOR_SYSTEM, nRadioId );
}

/*!
 * CPropColor::GetData - ダイアログデータの取得
 *
 * テーマとカスタムの色は、操作のたびに m_Common に設定しているため、ここでは画面の配色だけを取得する。
 *
 *	@return TRUE
 */
int CPropColor::GetData(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	// 画面の配色	// 2026.11.20
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_UICOLOR_DARK ) ){
		m_Common.m_sColorTheme.m_nUiColorMode = UI_COLOR_MODE_DARK;
	}else if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_UICOLOR_SYSTEM ) ){
		m_Common.m_sColorTheme.m_nUiColorMode = UI_COLOR_MODE_SYSTEM;
	}else{
		m_Common.m_sColorTheme.m_nUiColorMode = UI_COLOR_MODE_LIGHT;
	}
	return TRUE;
}

/*!
 * CPropColor::IsCustomTheme - カスタムテーマを選んでいるかを返す
 *
 *	@return true: カスタム / false: プリセット
 */
bool CPropColor::IsCustomTheme() const
{
	return m_Common.m_sColorTheme.m_nColorTheme == COLOR_THEME_CUSTOM;
}

/*!
 * CPropColor::GetDispColorInfoArr - 一覧に表示する色設定を返す
 *
 *	@return カスタム選択中はカスタムの色 (編集できる)、プリセット選択中はプリセットの配色
 */
ColorInfo* CPropColor::GetDispColorInfoArr()
{
	if( IsCustomTheme() ){
		return m_Common.m_sColorTheme.m_CustomColorInfoArr;
	}
	return m_PresetColorInfoArr;
}

/*!
 * CPropColor::SetColorList - 選択中のテーマの色で、色設定の一覧を作り直す
 */
void CPropColor::SetColorList(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	// プリセットの配色 (カスタム選択中は、項目名とインデックスの設定にだけ使う)
	SetPresetColorInfo( m_PresetColorInfoArr, m_Common.m_sColorTheme.m_nColorTheme );
	if( IsCustomTheme() ){
		ColorInfo* pCustom = m_Common.m_sColorTheme.m_CustomColorInfoArr;
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			pCustom[i].m_nColorIdx = i;
			wcscpy_s( pCustom[i].m_szName, _countof(pCustom[i].m_szName), m_PresetColorInfoArr[i].m_szName );
		}
	}

	ColorInfo* pColorInfoArr = GetDispColorInfoArr();
	HWND hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_COLORS );
	List_ResetContent( hwndList );
	for( int i = 0; i < COLORIDX_LAST; i++ ){
		const int nItem = List_AddString( hwndList, pColorInfoArr[i].m_szName );
		List_SetItemData( hwndList, nItem, &pColorInfoArr[i] );
	}
	List_SetCurSel( hwndList, m_nCurrentColorIdx );
	UpdateControls( hwndDlg );
}

/*!
 * CPropColor::UpdateControls - 選択中のテーマと色の項目に合わせて、コントロールの状態と値を設定する
 *
 * プリセット選択中は、色を変更するコントロールをすべて無効にする (エクスポートを除く)。
 * 「色分け/表示」は、全項目のテーマの値 (テーマを切り替えたときにタイプ別設定に適用する値) を表示・変更する。
 */
void CPropColor::UpdateControls(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	const bool bCustom = IsCustomTheme();
	const int nIdx = m_nCurrentColorIdx;
	const unsigned int fAttribute = g_ColorAttributeArr[nIdx].fAttribute;
	const ColorInfo& cColorInfo = GetDispColorInfoArr()[nIdx];

	const bool bDisp	= bCustom && 0 == (fAttribute & COLOR_ATTRIB_FORCE_DISP);	// 2026.11.21 全項目
	const bool bBold	= bCustom && 0 == (fAttribute & COLOR_ATTRIB_NO_BOLD);
	const bool bUnder	= bCustom && 0 == (fAttribute & COLOR_ATTRIB_NO_UNDERLINE);
	const bool bText	= bCustom && 0 == (fAttribute & COLOR_ATTRIB_NO_TEXT);
	const bool bBack	= bCustom && 0 == (fAttribute & COLOR_ATTRIB_NO_BACK);

	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_DISP ),			bDisp ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_BOLD ),			bBold ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_UNDERLINE ),		bUnder ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_MOZI ),			bText ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ),		bText ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMETEXTCOLOR ),	bText ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_STATIC_HAIKEI ),			bBack ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ),		bBack ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_SAMEBKCOLOR ),	bBack ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_COPY_PRESET ),	bCustom ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ),			bCustom ? TRUE : FALSE );

	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP,		cColorInfo.m_bDisp ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hwndDlg, IDC_CHECK_BOLD,		cColorInfo.m_bBoldFont ? BST_CHECKED : BST_UNCHECKED );
	::CheckDlgButton( hwndDlg, IDC_CHECK_UNDERLINE,	cColorInfo.m_bUnderLine ? BST_CHECKED : BST_UNCHECKED );

	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_TEXTCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_BACKCOLOR ), NULL, TRUE );
	::InvalidateRect( ::GetDlgItem( hwndDlg, IDC_LIST_COLORS ), NULL, TRUE );
}

/*!
 * CPropColor::CopyFromPreset - プリセットの配色を選んで、カスタムの色にコピーする
 *
 * プリセットの一覧をポップアップメニューで表示し、選んだテーマの配色でカスタムの色を上書きする (確認あり)。
 * 色以外の表示の設定 (TAB 表示、矢印表示、カーソル形状) も、選んだテーマの初期値で上書きする。	// 2026.11.21
 */
void CPropColor::CopyFromPreset(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	if( !IsCustomTheme() ){
		return;
	}
	HMENU hMenu = ::CreatePopupMenu();
	for( int i = 0; i < COLOR_THEME_SEL_COUNT; i++ ){
		const int nColorTheme = GetColorThemeBySelIndex( i );
		if( IsColorThemePreset( nColorTheme ) ){
			::AppendMenu( hMenu, MF_STRING, nColorTheme + 1, GetColorThemeDispName( nColorTheme ) );	// 0 は「選ばなかった」になるため +1
		}
	}
	RECT rc;
	::GetWindowRect( ::GetDlgItem( hwndDlg, IDC_BUTTON_COPY_PRESET ), &rc );
	const int nCmd = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON, rc.left, rc.bottom, 0, hwndDlg, NULL );
	::DestroyMenu( hMenu );
	if( nCmd <= 0 ){
		return;
	}
	const int nColorTheme = nCmd - 1;
	if( IDYES != ConfirmMessage( hwndDlg, L"カスタムの色を「%ts」の配色で上書きします。よろしいですか？", GetColorThemeDispName( nColorTheme ) ) ){
		return;
	}
	SetPresetColorInfo( m_Common.m_sColorTheme.m_CustomColorInfoArr, nColorTheme );
	SetPresetColorThemeView( &m_Common.m_sColorTheme.m_sCustomView, nColorTheme );	// 2026.11.21 TAB 表示・矢印表示・カーソル形状もコピーする
	SetColorList( hwndDlg );
	SetViewControls( hwndDlg );
}

/*!
 * CPropColor::SetViewControls - 選択中のテーマの、色以外の表示の設定 (TAB 表示、矢印表示、カーソル形状) を表示する
 *
 * プリセット選択中はテーマの初期値を表示してコントロールを無効にし、カスタム選択中はカスタムの値を表示して変更できるようにする。
 * 矢印表示がオンのときは、TAB 表示のエディットを無効にする (タイプ別設定の「スクリーン」タブと同じ)。
 */
void CPropColor::SetViewControls(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	const bool bCustom = IsCustomTheme();
	ColorThemeView sView;
	if( bCustom ){
		sView = m_Common.m_sColorTheme.m_sCustomView;
	}else{
		SetPresetColorThemeView( &sView, m_Common.m_sColorTheme.m_nColorTheme );
	}

	// エディットの EN_CHANGE で値を読み直す (GetTabViewString) ため、表示中の文字列と異なるときだけ設定する
	WCHAR szTab[_countof(sView.m_szTabViewString)];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_THEME_TABVIEWSTRING, szTab, _countof(szTab) );
	if( wcscmp( szTab, sView.m_szTabViewString ) != 0 ){
		::DlgItem_SetText( hwndDlg, IDC_EDIT_THEME_TABVIEWSTRING, sView.m_szTabViewString );
	}
	::CheckDlgButton( hwndDlg, IDC_CHECK_THEME_TAB_ARROW, sView.m_bTabArrow ? BST_CHECKED : BST_UNCHECKED );
	::CheckRadioButton( hwndDlg, IDC_RADIO_THEME_CARETTYPE0, IDC_RADIO_THEME_CARETTYPE1,
		( sView.m_nCaretType == 1 ) ? IDC_RADIO_THEME_CARETTYPE1 : IDC_RADIO_THEME_CARETTYPE0 );

	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_THEME_TABVIEWSTRING ), ( bCustom && !sView.m_bTabArrow ) ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_THEME_TAB_ARROW ), bCustom ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_THEME_CARETTYPE0 ), bCustom ? TRUE : FALSE );
	::EnableWindow( ::GetDlgItem( hwndDlg, IDC_RADIO_THEME_CARETTYPE1 ), bCustom ? TRUE : FALSE );
}

/*!
 * CPropColor::GetTabViewString - TAB 表示のエディットの文字列を、カスタムの値に設定する
 *
 * カスタム選択中だけ設定する。タイプ別設定の「スクリーン」タブ (CPropTypesScreen::GetData) と同じく、
 * 既定の文字列 "o......." の先頭から、TAB 表示に使える文字だけを 8 文字まで置き換える。
 */
void CPropColor::GetTabViewString(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	if( !IsCustomTheme() ){
		return;
	}
	ColorThemeView& sCustomView = m_Common.m_sColorTheme.m_sCustomView;
	WCHAR szTab[8+1];
	::DlgItem_GetText( hwndDlg, IDC_EDIT_THEME_TABVIEWSTRING, szTab, _countof(szTab) );
	wcscpy_s( sCustomView.m_szTabViewString, _countof(sCustomView.m_szTabViewString), L"o......." );
	for( int i = 0; i < 8; i++ ){
		if( !WCODE::IsTabAvailableCode( szTab[i] ) ){
			break;
		}
		sCustomView.m_szTabViewString[i] = szTab[i];
	}
}

/*!
 * CPropColor::Import - 色設定ファイル (.col) をカスタムの色に読み込む
 */
void CPropColor::Import(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	if( !IsCustomTheme() ){
		return;
	}
	CImpExpColors cImpExpColors( m_Common.m_sColorTheme.m_CustomColorInfoArr );
	if( cImpExpColors.ImportUI( G_AppInstance(), hwndDlg ) ){
		SetColorList( hwndDlg );
	}
}

/*!
 * CPropColor::Export - 表示中のテーマの色を、色設定ファイル (.col) に書き出す
 */
void CPropColor::Export(
	HWND	hwndDlg		//!< [in] ダイアログ
)
{
	CImpExpColors cImpExpColors( GetDispColorInfoArr() );
	cImpExpColors.ExportUI( G_AppInstance(), hwndDlg );
}

/*!
 * CPropColor::CanEditColor - 色設定の一覧で、値を変更してよいかを返す
 *
 * カスタム選択中だけ変更できる。「色分け/表示」は、全項目を表示する (2026.11.21 変更)。
 *
 *	@return	true: 変更してよい / false: 変更できない
 */
bool CPropColor::CanEditColor(
	void*			pParam,		//!< [in] CPropColor
	int				nColorIdx,	//!< [in] 色の項目 (EColorIndexType)
	EColorListEdit	eEdit		//!< [in] 変更する値
)
{
	const CPropColor* pThis = (const CPropColor*)pParam;
	switch( eEdit ){
	case COLORLIST_SHOW_DISP:
		return true;
	default:
		return pThis->IsCustomTheme();
	}
}
