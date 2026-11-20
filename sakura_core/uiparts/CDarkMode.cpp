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
