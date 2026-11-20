/*!
 *	@file
 *	@brief	ダークモード (画面の配色) の状態の管理と、ウィンドウへの適用
 *
 * 共通設定の「画面の配色」(ライト / ダーク / OS の設定に従う) と、OS のアプリのモード・ハイコントラストから
 * ダークモードを使うかを決め、プロセス内で共有する。ウィンドウの部品の描画では、システムカラーの代わりに
 * GetUiColor() / GetUiColorBrush() を使う (ライトのときはシステムカラーそのものを返す)。
 * タイトルバーやスクロールバーなど、Windows が描く部分は公開 API (DWM、SetWindowTheme) で暗くする。
 * 非公開 API (uxtheme.dll の序数指定の関数など) は使わない。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/20 新規作成
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
#include "uiparts/CDarkMode.h"
#include "env/DLLSHAREDATA.h"
#include "util/window.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <vsstyle.h>

//! タイトルバーをダークにする DWM の属性 (Windows 10 20H1 以降。それより前の Windows 10 は 19)
static const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE = 20;
static const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_OLD_VALUE = 19;
//! ウィンドウの枠の色を指定する DWM の属性 (Windows 11 以降)
static const DWORD DWMWA_BORDER_COLOR_VALUE = 34;

bool	CDarkMode::s_bEnabled = false;
HBRUSH	CDarkMode::s_ahBrush[32] = { NULL };

/*!
 * CDarkMode::Refresh - 設定と OS の状態から、ダークモードを使うかを決め直す
 *
 * 共通設定の「画面の配色」、OS のアプリのモード、ハイコントラストから判定する。
 * 状態が変わった場合は、ダークモード用のブラシを作り直すために破棄する。
 *
 *	@return	true: 状態が変わった / false: 変わらない
 */
bool CDarkMode::Refresh()
{
	HIGHCONTRAST hc = { sizeof( hc ) };
	bool bHighContrast = ::SystemParametersInfo( SPI_GETHIGHCONTRAST, sizeof( hc ), &hc, 0 ) && ( hc.dwFlags & HCF_HIGHCONTRASTON );
	bool bEnabled = IsDarkModeEnabledFor( GetDllShareData().m_Common.m_sColorTheme.m_nUiColorMode, IsOsDarkMode(), bHighContrast );
	if( bEnabled == s_bEnabled ){
		return false;
	}
	s_bEnabled = bEnabled;
	DeleteBrushes();
	return true;
}

/*!
 * CDarkMode::IsEnabled - ダークモードを使うかを返す
 *
 *	@return	true: ダークモード / false: ライト
 */
bool CDarkMode::IsEnabled()
{
	return s_bEnabled;
}

/*!
 * CDarkMode::IsOsDarkMode - OS のアプリのモードがダークかを返す
 *
 * レジストリの HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize の AppsUseLightTheme が
 * 0 のときダークとする。値がない場合はライトとする。
 *
 *	@return	true: ダーク / false: ライト
 */
bool CDarkMode::IsOsDarkMode()
{
	DWORD dwValue = 1;
	DWORD cbData = sizeof( dwValue );
	LSTATUS lRet = ::RegGetValueW( HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &dwValue, &cbData );
	return ERROR_SUCCESS == lRet && 0 == dwValue;
}

/*!
 * CDarkMode::GetUiColor - ウィンドウの部品の描画に使う色を返す
 *
 * ダークモードのときは、システムカラーの番号に対応するダークモードの色を返す。
 * ライトのとき、またはダークモードの配色表にない番号のときは、システムカラーを返す。
 *
 *	@return	色
 */
COLORREF CDarkMode::GetUiColor(
	int		nSysColorIndex	//!< [in] システムカラーの番号 (COLOR_*)
)
{
	COLORREF col;
	if( s_bEnabled && GetDarkModeSysColor( nSysColorIndex, &col ) ){
		return col;
	}
	return ::GetSysColor( nSysColorIndex );
}

/*!
 * CDarkMode::GetUiColorBrush - ウィンドウの部品の描画に使うブラシを返す
 *
 * GetUiColor() の色のブラシを返す。ブラシは CDarkMode が管理するので、呼び出し側で破棄しないこと。
 *
 *	@return	ブラシ
 */
HBRUSH CDarkMode::GetUiColorBrush(
	int		nSysColorIndex	//!< [in] システムカラーの番号 (COLOR_*)
)
{
	COLORREF col;
	if( !s_bEnabled || nSysColorIndex < 0 || _countof( s_ahBrush ) <= nSysColorIndex
	 || !GetDarkModeSysColor( nSysColorIndex, &col ) ){
		return ::GetSysColorBrush( nSysColorIndex );
	}
	if( NULL == s_ahBrush[nSysColorIndex] ){
		s_ahBrush[nSysColorIndex] = ::CreateSolidBrush( col );
	}
	return s_ahBrush[nSysColorIndex];
}

/*!
 * CDarkMode::ApplyTitleBar - タイトルバーの配色を、ダークモードの状態に合わせて設定する
 *
 * DwmSetWindowAttribute() の DWMWA_USE_IMMERSIVE_DARK_MODE で設定する。
 * 設定の後、タイトルバーを描き直すために枠の再計算を要求する。
 */
