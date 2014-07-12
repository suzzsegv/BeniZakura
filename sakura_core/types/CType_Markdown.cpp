/*!	@file	CType_Markdown.cpp
	@brief	タイプ別設定 Markdown デフォルト値設定クラス

	@author	Suzuki Satoshi
*/
/*
	Copyright (C) 2014, Suzuki Satoshi

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

#include "StdAfx.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"


/*!
 *	Markdown タイプ別設定のデフォルト値を設定する
 *
 *	@return なし
 */
void CType_Markdown::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("Markdown") );
	_tcscpy( pType->m_szTypeExts, _T("md") );

	//設定
	pType->m_nMaxLineKetas = CLayoutInt(120);					/* 折り返し桁数 */
	pType->m_eDefaultOutline = OUTLINE_HTML;					/* アウトライン解析方法 */

	//正規表現キーワード
	wchar_t* pKeyword = pType->m_RegexKeywordList;

	pType->m_bUseRegexKeyword = true;

	int keywordPos = 0;
	pType->m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_KEYWORD1;
	wcscpyn( &pKeyword[keywordPos],
		L"/\\s*# .*/k",
		_countof(pType->m_RegexKeywordList) - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_KEYWORD1;
	wcscpyn( &pKeyword[keywordPos],
		L"/\\s*## .*/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[2].m_nColorIndex = COLORIDX_KEYWORD1;
	wcscpyn( &pKeyword[keywordPos],
		L"/\\s*### .*/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[3].m_nColorIndex = COLORIDX_KEYWORD3;
	wcscpyn( &pKeyword[keywordPos],
		L"/ \\*\\*.*\\*\\* /k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[4].m_nColorIndex = COLORIDX_KEYWORD3;
	wcscpyn( &pKeyword[keywordPos],
		L"/^\\s*(\\d+\\. |\\* |\\+ |- )/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[5].m_nColorIndex = COLORIDX_BRACKET_PAIR;
	wcscpyn( &pKeyword[keywordPos],
		L"/  \\Z/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1 );
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pKeyword[keywordPos] = L'\0';
}

