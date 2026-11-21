#include "StdAfx.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"

/* 設定ファイル */
//Nov. 9, 2000 JEPRO Windows標準のini, inf, cnfファイルとsakuraキーワード設定ファイル.kwd, 色設定ファイル.col も読めるようにする
void CType_Ini::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, L"設定ファイル" );
	wcscpy( pType->m_szTypeExts, L"ini,inf,cnf,kwd,col" );
	
	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );				/* 行コメントデリミタ */
	pType->m_cLineComment.CopyTo( 1, L";", -1 );				/* 行コメントデリミタ2 */
	pType->m_eDefaultOutline = OUTLINE_TEXT;					/* アウトライン解析方法 */
}

/*!
 * CType_Ini::InitColorDispImp - 設定ファイルの「色分け/表示」の初期値を設定する
 *
 * InitTypeConfigImp から移動した (カラーテーマの切り替えで、タイプの初期値を求めるため)。
 */
void CType_Ini::InitColorDispImp(
	ColorInfo*	pColorInfoArr	//!< [i/o] 色設定 (COLORIDX_LAST 個)
)
{
	pColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
}