void CDarkMode::ApplyTitleBar(
	HWND	hwnd	//!< [in] トップレベルウィンドウ
)
{
	BOOL bDark = s_bEnabled ? TRUE : FALSE;
	if( FAILED( ::DwmSetWindowAttribute( hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_VALUE, &bDark, sizeof( bDark ) ) ) ){
		::DwmSetWindowAttribute( hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_OLD_VALUE, &bDark, sizeof( bDark ) );
	}
	::SetWindowPos( hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED );
}

/*!
 * CDarkMode::ApplyControlTheme - スクロールバーなどのコントロールのテーマを、ダークモードの状態に合わせて設定する
 *
 * ダークモードのときは Windows のテーマの "DarkMode_Explorer" を、ライトのときは標準のテーマを設定する。
 * "DarkMode_Explorer" がない Windows では、標準のテーマで表示される。
 */
void CDarkMode::ApplyControlTheme(
	HWND	hwnd	//!< [in] コントロール (スクロールバーなど)
)
{
	if( NULL == hwnd ){
		return;
	}
	::SetWindowTheme( hwnd, s_bEnabled ? L"DarkMode_Explorer" : NULL, NULL );
}

/*!
 * CDarkMode::ApplyMenuBackground - メニューの背景のブラシを、ダークモードの状態に合わせて設定する
 *
 * ダークモードのときは項目の外側の余白などを暗く塗るブラシを、ライトのときは標準 (NULL) を設定する。
 */
void CDarkMode::ApplyMenuBackground(
	HMENU	hMenu	//!< [in] メニュー (メニューバーまたはポップアップメニュー)
)
{
	if( NULL == hMenu ){
		return;
	}
	MENUINFO mi = { sizeof( mi ) };
	mi.fMask = MIM_BACKGROUND;
	mi.hbrBack = s_bEnabled ? GetUiColorBrush( COLOR_MENU ) : NULL;
	::SetMenuInfo( hMenu, &mi );
}

/*!
 * CDarkMode::ApplyMenuWindowBorder - 開いているメニューのウィンドウの枠の色を、ダークモードのときに暗くする
 *
 * DwmSetWindowAttribute() の DWMWA_BORDER_COLOR (Windows 11 以降) で設定する。
 * Windows 10 では失敗し、枠は Windows の配色のままになる。
 */
void CDarkMode::ApplyMenuWindowBorder(
	HWND	hwndMenu	//!< [in] メニューのウィンドウ (クラス #32768)
)
{
	if( !s_bEnabled || NULL == hwndMenu ){
		return;
	}
	COLORREF col = GetUiColor( COLOR_3DSHADOW );
	::DwmSetWindowAttribute( hwndMenu, DWMWA_BORDER_COLOR_VALUE, &col, sizeof( col ) );
}

/*!
 * CDarkMode::AttachSizeBox - サイズボックスを、ダークモードのときに暗く描くようにする
 *
 * サイズボックス (SBS_SIZEBOX の SCROLLBAR) は Windows のテーマで明るく描かれるため、サブクラス化して、
 * ダークモードのときだけ暗い背景とつかむ位置の模様を自分で描く。ライトのときは本来の描画のまま。
 * 起動中にライト・ダークを切り替えても、描画のたびに判定するので付け直す必要はない。
 */
void CDarkMode::AttachSizeBox(
	HWND	hwndSizeBox		//!< [in] サイズボックス
)
{
	if( NULL == hwndSizeBox ){
		return;
	}
	::SetWindowSubclass( hwndSizeBox, SizeBoxSubclassProc, 0, 0 );
}

/*!
 * CDarkMode::SizeBoxSubclassProc - サイズボックスのサブクラスプロシージャ
 *
 * ダークモードのときは、背景の消去を省き、WM_PAINT で自分で描画する (PaintSizeBoxDark)。
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CDarkMode::SizeBoxSubclassProc(
	HWND		hwnd,			//!< [in] サイズボックス
	UINT		uMsg,			//!< [in] メッセージ
	WPARAM		wParam,			//!< [in] パラメータ
	LPARAM		lParam,			//!< [in] パラメータ
	UINT_PTR	uIdSubclass,	//!< [in] サブクラスの ID
	DWORD_PTR	dwRefData		//!< [in] 未使用
)
{
	switch( uMsg ){
	case WM_ERASEBKGND:
		if( s_bEnabled ){
			return 1;
		}
		break;
	case WM_PAINT:
		if( s_bEnabled ){
			PaintSizeBoxDark( hwnd );
			return 0;
		}
		break;
	case WM_NCDESTROY:
		::RemoveWindowSubclass( hwnd, SizeBoxSubclassProc, uIdSubclass );
		break;
	}
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

/*!
 * CDarkMode::PaintSizeBoxDark - ダークモードのとき、サイズボックスを描画する
 *
 * 背景を暗い色で塗り、右下に三角形に並べた点 (つかむ位置の模様) を描く。
 */
