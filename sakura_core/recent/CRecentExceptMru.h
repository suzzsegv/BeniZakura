#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CMetaPath;

//! フォルダの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentExceptMRU : public CRecentImp<CMetaPath, LPCWSTR>{
public:
	//生成
	CRecentExceptMRU();

	//オーバーライド
	int				CompareItem( const CMetaPath* p1, LPCWSTR p2 ) const;
	void			CopyItem( CMetaPath* dst, LPCWSTR src ) const;
	const WCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCWSTR* dst, const CMetaPath* src ) const;
	bool			TextToDataType( CMetaPath* dst, LPCWSTR pszText ) const;
};
