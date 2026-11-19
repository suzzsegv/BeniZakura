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
#ifndef SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_
#define SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_

#include <string>
#include "util/string_ex.h"

#define m_delete2(p) { if(p){ delete[] p; p=0; } }

class CMyString{
public:
	//コンストラクタ・デストラクタ
	CMyString(WCHAR wc)								: m_wstr(1,wc),          m_str_cache(NULL) { }
	CMyString(const WCHAR* szData=L"")				: m_wstr(szData),        m_str_cache(NULL) { }
	CMyString(const WCHAR* pData, size_t nLength)	: m_wstr(pData,nLength), m_str_cache(NULL) { }
	CMyString(const ACHAR* szData)					: m_wstr(L""), m_str_cache(NULL) { set(szData); }
	CMyString(const ACHAR* pData, size_t nLength)	: m_wstr(L""), m_str_cache(NULL) { set(pData,nLength); }
	CMyString(ACHAR wc)								: m_wstr(L""), m_str_cache(NULL) { ACHAR buf[2]={wc,0}; set(buf); }
	CMyString(const CMyString& rhs) : m_wstr(rhs.c_wstr()), m_str_cache(NULL) { }
	~CMyString();

	//演算子
	operator const wchar_t* () const{ return c_wstr(); }
	operator const char* () const{ return c_astr(); }
	CMyString& operator = (const CMyString& rhs){ set(rhs); return *this; }

	//設定
	void set(const wchar_t* wszData){ m_wstr=wszData; m_delete2(m_str_cache); }
	void set(const wchar_t* wszData, int nLength){ m_wstr.assign(wszData, nLength); m_delete2(m_str_cache); }
	void set(const char* szData);
	void set(const char* szData, int nLength);
	void set(const CMyString& cszData){ set(cszData.c_wstr()); }

	//取得
	const wchar_t* c_wstr() const{ return m_wstr.c_str(); }
	const char* c_astr() const;
	int wlength() const{ return wcslen(c_wstr()); }
	int alength() const{ return strlen(c_astr()); }


private:
	std::wstring m_wstr;
	mutable char* m_str_cache; //c_str用キャッシュ。m_wstrが変更(set)されたらこれを解放し、NULLにしておくのがルール。
};

#define astring string



//共通マクロ
#include "util/StaticType.h"

//共通型
typedef StaticString<WCHAR,_MAX_PATH> SFilePath;
class CFilePath : public StaticString<WCHAR,_MAX_PATH>{
private:
	typedef StaticString<WCHAR,_MAX_PATH> Super;
public:
	CFilePath() : Super() { }
	CFilePath(const WCHAR* rhs) : Super(rhs) { }

	bool IsValidPath() const{ return At(0)!=L'\0'; }
	std::wstring GetDirPath() const
	{
		WCHAR	szDirPath[_MAX_PATH];
		WCHAR	szDrive[_MAX_DRIVE];
		WCHAR	szDir[_MAX_DIR];
		_wsplitpath( this->c_str(), szDrive, szDir, NULL, NULL );
		wcscpy( szDirPath, szDrive);
		wcscat( szDirPath, szDir );
		return szDirPath;
	}
	//拡張子を取得する
	LPCWSTR GetExt( bool bWithoutDot = false ) const
	{
		const WCHAR* head = c_str();
		const WCHAR* p = auto_strchr(head,L'\0') - 1;
		while(p>=head){
			if(*p==L'.')break;
			if(*p==L'\\')break;
			if(*p==L'/')break;
			p--;
		}
		if(p>=head && *p==L'.'){
			return bWithoutDot ? p+1 : p;	//bWithoutDot==trueならドットなしを返す
		}else{
			return auto_strchr(head,L'\0');
		}
	}
};


//$$ 仮
class CCommandLineString{
public:
	CCommandLineString()
	{
		m_szCmdLine[0] = L'\0';
		m_pHead = m_szCmdLine;
	}
	void AppendF(const WCHAR* szFormat, ...)
	{
		va_list v;
		va_start(v,szFormat);
		m_pHead+=auto_vsprintf_s(m_pHead,_countof(m_szCmdLine)-(m_pHead-m_szCmdLine),szFormat,v);
		va_end(v);
	}
	const WCHAR* c_str() const
	{
		return m_szCmdLine;
	}
	size_t size() const
	{
		return m_pHead - m_szCmdLine;
	}
	size_t max_size() const
	{
		return _countof(m_szCmdLine) - 1;
	}
private:
	WCHAR	m_szCmdLine[1024];
	WCHAR*	m_pHead;
};

#endif /* SAKURA_CMYSTRING_43EB58CF_8782_43AC_AC82_A22DFC99E063_H_ */
/*[EOF]*/
