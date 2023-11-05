/*! @file
	@brief タイプ別設定 - スクリーン

	@date 2008.04.12 kobake CPropTypes.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, MIK, hor, Stonee, asa-o
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta, こおり, Moca
	Copyright (C) 2003, MIK, zenryaku, Moca, naoh, KEITA, genta
	Copyright (C) 2005, MIK, genta, Moca, ryoji
	Copyright (C) 2006, ryoji, fon, novice
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CPropTypes.h"
#include "dlg/CDlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/file.h" // _IS_REL_PATH
#include "sakura_rc.h"


//アウトライン解析方法・標準ルール
TYPE_NAME<EOutlineType> OlmArr[] = {
//	{ OUTLINE_C,		_T("C") },
	{ OUTLINE_CPP,		_T("C/C++") },
	{ OUTLINE_PLSQL,	_T("PL/SQL") },
	{ OUTLINE_JAVA,		_T("Java") },
	{ OUTLINE_COBOL,	_T("COBOL") },
	{ OUTLINE_PERL,		_T("Perl") },				//Sep. 8, 2000 genta
	{ OUTLINE_ASM,		_T("アセンブラ") },
	{ OUTLINE_VB,		_T("Visual Basic") },		// 2001/06/23 N.Nakatani
	{ OUTLINE_PYTHON,	_T("Python") },				//	2007.02.08 genta
	{ OUTLINE_ERLANG,	_T("Erlang") },				//	2009.08.10 genta
	{ OUTLINE_WZTXT,	_T("WZ階層付テキスト") },	// 2003.05.20 zenryaku, 2003.06.23 Moca 名称変更
	{ OUTLINE_HTML,		_T("HTML") },				// 2003.05.20 zenryaku
	{ OUTLINE_TEX,		_T("TeX") },				// 2003.07.20 naoh
	{ OUTLINE_TEXT,		_T("テキスト") }			//Jul. 08, 2001 JEPRO 常に最後尾におく
};

TYPE_NAME<ESmartIndentType> SmartIndentArr[] = {
	{ SMARTINDENT_NONE,	_T("なし") },
	{ SMARTINDENT_CPP,	_T("C/C++") }
};

/*!	2行目以降のインデント方法

	@sa CLayoutMgr::SetLayoutInfo()
	@date Oct. 1, 2002 genta 
*/
TYPE_NAME<int> IndentTypeArr[] = {
	{ 0, _T("なし") },
	{ 1, _T("tx2x") },
	{ 2, _T("論理行先頭") },
};

// 2008.05.30 nasukoji	テキストの折り返し方法
TYPE_NAME<int> WrapMethodArr[] = {
	{ WRAP_NO_TEXT_WRAP,	_T("折り返さない") },
	{ WRAP_SETTING_WIDTH,	_T("指定桁で折り返す") },
	{ WRAP_WINDOW_WIDTH,	_T("右端で折り返す") },
};

//静的メンバ
std::vector<TYPE_NAME<EOutlineType> > CPropTypes::m_OlmArr;	//!<アウトライン解析ルール配列
std::vector<TYPE_NAME<ESmartIndentType> > CPropTypes::m_SIndentArr;	//!<スマートインデントルール配列

//スクリーンタブの初期化
void CPropTypesScreen::CPropTypes_Screen()
{
	//プラグイン無効の場合、ここで静的メンバを初期化する。プラグイン有効の場合はAddXXXMethod内で初期化する。
	if( m_OlmArr.empty() ){
		m_OlmArr.insert(m_OlmArr.end(), OlmArr, &OlmArr[_countof(OlmArr)]);	//アウトライン解析ルール
	}
	if( m_SIndentArr.empty() ){
		m_SIndentArr.insert(m_SIndentArr.end(), SmartIndentArr, &SmartIndentArr[_countof(SmartIndentArr)]);	//スマートインデントルール
	}
}