void CDarkMode::PaintSizeBoxDark(
	HWND	hwnd	//!< [in] サイズボックス
)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint( hwnd, &ps );
	RECT rc;
	::GetClientRect( hwnd, &rc );
	::FillRect( hdc, &rc, GetUiColorBrush( COLOR_3DFACE ) );
	DrawSizeGrip( hdc, rc );
	::EndPaint( hwnd, &ps );
}

/*!
 * CDarkMode::DrawSizeGrip - ダークモードの、サイズ変更のつかむ位置の模様を描く
 *
 * 指定した矩形の右下に、3 段の三角形に並べた点を描く。背景は塗らない。
 * サイズボックスと、ステータスバーの右下 (SBARS_SIZEGRIP) で使う。
 */
void CDarkMode::DrawSizeGrip(
	HDC			hdc,	//!< [in] 描画先
	const RECT&	rc		//!< [in] 模様を描く領域 (右下に寄せて描く)
)
{
	const int nDot = DpiScaleX( 2 );
	const int nStep = DpiScaleX( 4 );
	HBRUSH hbrDot = GetUiColorBrush( COLOR_GRAYTEXT );
	for( int nRow = 0; nRow < 3; nRow++ ){
		for( int nCol = 0; nCol <= nRow; nCol++ ){
			RECT rcDot;
			rcDot.right = rc.right - DpiScaleX( 2 ) - nCol * nStep;
			rcDot.bottom = rc.bottom - DpiScaleY( 2 ) - ( nRow - nCol ) * nStep;
			rcDot.left = rcDot.right - nDot;
			rcDot.top = rcDot.bottom - nDot;
			::FillRect( hdc, &rcDot, hbrDot );
		}
	}
}

/*!
 * CDarkMode::ApplyDialog - ダイアログのタイトルバーと子コントロールに画面の配色を適用する
 *
 * ダークモードのときだけ、タイトルバーを暗くし、子コントロールのテーマと色を設定する (ApplyChildControls)。
 * ライトのときは何もしない (Windows の標準の表示のまま)。ダイアログの WM_INITDIALOG で呼ぶ。
 * ダイアログの背景や文字の色は、WM_CTLCOLOR* (OnCtlColor) で別に設定する。
 */
void CDarkMode::ApplyDialog(
	HWND	hDlg	//!< [in] ダイアログ
)
{
	if( !s_bEnabled || NULL == hDlg ){
		return;
	}
	if( 0 == ( ::GetWindowLongPtr( hDlg, GWL_STYLE ) & WS_CHILD ) ){
		ApplyTitleBar( hDlg );
	}
	ApplyChildControls( hDlg );
}

/*!
 * CDarkMode::ApplyChildControls - 子コントロールに画面の配色を適用する
 *
 * ダークモードのときだけ、子孫のウィンドウをすべて列挙し、クラスに応じて
 * Windows のダークのテーマ (DarkMode_Explorer / DarkMode_CFD) や背景色・文字色を設定する。
 */
void CDarkMode::ApplyChildControls(
	HWND	hwndParent	//!< [in] 親ウィンドウ
)
{
	if( !s_bEnabled || NULL == hwndParent ){
		return;
	}
	::EnumChildWindows( hwndParent, ApplyChildControlProc, 0 );
}

/*!
 * CDarkMode::ApplyChildControlProc - 子コントロール 1 つに画面の配色を適用する (EnumChildWindows のコールバック)
 *
 * ボタン: グループボックスはサブクラス化して自分で描く。それ以外はダークのテーマを設定する
 * (チェックボックス・ラジオボタンの文字は NM_CUSTOMDRAW で描く)。
 * エディット・リストボックス・コンボボックス・リストビュー・ツリービューは、ダークのテーマと色を設定する。
 * タブコントロールはサブクラス化して自分で描く。
 *
 *	@return	TRUE: 列挙を続ける
 */
