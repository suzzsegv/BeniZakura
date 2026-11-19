#include "StdAfx.h"
#include "CRecentGrepFile.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFile::CRecentGrepFile()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFiles.dataPtr(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFiles._GetSizeRef(),
		NULL,
		MAX_GREPFILE,
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
const WCHAR* CRecentGrepFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentGrepFile::DataToReceiveType( LPCWSTR* dst, const CGrepFileString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFile::TextToDataType( CGrepFileString* dst, LPCWSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFile::CompareItem( const CGrepFileString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentGrepFile::CopyItem( CGrepFileString* dst, LPCWSTR src ) const
{
	//	バッファに収まらない場合は切り詰める (バッファオーバーフロー防止)
	wcsncpy_s( dst->GetBufferPointer(), dst->GetBufferCount(), src, _TRUNCATE );
}
