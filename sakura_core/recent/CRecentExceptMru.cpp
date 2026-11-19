#include "stdafx.h"
#include "CRecentExceptMRU.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExceptMRU::CRecentExceptMRU()
{
	Create(
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr(),
		&GetShareData()->m_sHistory.m_aExceptMRU._GetSizeRef(),
		NULL,
		MAX_MRU,
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
const WCHAR* CRecentExceptMRU::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExceptMRU::DataToReceiveType( LPCWSTR* dst, const CMetaPath* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExceptMRU::TextToDataType( CMetaPath* dst, LPCWSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentExceptMRU::CompareItem( const CMetaPath* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentExceptMRU::CopyItem( CMetaPath* dst, LPCWSTR src ) const
{
	//	バッファに収まらない場合は切り詰める (バッファオーバーフロー防止)
	wcsncpy_s( dst->GetBufferPointer(), dst->GetBufferCount(), src, _TRUNCATE );
}