BOOL CALLBACK CDarkMode::ApplyChildControlProc(
	HWND	hwnd,	//!< [in] 子コントロール
	LPARAM	lParam	//!< [in] 未使用
)
{
	WCHAR szClass[64];
	if( 0 == ::GetClassName( hwnd, szClass, _countof( szClass ) ) ){
		return TRUE;
	}
	LONG_PTR lStyle = ::GetWindowLongPtr( hwnd, GWL_STYLE );
	if( 0 == _wcsicmp( szClass, WC_BUTTON ) ){
		switch( lStyle & BS_TYPEMASK ){
		case BS_GROUPBOX:
			::SetWindowSubclass( hwnd, GroupBoxSubclassProc, 0, 0 );
			break;
		case BS_OWNERDRAW:
			break;
		default:
			::SetWindowTheme( hwnd, L"DarkMode_Explorer", NULL );
			break;
		}
	}else if( 0 == _wcsicmp( szClass, WC_EDIT ) ){
		::SetWindowTheme( hwnd, ( lStyle & ES_MULTILINE ) ? L"DarkMode_Explorer" : L"DarkMode_CFD", NULL );
	}else if( 0 == _wcsicmp( szClass, WC_LISTBOX ) ){
		::SetWindowTheme( hwnd, L"DarkMode_Explorer", NULL );
	}else if( 0 == _wcsicmp( szClass, WC_COMBOBOX ) ){
		::SetWindowTheme( hwnd, L"DarkMode_CFD", NULL );
		COMBOBOXINFO cbi = { sizeof( cbi ) };
		if( ::GetComboBoxInfo( hwnd, &cbi ) && NULL != cbi.hwndList ){
			::SetWindowTheme( cbi.hwndList, L"DarkMode_Explorer", NULL );
		}
	}else if( 0 == _wcsicmp( szClass, WC_LISTVIEW ) ){
		::SetWindowTheme( hwnd, L"DarkMode_Explorer", NULL );
		ListView_SetBkColor( hwnd, GetUiColor( COLOR_WINDOW ) );
		ListView_SetTextBkColor( hwnd, GetUiColor( COLOR_WINDOW ) );
		ListView_SetTextColor( hwnd, GetUiColor( COLOR_WINDOWTEXT ) );
		HWND hwndHeader = ListView_GetHeader( hwnd );
		if( NULL != hwndHeader ){
			::SetWindowTheme( hwndHeader, L"DarkMode_ItemsView", NULL );
		}
		::SetWindowSubclass( hwnd, ListViewSubclassProc, 0, 0 );	// 見出しの文字色
	}else if( 0 == _wcsicmp( szClass, WC_TABCONTROL ) ){
		::SetWindowSubclass( hwnd, TabControlSubclassProc, 0, 0 );
	}else if( 0 == _wcsicmp( szClass, WC_TREEVIEW ) ){
		::SetWindowTheme( hwnd, L"DarkMode_Explorer", NULL );
		TreeView_SetBkColor( hwnd, GetUiColor( COLOR_WINDOW ) );
		TreeView_SetTextColor( hwnd, GetUiColor( COLOR_WINDOWTEXT ) );
		TreeView_SetLineColor( hwnd, GetUiColor( COLOR_GRAYTEXT ) );
	}else if( 0 == _wcsicmp( szClass, UPDOWN_CLASS ) || 0 == _wcsicmp( szClass, WC_SCROLLBAR ) ){
		::SetWindowTheme( hwnd, L"DarkMode_Explorer", NULL );
	}
	return TRUE;
}

/*!
 * CDarkMode::ApplyTooltip - ツールチップに画面の配色を適用する
 *
 * ダークモードのときは Windows のテーマの "DarkMode_Explorer" を、ライトのときは標準のテーマを設定する。
 */
void CDarkMode::ApplyTooltip(
	HWND	hwndTooltip	//!< [in] ツールチップ (tooltips_class32)
)
{
	if( NULL == hwndTooltip ){
		return;
	}
	::SetWindowTheme( hwndTooltip, s_bEnabled ? L"DarkMode_Explorer" : NULL, NULL );
}

/*!
 * CDarkMode::OnCtlColor - ダイアログの WM_CTLCOLOR* を処理する
 *
 * ダークモードのときは、文字色・背景色を設定し、背景のブラシを返す。
 * エディット・リストボックスはウィンドウの背景色、それ以外 (ダイアログ・スタティック・ボタン) は 3D の面の色とする。
 *
 *	@return	背景のブラシ / 0: ライト (既定の処理を行う)
 */
INT_PTR CDarkMode::OnCtlColor(
	UINT	uMsg,	//!< [in] メッセージ (WM_CTLCOLOR*)
	WPARAM	wParam,	//!< [in] 描画先の HDC
	LPARAM	lParam	//!< [in] コントロール
)
{
	if( !s_bEnabled ){
		return 0;
	}
	int nBack;
	int nText;
	switch( uMsg ){
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
		nBack = COLOR_WINDOW;
		nText = COLOR_WINDOWTEXT;
		break;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		nBack = COLOR_3DFACE;
		nText = COLOR_BTNTEXT;
		break;
	default:
		return 0;
	}
	HDC hdc = (HDC)wParam;
	::SetTextColor( hdc, GetUiColor( nText ) );
	::SetBkColor( hdc, GetUiColor( nBack ) );
	return (INT_PTR)GetUiColorBrush( nBack );
}

/*!
 * CDarkMode::OnButtonCustomDraw - ボタンの NM_CUSTOMDRAW を処理する
 *
 * ダークモードのとき、チェックボックス・ラジオボタンを自分で描く (テーマの描画では文字が黒くなるため)。
 * 押しボタンやライトのときは処理しない。
 *
 *	@return	true: 処理した (*plResult に戻り値を設定) / false: 処理していない
 */
