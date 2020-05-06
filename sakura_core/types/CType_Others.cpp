#include "StdAfx.h"
#include "types/CType.h"

void CType_Other::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	auto_sprintf( pType->m_szTypeName, _T("設定%d"), pType->m_nIdx + 1 );
	_tcscpy( pType->m_szTypeExts, _T("") );

}
