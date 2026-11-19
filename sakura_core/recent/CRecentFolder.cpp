#include "StdAfx.h"
#include "CRecentFolder.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFolder::CRecentFolder()
{
	Create(
		&GetShareData()->m_sHistory.m_szOPENFOLDERArr[0],
		&GetShareData()->m_sHistory.m_nOPENFOLDERArrNum,
		GetShareData()->m_sHistory.m_bOPENFOLDERArrFavorite,
		MAX_OPENFOLDER,
		&(GetShareData()->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX)
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentFolder::DataToReceiveType( LPCWSTR* dst, const CPathString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentFolder::TextToDataType( CPathString* dst, LPCWSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentFolder::CompareItem( const CPathString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentFolder::CopyItem( CPathString* dst, LPCWSTR src ) const
{
	//	バッファに収まらない場合は切り詰める (バッファオーバーフロー防止)
	wcsncpy_s( dst->GetBufferPointer(), dst->GetBufferCount(), src, _TRUNCATE );
}
