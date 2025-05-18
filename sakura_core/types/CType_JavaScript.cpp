/*! @file CType_JavaScript.cpp
 *
 * @author Suzuki Satoshi
 */
/*
 *	Copyright (C) 2025 Suzuki Satoshi
 *
 *	This software is provided 'as-is', without any express or implied
 *	warranty. In no event will the authors be held liable for any damages
 *	arising from the use of this software.
 *
 *	Permission is granted to anyone to use this software for any purpose, 
 *	including commercial applications, and to alter it and redistribute it 
 *	freely, subject to the following restrictions:
 *
 *		1. The origin of this software must not be misrepresented;
 *		   you must not claim that you wrote the original software.
 *		   If you use this software in a product, an acknowledgment
 *		   in the product documentation would be appreciated but is
 *		   not required.
 *
 *		2. Altered source versions must be plainly marked as such, 
 *		   and must not be misrepresented as being the original software.
 *
 *		3. This notice may not be removed or altered from any source
 *		   distribution.
 */

#include "StdAfx.h"
#include "types/CType.h"
#include "view/Colors/EColorIndexType.h"

/*!
 * JavaScript キーワード 1
 */
static const wchar_t* g_defaultKeywordSetJavaScript1[] = {
	L"!",
	L"!=",
	L"&&",
	L"||",
	L"()",
	L"**",
	L"++",
	L"+=",
	L"--",
	L"-=",
	L".",
	L"<<",
	L"<=",
	L"=",
	L"==",
	L"=>",
	L">=",
	L">>",
	L"?",
	L"break",
	L"case",
	L"catch",
	L"const",
	L"constructor",
	L"continue",
	L"debugger",
	L"default",
	L"delete",
	L"do",
	L"each",
	L"else",
	L"export",
	L"extends",
	L"finally",
	L"for",
	L"function",
	L"function*",
	L"if",
	L"import",
	L"in",
	L"instanceof",
	L"new",
	L"of",
	L"return",
	L"static",
	L"super",
	L"switch",
	L"this",
	L"throw",
	L"try",
	L"typeof",
	L"void",
	L"while",
	L"with",
	L"yield",
	L"yield*",
};

/*!
 * JavaScript キーワード 2
 */
static const wchar_t* g_defaultKeywordSetJavaScript2[] = {
	L"!==",
	L"...",
	L"===",
	L">>>",
	L"Array",
	L"ArrayBuffer",
	L"Boolean",
	L"Date",
	L"JSON",
	L"Math",
	L"Number",
	L"RegExp",
	L"String",
	L"Symbol",
};

/*!
 * JavaScript Data Type キーワード
 */
static const wchar_t* g_defaultKeywordSetJavaScriptDataType[] = {
	L"class",
	L"let",
	L"var",
};

/*!
 * JavaScript Constant キーワード 1
 */
static const wchar_t* g_defaultKeywordSetJavaScriptConstant1[] = {
	L"false",
	L"true",
	L"null",
	L"undefined",
};

/*!
 * JavaScript Constant キーワード 2
 */
static const wchar_t* g_defaultKeywordSetJavaScriptConstant2[] = {
	L"EPSILON",
	L"NaN",
	L"Infinity",
	L"MAX_VALUE",
	L"MIN_VALUE",
	L"MAX_SAFE_INTEGER",
	L"MIN_SAFE_INTEGER",
	L"NEGATIVE_INFINITY",
	L"POSITIVE_INFINITY",
};

/*!
 * JavaScript Function キーワード
 */
static const wchar_t* g_defaultKeywordSetJavaScriptFunction[] = {
	L"console",
	L"document",
	L"element",
	L"event",
	L"navigator",
	L"screen",
	L"setTimeout",
	L"window",
	L"Coordinates",
	L"DeviceMotionEvent",
	L"DeviceOrientationEvent",
	L"EventTarget",
	L"Geolocation",
	L"Position",
	L"Storage",
	L"target",
	L"eval",
	L"uneval",
	L"isFinite",
	L"isNaN",
	L"parseFloat",
	L"parseInt",
	L"decodeURI",
	L"decodeURIComponent",
	L"encodeURI",
	L"encodeURIComponent",
	L"escape",
	L"unescape",
	L"alert",
	L"forEach",
};