bool CDarkMode::OnButtonCustomDraw(
	LPNMCUSTOMDRAW	pnmcd,		//!< [in] NM_CUSTOMDRAW の情報
	LRESULT*		plResult	//!< [out] メッセージの戻り値
)
{
	if( !s_bEnabled || NULL == pnmcd || NM_CUSTOMDRAW != pnmcd->hdr.code ){
		return false;
	}
	WCHAR szClass[16];
	if( 0 == ::GetClassName( pnmcd->hdr.hwndFrom, szClass, _countof( szClass ) ) || 0 != _wcsicmp( szClass, WC_BUTTON ) ){
		return false;
	}
	LONG_PTR lStyle = ::GetWindowLongPtr( pnmcd->hdr.hwndFrom, GWL_STYLE );
	if( lStyle & BS_PUSHLIKE ){
		return false;
	}
	switch( lStyle & BS_TYPEMASK ){
	case BS_CHECKBOX:
	case BS_AUTOCHECKBOX:
	case BS_3STATE:
	case BS_AUTO3STATE:
	case BS_RADIOBUTTON:
	case BS_AUTORADIOBUTTON:
		break;
	default:
		return false;
	}
	if( CDDS_PREPAINT != pnmcd->dwDrawStage ){
		*plResult = CDRF_DODEFAULT;
		return true;
	}
	DrawCheckRadioDark( pnmcd, lStyle );
	*plResult = CDRF_SKIPDEFAULT;
	return true;
}

/*!
 * CDarkMode::DrawCheckRadioDark - ダークモードのチェックボックス・ラジオボタンを描く
 *
 * 背景を 3D の面の色で塗り、チェックの印をテーマ (DarkMode_Explorer の Button) で描き、文字を明るい色で描く。
 * フォーカスがあれば、文字の周りにフォーカスの枠を描く。
 */
void CDarkMode::DrawCheckRadioDark(
	LPNMCUSTOMDRAW	pnmcd,	//!< [in] NM_CUSTOMDRAW の情報
	LONG_PTR		lStyle	//!< [in] ボタンのスタイル
)
{
	HWND hwnd = pnmcd->hdr.hwndFrom;
	HDC hdc = pnmcd->hdc;
	RECT rc = pnmcd->rc;
	bool bRadio = ( BS_RADIOBUTTON == ( lStyle & BS_TYPEMASK ) || BS_AUTORADIOBUTTON == ( lStyle & BS_TYPEMASK ) );
	bool bRightButton = ( 0 != ( lStyle & BS_RIGHTBUTTON ) );
	bool bDisabled = ( 0 != ( pnmcd->uItemState & CDIS_DISABLED ) );

	::FillRect( hdc, &rc, GetUiColorBrush( COLOR_3DFACE ) );

	// チェックの印
	LRESULT lCheck = ::SendMessage( hwnd, BM_GETCHECK, 0, 0 );
	int nState;	// CBS_* / RBS_* (未チェック・チェック・不定 ごとに 4 つずつ、通常・ホット・押下・無効の順)
	if( BST_INDETERMINATE == lCheck && !bRadio ){
		nState = CBS_MIXEDNORMAL;
	}else if( BST_CHECKED == lCheck ){
		nState = bRadio ? RBS_CHECKEDNORMAL : CBS_CHECKEDNORMAL;
	}else{
		nState = bRadio ? RBS_UNCHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
	}
	if( bDisabled ){
		nState += 3;
	}else if( pnmcd->uItemState & CDIS_SELECTED ){
		nState += 2;
	}else if( pnmcd->uItemState & CDIS_HOT ){
		nState += 1;
	}
	int nPart = bRadio ? BP_RADIOBUTTON : BP_CHECKBOX;
	SIZE sizeBox = { DpiScaleX( 13 ), DpiScaleY( 13 ) };
	HTHEME hTheme = ::OpenThemeData( hwnd, L"Button" );
	if( NULL != hTheme ){
		::GetThemePartSize( hTheme, hdc, nPart, nState, NULL, TS_DRAW, &sizeBox );
	}
	RECT rcBox;
	rcBox.top = rc.top + ( rc.bottom - rc.top - sizeBox.cy ) / 2;
	rcBox.bottom = rcBox.top + sizeBox.cy;
	if( bRightButton ){
		rcBox.right = rc.right;
		rcBox.left = rcBox.right - sizeBox.cx;
	}else{
		rcBox.left = rc.left;
		rcBox.right = rcBox.left + sizeBox.cx;
	}
	if( NULL != hTheme ){
		::DrawThemeBackground( hTheme, hdc, nPart, nState, &rcBox, NULL );
		::CloseThemeData( hTheme );
	}else{
		UINT uState = ( bRadio ? DFCS_BUTTONRADIO : DFCS_BUTTONCHECK ) | ( BST_CHECKED == lCheck ? DFCS_CHECKED : 0 ) | ( bDisabled ? DFCS_INACTIVE : 0 );
		::DrawFrameControl( hdc, &rcBox, DFC_BUTTON, uState );
	}

	// 文字
	RECT rcText = rc;
	const int nGap = DpiScaleX( 4 );
	if( bRightButton ){
		rcText.right = rcBox.left - nGap;
	}else{
		rcText.left = rcBox.right + nGap;
	}
	int nLen = ::GetWindowTextLength( hwnd );
	std::vector<WCHAR> vecText( nLen + 1 );
	::GetWindowText( hwnd, &vecText[0], nLen + 1 );

	LRESULT lUiState = ::SendMessage( hwnd, WM_QUERYUISTATE, 0, 0 );
	UINT uFormat = ( lStyle & BS_MULTILINE ) ? DT_WORDBREAK : DT_SINGLELINE;
	if( bRightButton ){
		uFormat |= DT_RIGHT;
	}
	if( lUiState & UISF_HIDEACCEL ){
		uFormat |= DT_HIDEPREFIX;
	}
	HFONT hFont = (HFONT)::SendMessage( hwnd, WM_GETFONT, 0, 0 );
	HFONT hFontOld = ( NULL != hFont ) ? (HFONT)::SelectObject( hdc, hFont ) : NULL;
	RECT rcCalc = rcText;
	::DrawText( hdc, &vecText[0], nLen, &rcCalc, uFormat | DT_CALCRECT );
	int nTextHeight = rcCalc.bottom - rcCalc.top;
	rcText.top = rc.top + ( rc.bottom - rc.top - nTextHeight ) / 2;
	rcText.bottom = rcText.top + nTextHeight;
	::SetBkMode( hdc, TRANSPARENT );
	::SetTextColor( hdc, GetUiColor( bDisabled ? COLOR_GRAYTEXT : COLOR_BTNTEXT ) );
	::DrawText( hdc, &vecText[0], nLen, &rcText, uFormat );

	// フォーカスの枠
	if( ( pnmcd->uItemState & CDIS_FOCUS ) && 0 == ( lUiState & UISF_HIDEFOCUS ) && 0 < nLen ){
		RECT rcFocus = rcText;
		int nWidth = rcCalc.right - rcCalc.left;
		if( bRightButton ){
			rcFocus.left = rcFocus.right - nWidth;
		}else{
			rcFocus.right = rcFocus.left + nWidth;
		}
		::InflateRect( &rcFocus, 1, 1 );
		::IntersectRect( &rcFocus, &rcFocus, &rc );
		::SetTextColor( hdc, GetUiColor( COLOR_BTNTEXT ) );
		::SetBkColor( hdc, GetUiColor( COLOR_3DFACE ) );
		::DrawFocusRect( hdc, &rcFocus );
	}
	if( NULL != hFontOld ){
		::SelectObject( hdc, hFontOld );
	}
}

