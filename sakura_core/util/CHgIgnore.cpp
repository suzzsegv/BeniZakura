/*!
 *	@file
 *	@brief	.hgignore (Mercurial) の除外指定の解釈
 *
 * .hgignore の各行を Mercurial と同じ規則で解釈し、正規表現に変換して照合する。
 * 正規表現は「パスの途中に一致すればよい」、glob は「どの階層からでも一致し、一致したフォルダの中も除外」、
 * rootglob は「ルートからのパスで一致」となる。クラスの説明と Mercurial との違いは CHgIgnore.h を参照。
 *
 * プリコンパイル済みヘッダー (StdAfx.h) は使用しない (単体テストからも単独でビルドするため)。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/15 新規作成
 */
/*!
 *	Copyright (C) 2026, Suzuki Satoshi
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

#include "CHgIgnore.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         補助関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! .hgignore の除外指定の種類 (警告の表示用)
static const wchar_t HGIGNORE_SOURCE[] = L".hgignore";

/*!
 * ReadUtf8TextFile - テキストファイルを UTF-8 として読み込む
 *
 * UTF-8 の BOM があれば取り除く。
 *
 *	@return true: 読み込んだ / false: ファイルを開けない
 */
static bool ReadUtf8TextFile(
	const std::wstring&	strFilePath,	//!< [in] ファイルのパス
	std::wstring*		pstrContent		//!< [out] ファイルの内容
)
{
	pstrContent->clear();
	const DWORD dwAttr = ::GetFileAttributesW( strFilePath.c_str() );
	if( dwAttr == INVALID_FILE_ATTRIBUTES || ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) ){
		return false;
	}
	FILE* fp = NULL;
	if( _wfopen_s( &fp, strFilePath.c_str(), L"rb" ) != 0 || fp == NULL ){
		return false;
	}
	std::string bytes;
	char buf[4096];
	size_t n;
	while( 0 < ( n = fread( buf, 1, sizeof(buf), fp ) ) ){
		bytes.append( buf, n );
	}
	fclose( fp );

	if( 3 <= bytes.size()
	 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF ){
		bytes.erase( 0, 3 );
	}
	if( !bytes.empty() ){
		const int nLen = ::MultiByteToWideChar( CP_UTF8, 0, bytes.data(), (int)bytes.size(), NULL, 0 );
		if( 0 < nLen ){
			pstrContent->resize( nLen );
			::MultiByteToWideChar( CP_UTF8, 0, bytes.data(), (int)bytes.size(), &(*pstrContent)[0], nLen );
		}
	}
	return true;
}

/*!
 * GetFullPath - パスを正規化したフルパスにする ("." や ".." を取り除き、'/' を '\\' にする)
 *
 *	@return フルパス (正規化できない場合は、'/' を '\\' にしただけのパス)
 */
static std::wstring GetFullPath(
	const std::wstring&	strPath		//!< [in] パス
)
{
	std::wstring strConv = strPath;
	for( size_t i = 0; i < strConv.size(); ++i ){
		if( strConv[i] == L'/' ){
			strConv[i] = L'\\';
		}
	}
	const DWORD nLen = ::GetFullPathNameW( strConv.c_str(), 0, NULL, NULL );
	if( nLen == 0 ){
		return strConv;
	}
	std::wstring strFull( nLen, L'\0' );
	const DWORD nCopied = ::GetFullPathNameW( strConv.c_str(), nLen, &strFull[0], NULL );
	if( nCopied == 0 || nLen <= nCopied ){
		return strConv;
	}
	strFull.resize( nCopied );
	return strFull;
}

/*!
 * GetDirOfFile - ファイルのパスから、ファイルのあるフォルダのパスを取り出す
 *
 *	@return フォルダのパス (末尾に '\\' を付けない)
 */
static std::wstring GetDirOfFile(
	const std::wstring&	strFilePath		//!< [in] ファイルのパス
)
{
	const std::wstring::size_type pos = strFilePath.find_last_of( L"\\/" );
	if( pos == std::wstring::npos ){
		return std::wstring();
	}
	return strFilePath.substr( 0, pos );
}

