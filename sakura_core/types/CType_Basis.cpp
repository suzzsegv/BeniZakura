#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CDocOutline.h"
#include "view/colors/EColorIndexType.h"

void CType_Basis::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, L"基本" );
	wcscpy( pType->m_szTypeExts, L"" );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(MAXLINEKETAS);			// 折り返し桁数
	pType->m_eDefaultOutline = OUTLINE_TEXT;					// アウトライン解析方法
}

/*!
 * CType_Basis::InitColorDispImp - 基本の「色分け/表示」の初期値を設定する
 *
 * InitTypeConfigImp から移動した (カラーテーマの切り替えで、タイプの初期値を求めるため)。
 */
void CType_Basis::InitColorDispImp(
	ColorInfo*	pColorInfoArr	//!< [i/o] 色設定 (COLORIDX_LAST 個)
)
{
	pColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	// シングルクォーテーション文字列を色分け表示しない	//Oct. 17, 2000 JEPRO
	pColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	// ダブルクォーテーション文字列を色分け表示しない	//Sept. 4, 2000 JEPRO
}