/*!
 * CDarkMode::GroupBoxSubclassProc - グループボックスのサブクラスプロシージャ
 *
 * ダークモードのときは、WM_PAINT で枠と見出しを自分で描く (PaintGroupBoxDark)。
 * テーマの描画では見出しの文字色を変えられないため。
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CDarkMode::GroupBoxSubclassProc(
	HWND		hwnd,			//!< [in] グループボックス
	UINT		uMsg,			//!< [in] メッセージ
	WPARAM		wParam,			//!< [in] パラメータ
	LPARAM		lParam,			//!< [in] パラメータ
	UINT_PTR	uIdSubclass,	//!< [in] サブクラスの ID
	DWORD_PTR	dwRefData		//!< [in] 未使用
)
{
	switch( uMsg ){
	case WM_PAINT:
		if( s_bEnabled ){
			PaintGroupBoxDark( hwnd );
			return 0;
		}
		break;
	case WM_ENABLE:
	case WM_SETTEXT:
		if( s_bEnabled ){
			// 標準の処理がテーマの配色で直接描くので、親ウィンドウのその範囲ごと描き直す
			LRESULT lRet = ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
			HWND hwndParent = ::GetParent( hwnd );
			RECT rc;
			::GetWindowRect( hwnd, &rc );
			::MapWindowPoints( NULL, hwndParent, (LPPOINT)&rc, 2 );
			::RedrawWindow( hwndParent, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
			return lRet;
		}
		break;
	case WM_NCDESTROY:
		::RemoveWindowSubclass( hwnd, GroupBoxSubclassProc, uIdSubclass );
		break;
	}
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

/*!
 * CDarkMode::PaintGroupBoxDark - ダークモードのとき、グループボックスを描画する
 *
 * 見出しの高さの中央を上端として 1 ドットの枠を描き、左上に見出しを明るい色で描く。
 * 内側は塗らない (中のコントロールを消さないため。背景は親ウィンドウが塗る)。
 */
