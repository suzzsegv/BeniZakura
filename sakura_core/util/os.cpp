#include "StdAfx.h"
#include "os.h"
#include "util/module.h"
#include <uxtheme.h>

/*!	自分が現在ビジュアルスタイル表示状態かどうかを示す
	Win32 API の IsAppThemed() はこれとは一致しない（IsAppThemed() と IsThemeActive() との差異は不明）

	@return ビジュアルスタイル表示状態(TRUE)／クラッシック表示状態(FALSE)

	@author ryoji
	@date 2006.06.17 ryoji 新規
*/
BOOL IsVisualStyle()
{
	// マニフェストで Comctl32.dll の Ver 6 を指定しているので、画面設定がビジュアルスタイル指定になっているかだけを調べる
	return ::IsThemeActive();
}



/*!	指定ウィンドウでビジュアルスタイルを使わないようにする

	@param[in] hWnd ウィンドウ

	@author ryoji
	@date 2006.06.23 ryoji 新規
*/
void PreventVisualStyle( HWND hWnd )
{
	::SetWindowTheme( hWnd, L"", L"" );
	return;
}




/*!	コモンコントロールを初期化する

	@author ryoji
	@date 2006.06.21 ryoji 新規
*/
void MyInitCommonControls()
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
	::InitCommonControlsEx( &icex );
}





/*!
	指定したウィンドウ／長方形領域／点／モニタに対応するモニタ作業領域を取得する

	モニタ作業領域：画面全体からシステムのタスクバーやアプリケーションのツールバーが占有する領域を除いた領域

	@param hWnd/prc/pt/hMon [in] 目的のウィンドウ／長方形領域／点／モニタ
	@param prcWork [out] モニタ作業領域
	@param prcMonitor [out] モニタ画面全体

	@retval true 対応するモニタはプライマリモニタ
	@retval false 対応するモニタは非プライマリモニタ

	@note 出力パラメータの prcWork や prcMonior に NULL を指定した場合、
	該当する領域情報は出力しない。呼び出し元は欲しいものだけを指定すればよい。
*/
//	From Here May 01, 2004 genta MutiMonitor
bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	// 2006.04.21 ryoji Windows API 形式の関数呼び出しに変更（スタブに PSDK の MultiMon.h を利用）
	HMONITOR hMon = ::MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}
//	To Here May 01, 2004 genta

//	From Here 2006.04.21 ryoji MutiMonitor
bool GetMonitorWorkRect(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromRect( prc, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(POINT pt, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
	return GetMonitorWorkRect( hMon, prcWork, prcMonitor );
}

bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	MONITORINFO mi;
	::ZeroMemory( &mi, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( hMon, &mi );
	if( NULL != prcWork )
		*prcWork = mi.rcWork;		// work area rectangle of the display monitor
	if( NULL != prcMonitor )
		*prcMonitor = mi.rcMonitor;	// display monitor rectangle
	return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
}
//	To Here 2006.04.21 ryoji MutiMonitor




/*!
	@brief レジストリから文字列を読み出す．
	
	@param Hive        [in]  HIVE
	@param Path        [in]  レジストリキーへのパス
	@param Item        [in]  レジストリアイテム名．NULLで標準のアイテム．
	@param Buffer      [out] 取得文字列を格納する場所
	@param BufferCount [in]  Bufferの指す領域のサイズ。文字単位。
	
	@retval true 値の取得に成功
	@retval false 値の取得に失敗
	
	@author 鬼
	@date 2002.09.10 genta CWSH.cppから移動
*/
bool ReadRegistry(HKEY Hive, const WCHAR* Path, const WCHAR* Item, WCHAR* Buffer, unsigned BufferCount)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		auto_memset(Buffer, 0, BufferCount);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = (BufferCount - 1) * sizeof(WCHAR); //※バイト単位！
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<LPBYTE>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      クリップボード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//SetClipboardTextA,SetClipboardTextT 実装用テンプレート
//2007.08.14 kobake UNICODE用に改造
//
/*! クリープボードにText形式でコピーする
	@param hwnd    [in] クリップボードのオーナー
	@param pszText [in] 設定するテキスト
	@param nLength [in] 有効なテキストの長さ。文字単位。
	
	@retval true コピー成功
	@retval false コピー失敗。場合によってはクリップボードに元の内容が残る
	@date 2004.02.17 Moca 各所のソースを統合
*/
template <class T>
bool SetClipboardTextImp( HWND hwnd, const T* pszText, int nLength )
{
	HGLOBAL	hgClip;
	T*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, (nLength + 1) * sizeof(T) );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (T*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	auto_memcpy( pszClip, pszText, nLength );
	pszClip[nLength] = 0;
	::GlobalUnlock( hgClip );
	if( !::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	if(sizeof(T)==sizeof(char)){
		::SetClipboardData( CF_OEMTEXT, hgClip );
	}
	else if(sizeof(T)==sizeof(wchar_t)){
		::SetClipboardData( CF_UNICODETEXT, hgClip );
	}
	else{
		assert(0); //※ここには来ない
	}
	::CloseClipboard();

	return true;
}

bool SetClipboardText( HWND hwnd, const ACHAR* pszText, int nLength )
{
	return SetClipboardTextImp<ACHAR>(hwnd,pszText,nLength);
}

bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength )
{
	return SetClipboardTextImp<WCHAR>(hwnd,pszText,nLength);
}

/*
	@date 2006.01.16 Moca 他のTYMEDが利用可能でも、取得できるように変更。
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSEでも受け入れてしまうバグを修正(ファイルのドロップ等)
	return S_OK == pDataObject->QueryGetData( &fe );
}

HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1からTYMED_HGLOBALに変更。
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDEDマクロではS_FALSEのとき困るので、S_OKに変更
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy_raw( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        便利クラス                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。

CCurrentDirectoryBackupPoint::CCurrentDirectoryBackupPoint()
{
	int n = ::GetCurrentDirectory(_countof(m_szCurDir),m_szCurDir);
	if(n>0 && n<_countof(m_szCurDir)){
		//ok
	}
	else{
		//ng
		m_szCurDir[0] = L'\0';
	}
}

CCurrentDirectoryBackupPoint::~CCurrentDirectoryBackupPoint()
{
	if(m_szCurDir[0]){
		::SetCurrentDirectory(m_szCurDir);
	}
}