/*!
 * IsRegexSpecialChar - 正規表現 (ECMAScript) で特別な意味を持つ文字かを返す
 *
 *	@return true: 特別な意味を持つ (エスケープが必要) / false: 持たない
 */
static bool IsRegexSpecialChar(
	wchar_t	c	//!< [in] 文字
)
{
	return wcschr( L"^$\\.*+?()[]{}|", c ) != NULL && c != L'\0';
}

/*!
 * RemoveComment - .hgignore の行からコメントを取り除く
 *
 * 直前の '\\' が偶数個 (0 個を含む) の '#' からを取り除き、"\\#" を "#" に置き換える。
 *
 *	@return コメントを取り除いた行
 */
static std::wstring RemoveComment(
	const std::wstring&	strLine		//!< [in] 行
)
{
	std::wstring s = strLine;
	for( size_t i = 0; i < s.size(); ++i ){
		if( s[i] != L'#' ){
			continue;
		}
		size_t nBackslash = 0;
		while( nBackslash < i && s[i - 1 - nBackslash] == L'\\' ){
			++nBackslash;
		}
		if( nBackslash % 2 == 0 ){
			s.erase( i );
			break;
		}
	}
	std::wstring::size_type pos;
	while( ( pos = s.find( L"\\#" ) ) != std::wstring::npos ){
		s.replace( pos, 2, L"#" );
	}
	return s;
}

/*!
 * TrimRight - 行末の空白 (空白、タブ、改行) を取り除く
 *
 *	@return 行末の空白を取り除いた文字列
 */
static std::wstring TrimRight(
	const std::wstring&	s	//!< [in] 文字列
)
{
	std::wstring::size_type n = s.size();
	while( 0 < n && ( s[n - 1] == L' ' || s[n - 1] == L'\t' || s[n - 1] == L'\r' || s[n - 1] == L'\n' ) ){
		--n;
	}
	return s.substr( 0, n );
}

/*!
 * Trim - 前後の空白 (空白、タブ) を取り除く
 *
 *	@return 前後の空白を取り除いた文字列
 */
static std::wstring Trim(
	const std::wstring&	s	//!< [in] 文字列
)
{
	std::wstring::size_type b = 0;
	while( b < s.size() && ( s[b] == L' ' || s[b] == L'\t' ) ){
		++b;
	}
	return TrimRight( s.substr( b ) );
}

/*!
 * SyntaxFromName - "syntax:" 行の書式名から書式を返す
 *
 *	@return true: 書式名を解釈した / false: 不明な書式名
 */
static bool SyntaxFromName(
	const std::wstring&	strName,	//!< [in] 書式名
	EHgIgnoreSyntax*	peSyntax	//!< [out] 書式
)
{
	if( strName == L"re" || strName == L"regexp" ){
		*peSyntax = HGIGNORE_REGEXP;
	}else if( strName == L"glob" ){
		*peSyntax = HGIGNORE_GLOB;
	}else if( strName == L"rootglob" ){
		*peSyntax = HGIGNORE_ROOTGLOB;
	}else if( strName == L"include" ){
		*peSyntax = HGIGNORE_INCLUDE;
	}else if( strName == L"subinclude" ){
		*peSyntax = HGIGNORE_SUBINCLUDE;
	}else{
		return false;
	}
	return true;
}

/*!
 * SplitLineSyntax - 行頭の書式指定 (re: glob: など) を取り出す
 *
 * Mercurial と同じ順序 (re, regexp, glob, rootglob, include, subinclude) で、
 * 「relre:」「relglob:」などの別名と「名前:」の形を調べる。
 *
 *	@return true: 書式指定があった (pstrPattern は書式指定を除いた部分) / false: 書式指定がない
 */
