#include "StdAfx.h"
#include "module.h"
#include "util/os.h"
#include "util/file.h"
#include <Shlwapi.h>	// 2006.06.17 ryoji

/*! 
	カレントディレクトリを実行ファイルの場所に移動
	@date 2010.08.28 Moca 新規作成
*/
void ChangeCurrentDirectoryToExeDir()
{
	WCHAR szExeDir[_MAX_PATH];
	szExeDir[0] = L'\0';
	GetExedir( szExeDir, NULL );
	if( szExeDir[0] ){
		::SetCurrentDirectory( szExeDir );
	}else{
		// 移動できないときはSYSTEM32(9xではSYSTEM)に移動
		szExeDir[0] = L'\0';
		int n = ::GetSystemDirectory( szExeDir, _MAX_PATH );
		if( n && n < _MAX_PATH ){
			::SetCurrentDirectory( szExeDir );
		}
	}
}

/*! 
	@date 2010.08.28 Moca 新規作成
*/
HMODULE LoadLibraryExedir(LPCWSTR pszDll)
{
	CCurrentDirectoryBackupPoint dirBack;
	// DLL インジェクション対策としてEXEのフォルダに移動する
	ChangeCurrentDirectoryToExeDir();
	return ::LoadLibrary( pszDll );
}

/*!
	@brief アプリケーションアイコンの取得
	
	アイコンファイルが存在する場合はそこから，無い場合は
	リソースファイルから取得する
	
	@param hInst [in] Instance Handle
	@param nResource [in] デフォルトアイコン用Resource ID
	@param szFile [in] アイコンファイル名
	@param bSmall [in] true: small icon (16x16) / false: large icon (32x32)
	
	@return アイコンハンドル．失敗した場合はNULL．
	
	@date 2002.12.02 genta 新規作成
	@data 2007.05.20 ryoji iniファイルパスを優先
	@author genta
*/
HICON GetAppIcon( HINSTANCE hInst, int nResource, const WCHAR* szFile, bool bSmall )
{
	// サイズの設定
	int size = ( bSmall ? 16 : 32 );

	WCHAR szPath[_MAX_PATH];
	HICON hIcon;

	// ファイルからの読み込みをまず試みる
	GetInidirOrExedir( szPath, szFile );

	hIcon = (HICON)::LoadImage(
		NULL,
		szPath,
		IMAGE_ICON,
		size,
		size,
		LR_SHARED | LR_LOADFROMFILE
	);
	if( hIcon != NULL ){
		return hIcon;
	}

	//	ファイルからの読み込みに失敗したらリソースから取得
	hIcon = (HICON)::LoadImage(
		hInst,
		MAKEINTRESOURCE(nResource),
		IMAGE_ICON,
		size,
		size,
		LR_SHARED
	);
	
	return hIcon;
}






struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	WCHAR	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/*! リソースから製品バージョンの取得
	@date 2004.05.13 Moca 一度取得したらキャッシュする
*/
void GetAppVersionInfo(
	HINSTANCE	hInstance,
	int			nVersionResourceID,
	DWORD*		pdwProductVersionMS,
	DWORD*		pdwProductVersionLS
)
{
	HRSRC					hRSRC;
	HGLOBAL					hgRSRC;
	VS_VERSION_INFO_HEAD*	pVVIH;
	/* リソースから製品バージョンの取得 */
	*pdwProductVersionMS = 0;
	*pdwProductVersionLS = 0;
	static bool bLoad = false;
	static DWORD dwVersionMS = 0;
	static DWORD dwVersionLS = 0;
	if( hInstance == NULL && bLoad ){
		*pdwProductVersionMS = dwVersionMS;
		*pdwProductVersionLS = dwVersionLS;
		return;
	}
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
		if( hInstance == NULL ){
			dwVersionMS = pVVIH->Value.dwProductVersionMS;
			dwVersionLS = pVVIH->Value.dwProductVersionLS;
			bLoad = true;
		}
	}
	return;

}
