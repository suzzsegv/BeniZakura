/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_
#define SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "doc/CColorTheme.h"	// ColorTheme, ColorInfo (2026.11.18 CColorTheme.h に移動)


//デフォルト色設定
void SetDefaultColorInfo(ColorInfo* pColorInfo, ColorTheme nIndex);



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           辞書                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//@@@ 2006.04.10 fon ADD-start
const int DICT_ABOUT_LEN = 50; /*!< 辞書の説明の最大長 -1 */
struct KeyHelpInfo {
	bool		m_bUse;						//!< 辞書を 使用する/しない
	TCHAR		m_szAbout[DICT_ABOUT_LEN];	//!< 辞書の説明(辞書ファイルの1行目から生成)
	SFilePath	m_szPath;					//!< ファイルパス
};
//@@@ 2006.04.10 fon ADD-end

#endif /* SAKURA_CDOCTYPESETTING_28058D99_2101_4488_A634_832BD50A2F3C9_H_ */
/*[EOF]*/