static bool SplitLineSyntax(
	const std::wstring&	strLine,		//!< [in] 行
	EHgIgnoreSyntax*	peSyntax,		//!< [out] 書式
	std::wstring*		pstrPattern		//!< [out] 書式指定を除いたパターン
)
{
	struct SPrefix { const wchar_t* pszAlias; const wchar_t* pszName; EHgIgnoreSyntax eSyntax; };
	static const SPrefix prefixes[] = {
		{ L"relre:",	L"re:",			HGIGNORE_REGEXP },
		{ L"relre:",	L"regexp:",		HGIGNORE_REGEXP },
		{ L"relglob:",	L"glob:",		HGIGNORE_GLOB },
		{ L"rootglob:",	L"rootglob:",	HGIGNORE_ROOTGLOB },
		{ L"include:",	L"include:",	HGIGNORE_INCLUDE },
		{ L"subinclude:", L"subinclude:", HGIGNORE_SUBINCLUDE },
	};
	for( size_t i = 0; i < _countof(prefixes); ++i ){
		const wchar_t* candidates[] = { prefixes[i].pszAlias, prefixes[i].pszName };
		for( size_t j = 0; j < _countof(candidates); ++j ){
			const size_t nLen = wcslen( candidates[j] );
			if( strLine.compare( 0, nLen, candidates[j] ) == 0 ){
				*peSyntax = prefixes[i].eSyntax;
				*pstrPattern = strLine.substr( nLen );
				return true;
			}
		}
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CHgIgnoreFile                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CHgIgnoreFile::CHgIgnoreFile - コンストラクタ
 */
CHgIgnoreFile::CHgIgnoreFile()
: m_bMatchErrorWarned( false )
{
}

/*!
 * CHgIgnoreFile::GlobToRegex - glob を正規表現に変換する (前後の付加なし)
 *
 * Mercurial (match.py の _globre) と同じ規則で変換する。
 *   *  → [^/]*     ** → .*     **\/ → (?:.*\/)?     ? → .
 *   [...] → 文字クラス ([!...] は否定)     {a,b} → (?:a|b)     \x → x そのもの
 * それ以外の正規表現の記号はエスケープする。
 *
 *	@return 変換した正規表現
 */
std::wstring CHgIgnoreFile::GlobToRegex(
	const std::wstring&	strGlob		//!< [in] glob ('\\' は '/' に変換済みのもの)
)
{
	std::wstring res;
	const size_t n = strGlob.size();
	size_t i = 0;
	int nGroup = 0;
	while( i < n ){
		const wchar_t c = strGlob[i++];
		if( c == L'*' ){
			if( i < n && strGlob[i] == L'*' ){
				++i;
				if( i < n && strGlob[i] == L'/' ){
					++i;
					res += L"(?:.*/)?";
				}else{
					res += L".*";
				}
			}else{
				res += L"[^/]*";
			}
		}else if( c == L'?' ){
			res += L".";
		}else if( c == L'[' ){
			size_t j = i;
			if( j < n && ( strGlob[j] == L'!' || strGlob[j] == L']' ) ){
				++j;
			}
			while( j < n && strGlob[j] != L']' ){
				++j;
			}
			if( n <= j ){
				res += L"\\[";	// 閉じていない '[' は文字そのもの
			}else{
				std::wstring stuff = strGlob.substr( i, j - i );
				i = j + 1;
				std::wstring cls;
				size_t k = 0;
				if( !stuff.empty() && stuff[0] == L'!' ){
					cls += L'^';
					k = 1;
				}else if( !stuff.empty() && stuff[0] == L'^' ){
					cls += L"\\^";
					k = 1;
				}
				for( ; k < stuff.size(); ++k ){
					// ECMAScript では、文字クラスの中の ']' '[' '\\' をエスケープする
					if( stuff[k] == L']' || stuff[k] == L'[' || stuff[k] == L'\\' ){
						cls += L'\\';
					}
					cls += stuff[k];
				}
				res += L"[" + cls + L"]";
			}
		}else if( c == L'{' ){
			++nGroup;
			res += L"(?:";
		}else if( c == L'}' && 0 < nGroup ){
			res += L")";
			--nGroup;
		}else if( c == L',' && 0 < nGroup ){
			res += L"|";
		}else if( c == L'\\' ){
			if( i < n ){
				const wchar_t p = strGlob[i++];
				if( IsRegexSpecialChar( p ) ){
					res += L'\\';
				}
				res += p;
			}else{
				res += L"\\\\";
			}
		}else{
			if( IsRegexSpecialChar( c ) ){
				res += L'\\';
			}
			res += c;
		}
	}
	return res;
}

/*!
 * CHgIgnoreFile::PatternToRegex - .hgignore の 1 つのパターンを、照合用の正規表現に変換する
 *
 * Mercurial (match.py の _regex) と同じ規則で変換する。
 *   regexp:   '^' で始まればそのまま、そうでなければ先頭に ".*" を付ける (パスの途中に一致すればよい)
 *   glob:     "(?:|.*\/)" + glob の変換 + "(?:\/|$)" (どの階層からでも一致し、一致したフォルダの中も除外)
 *   rootglob: glob の変換 + "(?:\/|$)" (ルートからのパスで一致)
 * glob と rootglob の '\\' は、Windows 版の Mercurial と同じく '/' に変換する。
 *
 *	@return 変換した正規表現 (照合はパスの先頭からの一致で行う)。include / subinclude の場合は空文字列
 */
std::wstring CHgIgnoreFile::PatternToRegex(
	EHgIgnoreSyntax		eSyntax,		//!< [in] パターンの書式
	const std::wstring&	strPattern		//!< [in] パターン
)
{
	if( eSyntax == HGIGNORE_REGEXP ){
		if( !strPattern.empty() && strPattern[0] == L'^' ){
			return strPattern;
		}
		return L".*" + strPattern;
	}
	if( eSyntax != HGIGNORE_GLOB && eSyntax != HGIGNORE_ROOTGLOB ){
		return std::wstring();
	}

	std::wstring strGlob = strPattern;
	for( size_t i = 0; i < strGlob.size(); ++i ){
		if( strGlob[i] == L'\\' ){
			strGlob[i] = L'/';
		}
	}
	const std::wstring strGlobRe = GlobToRegex( strGlob );
	static const wchar_t GLOB_SUFFIX[] = L"(?:/|$)";
	if( eSyntax == HGIGNORE_ROOTGLOB ){
		return strGlobRe + GLOB_SUFFIX;
	}
	// "*XYZ" (よくある形) は、"(?:|.*/)[^/]*XYZ" と同じ意味の ".*XYZ" にする (Mercurial と同じ)
	static const wchar_t ANY_NAME[] = L"[^/]*";
	if( strGlobRe.compare( 0, wcslen( ANY_NAME ), ANY_NAME ) == 0 ){
		return L".*" + strGlobRe.substr( wcslen( ANY_NAME ) ) + GLOB_SUFFIX;
	}
	return L"(?:|.*/)" + strGlobRe + GLOB_SUFFIX;
}

/*!
 * CHgIgnoreFile::Clear - 読み込んだパターンと警告を消去し、リポジトリのルートを設定する
 */
void CHgIgnoreFile::Clear(
	const std::wstring&	strRootDir		//!< [in] リポジトリのルートのパス (末尾に '\\' を付けない)
)
{
	m_strRootDir = strRootDir;
	m_scopes.clear();
	m_includeStack.clear();
	m_warnings.clear();
	m_bMatchErrorWarned = false;

	SScope root;
	root.bCombined = false;
	m_scopes.push_back( root );
}

/*!
 * CHgIgnoreFile::LoadFromFile - リポジトリのルートの .hgignore を読み込む
 *
 * include: / subinclude: で指定されたファイルも読み込む。
 *
 *	@return true: 読み込んだ / false: .hgignore がない (除外しない)
 */
bool CHgIgnoreFile::LoadFromFile(
	const std::wstring&	strRootDir		//!< [in] リポジトリのルートのパス (末尾に '\\' を付けない)
)
{
	Clear( strRootDir );
	const std::wstring strFilePath = strRootDir + L"\\.hgignore";
	std::wstring strContent;
	if( !ReadUtf8TextFile( strFilePath, &strContent ) ){
		return false;
	}
	m_includeStack.push_back( GetFullPath( strFilePath ) );
	ParseContent( strContent, strFilePath, 0 );
	m_includeStack.pop_back();
	CombineScopes();
	return true;
}

/*!
 * CHgIgnoreFile::LoadFromString - リポジトリのルートの .hgignore の内容を読み込む
 *
 * 内容は「ルートの .hgignore」として扱う (include: / subinclude: のパスはルートからの相対パス)。
 */
void CHgIgnoreFile::LoadFromString(
	const std::wstring&	strRootDir,		//!< [in] リポジトリのルートのパス (末尾に '\\' を付けない)
	const std::wstring&	strContent		//!< [in] .hgignore の内容
)
{
	Clear( strRootDir );
	const std::wstring strFilePath = strRootDir + L"\\.hgignore";
	m_includeStack.push_back( GetFullPath( strFilePath ) );
	ParseContent( strContent, strFilePath, 0 );
	m_includeStack.pop_back();
	CombineScopes();
}

/*!
 * CHgIgnoreFile::ReadPatternFile - include: / subinclude: で指定されたファイルを読み込む
 *
 * 読み込み中のファイルをもう一度読み込もうとした場合 (循環) と、読み込めない場合は、警告を記録して読み飛ばす。
 */
void CHgIgnoreFile::ReadPatternFile(
	const std::wstring&	strFilePath,	//!< [in] 読み込むファイルのフルパス
	size_t				nScope,			//!< [in] パターンを追加する適用範囲
	const std::wstring&	strFromFile,	//!< [in] 読み込みを指定したファイル (警告の表示用)
	int					nFromLine,		//!< [in] 読み込みを指定した行 (警告の表示用)
	const std::wstring&	strFromPattern	//!< [in] 読み込みを指定したパターン (警告の表示用)
)
{
	for( size_t i = 0; i < m_includeStack.size(); ++i ){
		if( _wcsicmp( m_includeStack[i].c_str(), strFilePath.c_str() ) == 0 ){
			AddWarning( strFromFile, nFromLine, L"循環して読み込まれるファイル '" + strFromPattern + L"' を無視" );
			return;
		}
	}
	std::wstring strContent;
	if( !ReadUtf8TextFile( strFilePath, &strContent ) ){
		AddWarning( strFromFile, nFromLine, L"読み込めないファイル '" + strFromPattern + L"' を無視" );
		return;
	}
	m_includeStack.push_back( strFilePath );
	ParseContent( strContent, strFilePath, nScope );
	m_includeStack.pop_back();
}

/*!
 * CHgIgnoreFile::ParseContent - .hgignore の内容を 1 行ずつ解釈して、パターンを追加する
 *
 * 行の途中の '#' からはコメント、行末の空白は取り除く。"syntax:" 行は以降の行の書式を切り替える。
 */
void CHgIgnoreFile::ParseContent(
	const std::wstring&	strContent,		//!< [in] ファイルの内容
	const std::wstring&	strFilePath,	//!< [in] ファイルのパス (警告の表示と、include: の基準)
	size_t				nScope			//!< [in] パターンを追加する適用範囲
)
{
	EHgIgnoreSyntax eSyntax = HGIGNORE_REGEXP;
	int nLine = 0;
	std::wstring::size_type begin = 0;
	while( begin <= strContent.size() ){
		std::wstring::size_type end = strContent.find( L'\n', begin );
		if( end == std::wstring::npos ){
			end = strContent.size();
		}
		++nLine;
		std::wstring strLine = strContent.substr( begin, end - begin );
		begin = end + 1;

		strLine = TrimRight( RemoveComment( strLine ) );
		if( strLine.empty() ){
			continue;
		}

		static const wchar_t SYNTAX_PREFIX[] = L"syntax:";
		if( strLine.compare( 0, wcslen( SYNTAX_PREFIX ), SYNTAX_PREFIX ) == 0 ){
			const std::wstring strName = Trim( strLine.substr( wcslen( SYNTAX_PREFIX ) ) );
			if( !SyntaxFromName( strName, &eSyntax ) ){
				AddWarning( strFilePath, nLine, L"不明な書式 '" + strName + L"' を無視" );
			}
			continue;
		}

		EHgIgnoreSyntax eLineSyntax = eSyntax;
		std::wstring strPattern;
		if( !SplitLineSyntax( strLine, &eLineSyntax, &strPattern ) ){
			strPattern = strLine;
		}
		if( eLineSyntax == HGIGNORE_INCLUDE || eLineSyntax == HGIGNORE_SUBINCLUDE ){
			IncludeFile( eLineSyntax, strPattern, strFilePath, nLine, nScope );
		}else{
			AddPattern( eLineSyntax, strPattern, strFilePath, nLine, nScope );
		}
	}
}

/*!
 * CHgIgnoreFile::IncludeFile - include: / subinclude: の行を処理する
 *
 * ファイルのパスは、その行を書いたファイルのあるフォルダからの相対パスとする。
 * include: は読み込み元と同じ適用範囲に、subinclude: は読み込むファイルのあるフォルダを適用範囲として追加する。
 */
void CHgIgnoreFile::IncludeFile(
	EHgIgnoreSyntax		eSyntax,		//!< [in] HGIGNORE_INCLUDE または HGIGNORE_SUBINCLUDE
	const std::wstring&	strPattern,		//!< [in] 読み込むファイルのパス
	const std::wstring&	strFilePath,	//!< [in] その行を書いたファイルのパス
	int					nLine,			//!< [in] その行の行番号
	size_t				nScope			//!< [in] その行を書いたファイルの適用範囲
)
{
	const std::wstring strIncludePath = GetFullPath( GetDirOfFile( strFilePath ) + L"\\" + strPattern );
	if( eSyntax == HGIGNORE_INCLUDE ){
		ReadPatternFile( strIncludePath, nScope, strFilePath, nLine, strPattern );
		return;
	}

	// subinclude: 読み込むファイルのあるフォルダ (ルートからの相対パス) を適用範囲とする
	const std::wstring strDir = GetDirOfFile( strIncludePath );
	const std::wstring strRootFull = GetFullPath( m_strRootDir );
	std::wstring strBaseDir;
	if( _wcsicmp( strDir.c_str(), strRootFull.c_str() ) == 0 ){
		strBaseDir.clear();
	}else if( strRootFull.size() < strDir.size()
	 && _wcsnicmp( strDir.c_str(), strRootFull.c_str(), strRootFull.size() ) == 0
	 && strDir[strRootFull.size()] == L'\\' ){
		strBaseDir = strDir.substr( strRootFull.size() + 1 );
		for( size_t i = 0; i < strBaseDir.size(); ++i ){
			if( strBaseDir[i] == L'\\' ){
				strBaseDir[i] = L'/';
			}
		}
	}else{
		AddWarning( strFilePath, nLine, L"リポジトリの外のファイル '" + strPattern + L"' を無視" );
		return;
	}
	SScope scope;
	scope.strBaseDir = strBaseDir;
	scope.bCombined = false;
	m_scopes.push_back( scope );
	ReadPatternFile( strIncludePath, m_scopes.size() - 1, strFilePath, nLine, strPattern );
}

/*!
 * CHgIgnoreFile::AddPattern - パターンを正規表現に変換して、適用範囲に追加する
 *
 * 正規表現として解釈できないパターンは、警告を記録して追加しない。
 */
void CHgIgnoreFile::AddPattern(
	EHgIgnoreSyntax		eSyntax,		//!< [in] パターンの書式
	const std::wstring&	strPattern,		//!< [in] パターン
	const std::wstring&	strFilePath,	//!< [in] パターンを書いたファイルのパス (警告の表示用)
	int					nLine,			//!< [in] パターンを書いた行の行番号 (警告の表示用)
	size_t				nScope			//!< [in] パターンを追加する適用範囲
)
{
	const std::wstring strRegex = PatternToRegex( eSyntax, strPattern );
	try{
		std::wregex re( strRegex, std::regex_constants::ECMAScript | std::regex_constants::nosubs | std::regex_constants::optimize );
		m_scopes[nScope].regexSources.push_back( strRegex );
		m_scopes[nScope].regexes.push_back( re );
	}catch( const std::regex_error& ){
		AddWarning( strFilePath, nLine, L"正規表現として解釈できないため無視: " + strPattern );
	}
}

/*!
 * CHgIgnoreFile::CombineScopes - 適用範囲ごとに、パターンを 1 つの正規表現にまとめる
 *
 * 照合の回数を減らすため (Mercurial と同じ)。まとめられない場合は、パターンごとの正規表現で照合する。
 */
void CHgIgnoreFile::CombineScopes()
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		SScope& scope = m_scopes[i];
		scope.bCombined = false;
		if( scope.regexSources.empty() ){
			continue;
		}
		std::wstring strCombined;
		for( size_t j = 0; j < scope.regexSources.size(); ++j ){
			if( j ){
				strCombined += L"|";
			}
			strCombined += L"(?:" + scope.regexSources[j] + L")";
		}
		try{
			scope.combined.assign( strCombined, std::regex_constants::ECMAScript | std::regex_constants::nosubs | std::regex_constants::optimize );
			scope.bCombined = true;
		}catch( const std::regex_error& ){
			scope.bCombined = false;
		}
	}
}

