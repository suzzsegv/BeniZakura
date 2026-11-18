// 2000.10.08 JEPRO  背景色を真っ白RGB(255,255,255)→(255,251,240)に変更(眩しさを押さえた)
// 2000.12.09 Jepro  note: color setting (詳細は CshareData.h を参照のこと)
// 2000.09.04 JEPRO  シングルクォーテーション文字列に色を割り当てるが色分け表示はしない
// 2000.10.17 JEPRO  色分け表示するように変更(最初のFALSE→TRUE)
// 2008.03.27 kobake 大整理
// 2026.11.18 プリセットの配色表とテーマの適用は doc/CColorTheme.cpp に移動

#include "StdAfx.h"
#include "CDocTypeSetting.h"


/*!
	SetDefaultColorInfo - 画面表示色に関する情報を指定テーマの初期値に設定する

	@param [out] ColorInfo* pColorInfo
	@param [in]  ColorTheme themeNum
	@retval なし
 */
void SetDefaultColorInfo(ColorInfo* pColorInfo, ColorTheme themeNum)
{
	SetPresetColorInfo( pColorInfo, themeNum );
}