/* Screen メッセージ処理 */
INT_PTR CPropTypesScreen::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	switch( uMsg ){

	case WM_INITDIALOG:
		m_hwndThis = hwndDlg;
		/* ダイアログデータの設定 Screen */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		// エディットコントロールの入力文字数制限
		EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME        ), _countof( m_Types.m_szTypeName      ) - 1 );
		EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS        ), _countof( m_Types.m_szTypeExts      ) - 1 );
		EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_INDENTCHARS     ), _countof( m_Types.m_szIndentChars   ) - 1 );
		EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING   ), _countof( m_Types.m_szTabViewString ) - 1 );
		EditCtl_LimitText( GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), _countof2( m_Types.m_szOutlineRuleFilename ) - 1 );	//	Oct. 5, 2002 genta 画面上でも入力制限

		if( 0 == m_Types.m_nIdx ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPENAME ), FALSE );	//設定の名前
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TYPEEXTS ), FALSE );	//ファイル拡張子
		}

		return TRUE;
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	/* 通知コード */
		wID			= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
//		hwndCtl		= (HWND) lParam;	/* コントロールのハンドル */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			/*	2002.04.01 YAZAKI オートインデントを削除（もともと不要）
				アウトライン解析にルールファイル関連を追加
			*/
			case IDC_RADIO_OUTLINEDEFAULT:	/* アウトライン解析→標準ルール */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

				Combo_SetCurSel( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), 0 );

				return TRUE;
			case IDC_RADIO_OUTLINERULEFILE:	/* アウトライン解析→ルールファイル */
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );
				return TRUE;

			case IDC_BUTTON_RULEFILE_REF:	/* アウトライン解析→ルールファイルの「参照...」ボタン */
				{
					CDlgOpenFile	cDlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
					// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
					if( _IS_REL_PATH( m_Types.m_szOutlineRuleFilename ) ){
						GetInidirOrExedir( szPath, m_Types.m_szOutlineRuleFilename );
					}else{
						_tcscpy( szPath, m_Types.m_szOutlineRuleFilename );
					}
					/* ファイルオープンダイアログの初期化 */
					cDlgOpenFile.Create(
						m_hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if( cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
						_tcscpy( m_Types.m_szOutlineRuleFilename, szPath );
						::DlgItem_SetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );
					}
				}
				return TRUE;

			case IDC_CHECK_TAB_ARROW:
				// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW ) ){
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), FALSE );
				}
				else {
					::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), TRUE );
				}
				return TRUE;

			case IDC_CHECK_KINSOKURET:		//改行文字をぶら下げる
			case IDC_CHECK_KINSOKUKUTO:		//句読点をぶら下げる
				// ぶら下げを隠すの有効化	2012/11/30 Uchi
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KINSOKUHIDE ), 
					::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKURET ) 
				 || ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUKUTO ) );
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( idCtrl ){
		case IDC_SPIN_MAXLINELEN:
			/* 折り返し桁数 */
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < MINLINEKETAS ){
				nVal = MINLINEKETAS;
			}
			if( nVal > MAXLINEKETAS ){
				nVal = MAXLINEKETAS;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_CHARSPACE:
			/* 文字の隙間 */
//			MYTRACE( _T("IDC_SPIN_CHARSPACE\n") );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 0 ){
				nVal = 0;
			}
			if( nVal > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				nVal = COLUMNSPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_LINESPACE:
			/* 行の隙間 */
//			MYTRACE( _T("IDC_SPIN_LINESPACE\n") );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
//	From Here Oct. 8, 2000 JEPRO 行間も最小0まで設定できるように変更(昔に戻っただけ?)
//			if( nVal < 1 ){
//				nVal = 1;
//			}
			if( nVal < 0 ){
				nVal = 0;
			}
//	To Here  Oct. 8, 2000
			if( nVal > LINESPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
				nVal = LINESPACE_MAX;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, nVal, FALSE );
			return TRUE;
		case IDC_SPIN_TABSPACE:
			//	Sep. 22, 2002 genta
			/* TAB幅 */
//			MYTRACE( _T("IDC_SPIN_CHARSPACE\n") );
			nVal = ::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE );
			if( pMNUD->iDelta < 0 ){
				++nVal;
			}else
			if( pMNUD->iDelta > 0 ){
				--nVal;
			}
			if( nVal < 1 ){
				nVal = 1;
			}
			if( nVal > 64 ){
				nVal = 64;
			}
			::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, nVal, FALSE );
			return TRUE;

		default:
			switch( pNMHDR->code ){
			case PSN_KILLACTIVE:
				/* ダイアログデータの取得 Screen */
				GetData( hwndDlg );

				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = 0;
				return TRUE;
			}
			break;
		}