void CType_JavaScript::InitTypeConfigImp(STypeConfig* pType)
{
	// 名前と拡張子
	_tcscpy(pType->m_szTypeName, _T("JavaScript"));
	_tcscpy(pType->m_szTypeExts, _T("js"));

	// 設定
	pType->m_cLineComment.CopyTo(0, L"//", -1); // 行コメントデリミタ
	pType->m_cBlockComments[0].SetBlockCommentRule(L"/*", L"*/"); // ブロックコメントデリミタ
	pType->m_eDefaultOutline = OUTLINE_CPP; // アウトライン解析方法
	pType->m_eSmartIndent = SMARTINDENT_NONE; // スマートインデント種別
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true; //半角数値を色分け表示
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true; //対括弧の強調

	pType->m_nKeyWordSetIdx[0] = AddDefaultKeywordSet(
		L"JavaScript Keyword 1",
		true,
		_countof(g_defaultKeywordSetJavaScript1),
		g_defaultKeywordSetJavaScript1);

	pType->m_nKeyWordSetIdx[1] = AddDefaultKeywordSet(
		L"JavaScript Keyword 2",
		true,
		_countof(g_defaultKeywordSetJavaScript2),
		g_defaultKeywordSetJavaScript2);

	pType->m_nKeyWordSetIdx[2] = AddDefaultKeywordSet(
		L"JavaScript Data Type",
		true,
		_countof(g_defaultKeywordSetJavaScriptDataType),
		g_defaultKeywordSetJavaScriptDataType);

	pType->m_nKeyWordSetIdx[3] = AddDefaultKeywordSet(
		L"JavaScript Constant 1",
		true,
		_countof(g_defaultKeywordSetJavaScriptConstant1),
		g_defaultKeywordSetJavaScriptConstant1);

	pType->m_nKeyWordSetIdx[4] = AddDefaultKeywordSet(
		L"JavaScript Constant 1",
		true,
		_countof(g_defaultKeywordSetJavaScriptConstant2),
		g_defaultKeywordSetJavaScriptConstant2);

	pType->m_nKeyWordSetIdx[5] = AddDefaultKeywordSet(
		L"JavaScript Function",
		true,
		_countof(g_defaultKeywordSetJavaScriptFunction),
		g_defaultKeywordSetJavaScriptFunction);

	//正規表現キーワード
	wchar_t* pKeyword = pType->m_RegexKeywordList;

	pType->m_bUseRegexKeyword = true;

	int keywordPos = 0;
	pType->m_RegexKeywordArr[0].m_nColorIndex = COLORIDX_KEYWORD1;
	wcscpyn(&pKeyword[keywordPos],
		L"/(if|for|while|switch|do|catch|return)[ \\t]*(?=\\()/k",
		_countof(pType->m_RegexKeywordList) - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[1].m_nColorIndex = COLORIDX_REGEX1;
	wcscpyn(&pKeyword[keywordPos],
		L"/[a-zA-Z_]+[0-9a-zA-Z_]*[ \\t]*(?=\\()/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[2].m_nColorIndex = COLORIDX_KEYWORD2;
	wcscpyn(&pKeyword[keywordPos],
		L"/\\/[^*][^ ]+\\/[dgimsuvy]*/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pType->m_RegexKeywordArr[3].m_nColorIndex = COLORIDX_SSTRING;
	wcscpyn(&pKeyword[keywordPos],
		L"/(?<!\\\\)`((?:[^`\\\\]|\\\\`)*?)`/k",
		_countof(pType->m_RegexKeywordList) - keywordPos - 1);
	keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;

	pKeyword[keywordPos] = L'\0';
}