/*!
 * CHgIgnoreFile::AddWarning - 警告を記録する
 */
void CHgIgnoreFile::AddWarning(
	const std::wstring&	strFilePath,	//!< [in] 警告の対象のファイルのパス
	int					nLine,			//!< [in] 行番号 (行に関係しない警告は 0)
	const std::wstring&	strMessage		//!< [in] 警告の内容
) const
{
	SVcsIgnoreWarning warning;
	warning.strSource = HGIGNORE_SOURCE;
	warning.strFilePath = strFilePath;
	warning.nLine = nLine;
	warning.strMessage = strMessage;
	m_warnings.push_back( warning );
}

/*!
 * CHgIgnoreFile::MatchScope - 適用範囲のパターンに、パスが先頭から一致するかを返す
 *
 * 照合中に例外が発生した場合は「一致しない」とし、警告を 1 回だけ記録する。
 *
 *	@return true: 一致する / false: 一致しない
 */
bool CHgIgnoreFile::MatchScope(
	const SScope&		scope,		//!< [in] 適用範囲
	const std::wstring&	strPath		//!< [in] 適用範囲のフォルダからの相対パス ('/' 区切り)
) const
{
	try{
		if( scope.bCombined ){
			return std::regex_search( strPath, scope.combined, std::regex_constants::match_continuous );
		}
		for( size_t i = 0; i < scope.regexes.size(); ++i ){
			if( std::regex_search( strPath, scope.regexes[i], std::regex_constants::match_continuous ) ){
				return true;
			}
		}
	}catch( const std::regex_error& ){
		if( !m_bMatchErrorWarned ){
			m_bMatchErrorWarned = true;
			AddWarning( m_strRootDir + L"\\.hgignore", 0, L"照合できないパスがあったため、一部のパターンを適用していない: " + strPath );
		}
	}
	return false;
}