//		MYTRACE( _T("pNMHDR->hwndFrom	=%xh\n"),	pNMHDR->hwndFrom );
//		MYTRACE( _T("pNMHDR->idFrom	=%xh\n"),	pNMHDR->idFrom );
//		MYTRACE( _T("pNMHDR->code		=%xh\n"),	pNMHDR->code );
//		MYTRACE( _T("pMNUD->iPos		=%d\n"),		pMNUD->iPos );
//		MYTRACE( _T("pMNUD->iDelta		=%d\n"),		pMNUD->iDelta );
		break;

	}
	return FALSE;
}



/* ダイアログデータの設定 Screen */
void CPropTypesScreen::SetData( HWND hwndDlg )
{
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName );	//設定の名前
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts );	//ファイル拡張子

	//レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WRAPMETHOD );
		Combo_ResetContent( hwndCombo );
		int		nSelPos = 0;
		for( int i = 0; i < _countof( WrapMethodArr ); ++i ){
			Combo_InsertString( hwndCombo, i, WrapMethodArr[i].pszName );
			if( WrapMethodArr[i].nMethod == m_Types.m_nTextWrapMethod ){		// テキストの折り返し方法
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		::SetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, (Int)m_Types.m_nMaxLineKetas, FALSE );	// 折り返し文字数
		::SetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, m_Types.m_nColumnSpace, FALSE );			// 文字の間隔
		::SetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, m_Types.m_nLineSpace, FALSE );			// 行の間隔
		::SetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, (Int)m_Types.m_nTabSpace, FALSE );			// TAB幅	//	Sep. 22, 2002 genta
		::DlgItem_SetText( hwndDlg, IDC_EDIT_TABVIEWSTRING, m_Types.m_szTabViewString );		// TAB表示(8文字)
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TABVIEWSTRING ), !m_Types.m_bTabArrow );	// Mar. 31, 2003 genta 矢印表示のON/OFFをTAB文字列設定に連動させる
		::CheckDlgButton( hwndDlg, IDC_CHECK_TAB_ARROW, m_Types.m_bTabArrow );					// 矢印表示 [チェックボックス]	//@@@ 2003.03.26 MIK
		::CheckDlgButton( hwndDlg, IDC_CHECK_INS_SPACE, m_Types.m_bInsSpace );					// SPACEの挿入 [チェックボックス]	// From Here 2001.12.03 hor
	}

	//インデント
	{
		/* 自動インデント */
		::CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENT, m_Types.m_bAutoIndent );

		/* 日本語空白もインデント */
		::CheckDlgButtonBool( hwndDlg, IDC_CHECK_INDENT_WSPACE, m_Types.m_bAutoIndent_ZENSPACE );

		/* スマートインデント種別 */
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
		Combo_ResetContent( hwndCombo );
		int		nSelPos = 0;
		int nSize = (int)m_SIndentArr.size();
		for( int i = 0; i < nSize; ++i ){
			Combo_InsertString( hwndCombo, i, m_SIndentArr[i].pszName );
			if( m_SIndentArr[i].nMethod == m_Types.m_eSmartIndent ){	/* スマートインデント種別 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		// その他のインデント対象文字
		::DlgItem_SetText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars );

		//折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
		Combo_ResetContent( hwndCombo );
		nSelPos = 0;
		for( int i = 0; i < _countof( IndentTypeArr ); ++i ){
			Combo_InsertString( hwndCombo, i, IndentTypeArr[i].pszName );
			if( IndentTypeArr[i].nMethod == m_Types.m_nIndentLayout ){	/* 折り返しインデント種別 */
				nSelPos = i;
			}
		}
		Combo_SetCurSel( hwndCombo, nSelPos );

		// 改行時に末尾の空白を削除	//2005.10.11 ryoji
		::CheckDlgButton( hwndDlg, IDC_CHECK_RTRIM_PREVLINE, m_Types.m_bRTrimPrevLine );
	}

	//アウトライン解析方法
	//2002.04.01 YAZAKI ルールファイル関連追加
	{
		//標準ルールのコンボボックス初期化
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
		Combo_ResetContent( hwndCombo );
		int		nSelPos = 0;
		int nSize = (int)m_OlmArr.size();
		for( int i = 0; i < nSize; ++i ){
			Combo_InsertString( hwndCombo, i, m_OlmArr[i].pszName );
			if( m_OlmArr[i].nMethod == m_Types.m_eDefaultOutline ){	/* アウトライン解析方法 */
				nSelPos = i;
			}
		}

		//ルールファイル	// 2003.06.23 Moca ルールファイル名は使わなくてもセットしておく
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), TRUE );
		::DlgItem_SetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename );

		//標準ルール
		if( m_Types.m_eDefaultOutline != OUTLINE_FILE ){
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, TRUE );
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, FALSE );

			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_OUTLINERULEFILE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), FALSE );

			Combo_SetCurSel( hwndCombo, nSelPos );
		}
		//ルールファイル
		else{
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINEDEFAULT, FALSE );
			::CheckDlgButton( hwndDlg, IDC_RADIO_OUTLINERULEFILE, TRUE );

			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RULEFILE_REF ), TRUE );
		}
	}

	//その他
	{
		/* 英文ワードラップをする */
		::CheckDlgButton( hwndDlg, IDC_CHECK_WORDWRAP, m_Types.m_bWordWrap ? TRUE : FALSE );
		
		/* 禁則処理 */
		{	//@@@ 2002.04.08 MIK start
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUHEAD, m_Types.m_bKinsokuHead ? TRUE : FALSE );
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUTAIL, m_Types.m_bKinsokuTail ? TRUE : FALSE );
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKURET,  m_Types.m_bKinsokuRet  ? TRUE : FALSE );	/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUKUTO, m_Types.m_bKinsokuKuto ? TRUE : FALSE );	/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
			::CheckDlgButton( hwndDlg, IDC_CHECK_KINSOKUHIDE, m_Types.m_bKinsokuHide ? TRUE : FALSE );	// ぶら下げを隠す			// 2011/11/30 Uchi
			EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUHEAD ), _countof(m_Types.m_szKinsokuHead) - 1 );
			EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUTAIL ), _countof(m_Types.m_szKinsokuTail) - 1 );
			EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_KINSOKUKUTO ), _countof(m_Types.m_szKinsokuKuto) - 1 );	// 2009.08.07 ryoji
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead );
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail );
			::DlgItem_SetText( hwndDlg, IDC_EDIT_KINSOKUKUTO, m_Types.m_szKinsokuKuto );	// 2009.08.07 ryoji
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KINSOKUHIDE ), ( m_Types.m_bKinsokuRet || m_Types.m_bKinsokuKuto ) ? TRUE : FALSE );	// ぶら下げを隠すの有効化	2012/11/30 Uchi
		}	//@@@ 2002.04.08 MIK end
		
		// 文書アイコンを使う	//Sep. 10, 2002 genta
		::CheckDlgButton( hwndDlg, IDC_CHECK_DOCICON, m_Types.m_bUseDocumentIcon  ? TRUE : FALSE );
	}
}