void CDarkMode::PaintGroupBoxDark(
	HWND	hwnd	//!< [in] グループボックス
)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint( hwnd, &ps );
	RECT rc;
	::GetClientRect( hwnd, &rc );

	HFONT hFont = (HFONT)::SendMessage( hwnd, WM_GETFONT, 0, 0 );
	HFONT hFontOld = ( NULL != hFont ) ? (HFONT)::SelectObject( hdc, hFont ) : NULL;

	int nLen = ::GetWindowTextLength( hwnd );
	std::vector<WCHAR> vecText( nLen + 1 );
	::GetWindowText( hwnd, &vecText[0], nLen + 1 );

	UINT uFormat = DT_SINGLELINE | DT_LEFT | DT_TOP;
	if( ::SendMessage( hwnd, WM_QUERYUISTATE, 0, 0 ) & UISF_HIDEACCEL ){
		uFormat |= DT_HIDEPREFIX;
	}
	const int nTextLeft = rc.left + DpiScaleX( 9 );
	RECT rcText = { nTextLeft, rc.top, rc.right - DpiScaleX( 9 ), rc.bottom };
	::DrawText( hdc, &vecText[0], nLen, &rcText, uFormat | DT_CALCRECT );
	if( rcText.right > rc.right - DpiScaleX( 9 ) ){
		rcText.right = rc.right - DpiScaleX( 9 );
	}
	int nTextHeight = rcText.bottom - rcText.top;
	if( 0 == nLen ){
		TEXTMETRIC tm;
		::GetTextMetrics( hdc, &tm );
		nTextHeight = tm.tmHeight;
	}

	// 枠 (見出しの部分は空ける)
	HBRUSH hbrFrame = GetUiColorBrush( COLOR_3DSHADOW );
	int nFrameTop = rc.top + nTextHeight / 2;
	RECT rcLine;
	::SetRect( &rcLine, rc.left, nFrameTop, rc.left + 1, rc.bottom );			::FillRect( hdc, &rcLine, hbrFrame );	// 左
	::SetRect( &rcLine, rc.right - 1, nFrameTop, rc.right, rc.bottom );			::FillRect( hdc, &rcLine, hbrFrame );	// 右
	::SetRect( &rcLine, rc.left, rc.bottom - 1, rc.right, rc.bottom );			::FillRect( hdc, &rcLine, hbrFrame );	// 下
	if( 0 < nLen ){
		const int nPad = DpiScaleX( 2 );
		::SetRect( &rcLine, rc.left, nFrameTop, rcText.left - nPad, nFrameTop + 1 );	::FillRect( hdc, &rcLine, hbrFrame );	// 上 (見出しの左)
		::SetRect( &rcLine, rcText.right + nPad, nFrameTop, rc.right, nFrameTop + 1 );	::FillRect( hdc, &rcLine, hbrFrame );	// 上 (見出しの右)
	}else{
		::SetRect( &rcLine, rc.left, nFrameTop, rc.right, nFrameTop + 1 );			::FillRect( hdc, &rcLine, hbrFrame );	// 上
	}

	// 見出し
	if( 0 < nLen ){
		::SetBkMode( hdc, TRANSPARENT );
		::SetTextColor( hdc, GetUiColor( ::IsWindowEnabled( hwnd ) ? COLOR_BTNTEXT : COLOR_GRAYTEXT ) );
		::DrawText( hdc, &vecText[0], nLen, &rcText, uFormat );
	}

	if( NULL != hFontOld ){
		::SelectObject( hdc, hFontOld );
	}
	::EndPaint( hwnd, &ps );
}

/*!
 * CDarkMode::ListViewSubclassProc - リストビューのサブクラスプロシージャ
 *
 * ダークモードのとき、見出し (ヘッダーコントロール) の NM_CUSTOMDRAW で文字色を明るくする。
 * 見出しの背景は、テーマ (DarkMode_ItemsView) で暗く描かれる。
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CDarkMode::ListViewSubclassProc(
	HWND		hwnd,			//!< [in] リストビュー
	UINT		uMsg,			//!< [in] メッセージ
	WPARAM		wParam,			//!< [in] パラメータ
	LPARAM		lParam,			//!< [in] パラメータ
	UINT_PTR	uIdSubclass,	//!< [in] サブクラスの ID
	DWORD_PTR	dwRefData		//!< [in] 未使用
)
{
	switch( uMsg ){
	case WM_NOTIFY:
		if( s_bEnabled ){
			LPNMCUSTOMDRAW pnmcd = (LPNMCUSTOMDRAW)lParam;
			if( NM_CUSTOMDRAW == pnmcd->hdr.code && pnmcd->hdr.hwndFrom == ListView_GetHeader( hwnd ) ){
				switch( pnmcd->dwDrawStage ){
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					::SetTextColor( pnmcd->hdc, GetUiColor( COLOR_WINDOWTEXT ) );
					return CDRF_DODEFAULT;
				}
			}
		}
		break;
	case WM_NCDESTROY:
		::RemoveWindowSubclass( hwnd, ListViewSubclassProc, uIdSubclass );
		break;
	}
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

/*!
 * CDarkMode::TabControlSubclassProc - タブコントロールのサブクラスプロシージャ
 *
 * ダークモードのときは、背景の消去を省き、WM_PAINT で自分で描画する (PaintTabControlDark)。
 * テーマの描画では、タブの背景と文字の色を変えられないため。
 *
 *	@return	メッセージの処理結果
 */
LRESULT CALLBACK CDarkMode::TabControlSubclassProc(
	HWND		hwnd,			//!< [in] タブコントロール
	UINT		uMsg,			//!< [in] メッセージ
	WPARAM		wParam,			//!< [in] パラメータ
	LPARAM		lParam,			//!< [in] パラメータ
	UINT_PTR	uIdSubclass,	//!< [in] サブクラスの ID
	DWORD_PTR	dwRefData		//!< [in] 未使用
)
{
	switch( uMsg ){
	case WM_ERASEBKGND:
		if( s_bEnabled ){
			return 1;
		}
		break;
	case WM_PAINT:
		if( s_bEnabled ){
			PaintTabControlDark( hwnd );
			return 0;
		}
		break;
	case WM_NCDESTROY:
		::RemoveWindowSubclass( hwnd, TabControlSubclassProc, uIdSubclass );
		break;
	}
	return ::DefSubclassProc( hwnd, uMsg, wParam, lParam );
}

