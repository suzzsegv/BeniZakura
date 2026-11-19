#include "StdAfx.h"
#include <string.h>
#include "CRecentGrepFolder.h"
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFolder::CRecentGrepFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFolders._GetSizeRef(),
		NULL,
		MAX_GREPFOLDER,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentGrepFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentGrepFolder::DataToReceiveType( LPCWSTR* dst, const CGrepFolderString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFolder::TextToDataType( CGrepFolderString* dst, LPCWSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFolder::CompareItem( const CGrepFolderString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentGrepFolder::CopyItem( CGrepFolderString* dst, LPCWSTR src ) const
{
	//	バッファに収まらない場合は切り詰める (バッファオーバーフロー防止)
	wcsncpy_s( dst->GetBufferPointer(), dst->GetBufferCount(), src, _TRUNCATE );
}