/* ダイアログデータの取得 Screen */
int CPropTypesScreen::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = 0;

	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPENAME, m_Types.m_szTypeName, _countof( m_Types.m_szTypeName ) );	// 設定の名前
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TYPEEXTS, m_Types.m_szTypeExts, _countof( m_Types.m_szTypeExts ) );	// ファイル拡張子

	//レイアウト
	{
		// 2008.05.30 nasukoji	テキストの折り返し方法
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_WRAPMETHOD );
		int		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nTextWrapMethod = WrapMethodArr[nSelPos].nMethod;		// テキストの折り返し方法

		/* 折り返し桁数 */
		m_Types.m_nMaxLineKetas = CLayoutInt(::GetDlgItemInt( hwndDlg, IDC_EDIT_MAXLINELEN, NULL, FALSE ));
		if( m_Types.m_nMaxLineKetas < CLayoutInt(MINLINEKETAS) ){
			m_Types.m_nMaxLineKetas = CLayoutInt(MINLINEKETAS);
		}
		if( m_Types.m_nMaxLineKetas > CLayoutInt(MAXLINEKETAS) ){
			m_Types.m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);
		}

		/* 文字の間隔 */
		m_Types.m_nColumnSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_CHARSPACE, NULL, FALSE );
		if( m_Types.m_nColumnSpace < 0 ){
			m_Types.m_nColumnSpace = 0;
		}
		if( m_Types.m_nColumnSpace > COLUMNSPACE_MAX ){ // Feb. 18, 2003 genta 最大値の定数化
			m_Types.m_nColumnSpace = COLUMNSPACE_MAX;
		}

		/* 行の間隔 */
		m_Types.m_nLineSpace = ::GetDlgItemInt( hwndDlg, IDC_EDIT_LINESPACE, NULL, FALSE );
		if( m_Types.m_nLineSpace < 0 ){
			m_Types.m_nLineSpace = 0;
		}
		if( m_Types.m_nLineSpace > LINESPACE_MAX ){	// Feb. 18, 2003 genta 最大値の定数化
			m_Types.m_nLineSpace = LINESPACE_MAX;
		}

		/* TAB幅 */
		m_Types.m_nTabSpace = CLayoutInt(::GetDlgItemInt( hwndDlg, IDC_EDIT_TABSPACE, NULL, FALSE ));
		if( m_Types.m_nTabSpace < CLayoutInt(1) ){
			m_Types.m_nTabSpace = CLayoutInt(1);
		}
		if( m_Types.m_nTabSpace > CLayoutInt(64) ){
			m_Types.m_nTabSpace = CLayoutInt(64);
		}

		/* TAB表示文字列 */
		WIN_CHAR szTab[8+1]; /* +1. happy */
		::DlgItem_GetText( hwndDlg, IDC_EDIT_TABVIEWSTRING, szTab, _countof( szTab ) );
		wcscpy( m_Types.m_szTabViewString, L"o......." );
		for( int i = 0; i < 8; i++ ){
			if( !TCODE::IsTabAvailableCode(szTab[i]) )break;
			m_Types.m_szTabViewString[i] = szTab[i];
		}

		// タブ矢印表示	//@@@ 2003.03.26 MIK
		m_Types.m_bTabArrow = (0 != ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TAB_ARROW ));

		// SPACEの挿入
		m_Types.m_bInsSpace = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_INS_SPACE );
	}

	//インデント
	{
		/* 自動インデント */
		m_Types.m_bAutoIndent = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENT );

		/* 日本語空白もインデント */
		m_Types.m_bAutoIndent_ZENSPACE = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_INDENT_WSPACE );

		/* スマートインデント種別 */
		HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_SMARTINDENT );
		int		nSelPos = Combo_GetCurSel( hwndCombo );
		if( nSelPos >= 0 ){
			m_Types.m_eSmartIndent = m_SIndentArr[nSelPos].nMethod;	/* スマートインデント種別 */
		}

		/* その他のインデント対象文字 */
		::DlgItem_GetText( hwndDlg, IDC_EDIT_INDENTCHARS, m_Types.m_szIndentChars, _countof( m_Types.m_szIndentChars ) );

		// 折り返し行インデント	//	Oct. 1, 2002 genta コンボボックスに変更
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_INDENTLAYOUT );
		nSelPos = Combo_GetCurSel( hwndCombo );
		m_Types.m_nIndentLayout = IndentTypeArr[nSelPos].nMethod;	/* 折り返し部インデント種別 */

		// 改行時に末尾の空白を削除	//2005.10.11 ryoji
		m_Types.m_bRTrimPrevLine = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_RTRIM_PREVLINE ) ? TRUE : FALSE;
	}

	//アウトライン解析方法
	//2002.04.01 YAZAKI ルールファイル関連追加
	{
		//標準ルール
		if ( !::IsDlgButtonChecked( hwndDlg, IDC_RADIO_OUTLINERULEFILE) ){
			HWND	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_OUTLINES );
			int		nSelPos = Combo_GetCurSel( hwndCombo );
			if( nSelPos >= 0 ){
				m_Types.m_eDefaultOutline = m_OlmArr[nSelPos].nMethod;	/* アウトライン解析方法 */
			}
		}
		//ルールファイル
		else {
			m_Types.m_eDefaultOutline = OUTLINE_FILE;
		}

		//ルールファイル	//2003.06.23 Moca ルールを使っていなくてもファイル名を保持
		::DlgItem_GetText( hwndDlg, IDC_EDIT_OUTLINERULEFILE, m_Types.m_szOutlineRuleFilename, _countof2( m_Types.m_szOutlineRuleFilename ));
	}

	//その他
	{
		/* 英文ワードラップをする */
		m_Types.m_bWordWrap = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_WORDWRAP ) ? true : false;

		/* 禁則処理 */
		{	//@@@ 2002.04.08 MIK start
			m_Types.m_bKinsokuHead = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUHEAD ) ? true : false;
			m_Types.m_bKinsokuTail = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUTAIL ) ? true : false;
			m_Types.m_bKinsokuRet  = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKURET  ) ? true : false;	// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
			m_Types.m_bKinsokuKuto = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUKUTO ) ? true : false;	// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
			m_Types.m_bKinsokuHide = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KINSOKUHIDE ) ? true : false;	// ぶら下げを隠す		// 2011/11/30 Uchi
			::DlgItem_GetText( hwndDlg, IDC_EDIT_KINSOKUHEAD, m_Types.m_szKinsokuHead, _countof( m_Types.m_szKinsokuHead ) );
			::DlgItem_GetText( hwndDlg, IDC_EDIT_KINSOKUTAIL, m_Types.m_szKinsokuTail, _countof( m_Types.m_szKinsokuTail ) );
			::DlgItem_GetText( hwndDlg, IDC_EDIT_KINSOKUKUTO, m_Types.m_szKinsokuKuto, _countof( m_Types.m_szKinsokuKuto ) );	// 2009.08.07 ryoji
		}	//@@@ 2002.04.08 MIK end

	}

	return TRUE;
}