/*!
 * CDarkMode::PaintTabControlDark - ダークモードのとき、タブコントロールを描画する
 *
 * 背景を 3D の面の色で塗り、ページの領域を 1 ドットの枠で囲む。各タブは、選択中のものを面の色で
 * ページとつなげて描き、それ以外をウィンドウの背景色で描く。タブの文字は明るい色で中央に描く。
 * タブの画像 (イメージリスト) は描かない。
 */
void CDarkMode::PaintTabControlDark(
	HWND	hwnd	//!< [in] タブコントロール
)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint( hwnd, &ps );
	RECT rc;
	::GetClientRect( hwnd, &rc );
	::FillRect( hdc, &rc, GetUiColorBrush( COLOR_3DFACE ) );

	HBRUSH hbrFrame = GetUiColorBrush( COLOR_3DSHADOW );
	int nCount = TabCtrl_GetItemCount( hwnd );
	int nCurSel = TabCtrl_GetCurSel( hwnd );

	// ページの領域の枠 (タブの並びの下から)
	RECT rcPane = rc;
	for( int i = 0; i < nCount; i++ ){
		RECT rcItem;
		if( TabCtrl_GetItemRect( hwnd, i, &rcItem ) && rcPane.top < rcItem.bottom ){
			rcPane.top = rcItem.bottom;
		}
	}
	::FrameRect( hdc, &rcPane, hbrFrame );

	HFONT hFont = (HFONT)::SendMessage( hwnd, WM_GETFONT, 0, 0 );
	HFONT hFontOld = ( NULL != hFont ) ? (HFONT)::SelectObject( hdc, hFont ) : NULL;
	::SetBkMode( hdc, TRANSPARENT );
	UINT uFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS;
	if( ::SendMessage( hwnd, WM_QUERYUISTATE, 0, 0 ) & UISF_HIDEACCEL ){
		uFormat |= DT_HIDEPREFIX;
	}

	for( int i = 0; i < nCount; i++ ){
		RECT rcItem;
		if( !TabCtrl_GetItemRect( hwnd, i, &rcItem ) ){
			continue;
		}
		bool bSelected = ( i == nCurSel );
		if( bSelected ){
			// 選択中のタブは少し大きくし、下の枠を消してページとつなげる
			rcItem.top -= DpiScaleY( 2 );
			rcItem.left -= DpiScaleX( 2 );
			rcItem.right += DpiScaleX( 2 );
			rcItem.bottom += 1;
		}
		::FillRect( hdc, &rcItem, GetUiColorBrush( bSelected ? COLOR_3DFACE : COLOR_WINDOW ) );
		RECT rcLine;
		::SetRect( &rcLine, rcItem.left, rcItem.top, rcItem.right, rcItem.top + 1 );		::FillRect( hdc, &rcLine, hbrFrame );	// 上
		::SetRect( &rcLine, rcItem.left, rcItem.top, rcItem.left + 1, rcItem.bottom );		::FillRect( hdc, &rcLine, hbrFrame );	// 左
		::SetRect( &rcLine, rcItem.right - 1, rcItem.top, rcItem.right, rcItem.bottom );	::FillRect( hdc, &rcLine, hbrFrame );	// 右

		WCHAR szText[256] = L"";
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = szText;
		tci.cchTextMax = _countof( szText );
		TabCtrl_GetItem( hwnd, i, &tci );
		RECT rcText = rcItem;
		::InflateRect( &rcText, -DpiScaleX( 4 ), 0 );
		::SetTextColor( hdc, GetUiColor( bSelected ? COLOR_BTNTEXT : COLOR_GRAYTEXT ) );
		::DrawText( hdc, szText, -1, &rcText, uFormat );

		if( bSelected && ::GetFocus() == hwnd && 0 == ( ::SendMessage( hwnd, WM_QUERYUISTATE, 0, 0 ) & UISF_HIDEFOCUS ) ){
			RECT rcFocus = rcItem;
			::InflateRect( &rcFocus, -DpiScaleX( 3 ), -DpiScaleY( 3 ) );
			::SetTextColor( hdc, GetUiColor( COLOR_BTNTEXT ) );
			::SetBkColor( hdc, GetUiColor( COLOR_3DFACE ) );
			::DrawFocusRect( hdc, &rcFocus );
		}
	}

	if( NULL != hFontOld ){
		::SelectObject( hdc, hFontOld );
	}
	::EndPaint( hwnd, &ps );
}

/*!
 * CDarkMode::DeleteBrushes - ダークモード用のブラシを破棄する
 */
void CDarkMode::DeleteBrushes()
{
	for( int i = 0; i < _countof( s_ahBrush ); i++ ){
		if( NULL != s_ahBrush[i] ){
			::DeleteObject( s_ahBrush[i] );
			s_ahBrush[i] = NULL;
		}
	}
}