/*!
 * CHgIgnoreFile::IsIgnored - パスが除外されるかを返す
 *
 * subinclude: の適用範囲は、そのフォルダの中のパスだけを、そのフォルダからの相対パスで照合する。
 * いずれかの適用範囲で一致すれば除外する (否定はない)。
 *
 *	@return true: 除外する / false: 除外しない
 */
bool CHgIgnoreFile::IsIgnored(
	const std::wstring&	strRelPath		//!< [in] リポジトリのルートからの相対パス ('/' 区切り)
) const
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		const SScope& scope = m_scopes[i];
		if( scope.regexSources.empty() ){
			continue;
		}
		if( scope.strBaseDir.empty() ){
			if( MatchScope( scope, strRelPath ) ){
				return true;
			}
			continue;
		}
		const std::wstring strPrefix = scope.strBaseDir + L"/";
		if( strRelPath.size() <= strPrefix.size() || strRelPath.compare( 0, strPrefix.size(), strPrefix ) != 0 ){
			continue;
		}
		if( MatchScope( scope, strRelPath.substr( strPrefix.size() ) ) ){
			return true;
		}
	}
	return false;
}

/*!
 * CHgIgnoreFile::TakeWarnings - 前回の取り出し以降に記録された警告を取り出す
 */
void CHgIgnoreFile::TakeWarnings(
	std::vector<SVcsIgnoreWarning>*	pWarnings	//!< [out] 警告の追加先
)
{
	pWarnings->insert( pWarnings->end(), m_warnings.begin(), m_warnings.end() );
	m_warnings.clear();
}

/*!
 * CHgIgnoreFile::GetPatternCount - 読み込んだパターンの数を返す (すべての適用範囲の合計)
 *
 *	@return パターンの数
 */
size_t CHgIgnoreFile::GetPatternCount() const
{
	size_t n = 0;
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		n += m_scopes[i].regexSources.size();
	}
	return n;
}