//アウトライン解析ルールの追加
void CPropTypesScreen::AddOutlineMethod(int nMethod, const WCHAR* szName)
{
	if( m_OlmArr.empty() ){
		m_OlmArr.insert(m_OlmArr.end(), OlmArr, &OlmArr[_countof(OlmArr)]);	//アウトライン解析ルール
	}
	TYPE_NAME<EOutlineType> method;
	method.nMethod = (EOutlineType)nMethod;
	const TCHAR* tszName = to_tchar( szName );
	TCHAR* pszName = new TCHAR[ _tcslen(tszName) + 1 ];
	_tcscpy( pszName, tszName );
	method.pszName = pszName;
	m_OlmArr.push_back(method);
}

void CPropTypesScreen::RemoveOutlineMethod(int nMethod, const WCHAR* szName)
{
	int nSize = (int)m_OlmArr.size();
	for(int i = 0; i < nSize; i++ ){
		if( m_OlmArr[i].nMethod == (EOutlineType)nMethod ){
			delete [] m_OlmArr[i].pszName;
			m_OlmArr.erase( m_OlmArr.begin() + i );
			break;
		}
	}
}

//スマートインデントルールの追加
void CPropTypesScreen::AddSIndentMethod(int nMethod, const WCHAR* szName)
{
	if( m_SIndentArr.empty() ){
		m_SIndentArr.insert(m_SIndentArr.end(), SmartIndentArr, &SmartIndentArr[_countof(SmartIndentArr)]);	//スマートインデントルール
	}
	TYPE_NAME<ESmartIndentType> method;
	method.nMethod = (ESmartIndentType)nMethod;
	const TCHAR* tszName = to_tchar( szName );
	TCHAR* pszName = new TCHAR[ _tcslen(tszName) + 1 ];
	_tcscpy( pszName, tszName );
	method.pszName = pszName;
	m_SIndentArr.push_back(method);
}

void CPropTypesScreen::RemoveSIndentMethod(int nMethod, const WCHAR* szName)
{
	int nSize = (int)m_SIndentArr.size();
	for(int i = 0; i < nSize; i++ ){
		if( m_SIndentArr[i].nMethod == (EOutlineType)nMethod ){
			delete [] m_SIndentArr[i].pszName;
			m_SIndentArr.erase( m_SIndentArr.begin() + i );
			break;
		}
	}
}
