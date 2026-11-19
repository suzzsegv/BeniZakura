#include "StdAfx.h"
#include "CRecentCmd.h"
#include "config/maxdata.h"
#include "env/DLLSHAREDATA.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCmd::CRecentCmd()
{
	Create(
		GetShareData()->m_sHistory.m_aCommands.dataPtr(),
		&GetShareData()->m_sHistory.m_aCommands._GetSizeRef(),
		NULL,
		MAX_CMDARR,
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
const WCHAR* CRecentCmd::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentCmd::DataToReceiveType( LPCWSTR* dst, const CCmdString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentCmd::TextToDataType( CCmdString* dst, LPCWSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentCmd::CompareItem( const CCmdString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentCmd::CopyItem( CCmdString* dst, LPCWSTR src ) const
{
	//	バッファに収まらない場合は切り詰める (バッファオーバーフロー防止)
	wcsncpy_s( dst->GetBufferPointer(), dst->GetBufferCount(), src, _TRUNCATE );
}
