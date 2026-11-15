/*!
 *	@file
 *	@brief	.gitignore の除外指定の解釈
 *
 * .gitignore の各行をパターンとして解析し、リポジトリのルートからの相対パス ('/' 区切り) と照合する。
 * 同じ .gitignore の中では後の行が、上位より下位のフォルダの .gitignore が優先される。
 * 英大文字と小文字は区別する。クラスの説明と git との違いは CGitIgnore.h を参照。
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

#include "CGitIgnore.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ワイルドカードの照合                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * MatchCharClass - 文字クラス [...] を照合する
 *
 *	@return true: 文字クラスを解析できた / false: 文字クラスが閉じていない (']' がない)
 */
static bool MatchCharClass(
	const wchar_t**	pp,			//!< [in,out] '[' の位置。照合後は ']' の次を指す
	wchar_t			c,			//!< [in] 照合する文字
	bool*			pMatched	//!< [out] 文字クラスに一致したか
)
{
	const wchar_t* p = *pp + 1;	// '[' の次
	bool bNegate = false;
	if( *p == L'!' || *p == L'^' ){
		bNegate = true;
		++p;
	}

	bool bMatched = false;
	bool bFirst = true;
	while( *p != L'\0' && ( bFirst || *p != L']' ) ){
		bFirst = false;

		wchar_t lo = *p;
		if( lo == L'\\' && p[1] != L'\0' ){
			++p;
			lo = *p;
		}
		++p;

		if( *p == L'-' && p[1] != L'\0' && p[1] != L']' ){
			// 範囲指定 (例: a-z)
			++p;
			wchar_t hi = *p;
			if( hi == L'\\' && p[1] != L'\0' ){
				++p;
				hi = *p;
			}
			++p;
			if( lo <= c && c <= hi ){
				bMatched = true;
			}
		}else if( c == lo ){
			bMatched = true;
		}
	}
	if( *p != L']' ){
		return false;
	}
	*pp = p + 1;
	*pMatched = ( bMatched != bNegate );
	return true;
}

/*!
 * WildMatchImpl - ワイルドカードを照合する (CGitIgnorePattern::WildMatch の本体)
 *
 *	@return true: 一致する / false: 一致しない
 */
static bool WildMatchImpl(
	const wchar_t*	p,			//!< [in] パターンの現在位置
	const wchar_t*	t,			//!< [in] 照合する文字列の現在位置
	const wchar_t*	pStart		//!< [in] パターンの先頭 ("**" が先頭にあるかの判定に使う)
)
{
	while( *p != L'\0' ){
		const wchar_t c = *p;

		if( c == L'\\' ){
			// エスケープ: 次の 1 文字をそのまま照合する
			++p;
			if( *p == L'\0' || *t != *p ){
				return false;
			}
			++p;
			++t;
			continue;
		}

		if( c == L'?' ){
			if( *t == L'\0' || *t == L'/' ){
				return false;
			}
			++p;
			++t;
			continue;
		}

		if( c == L'[' ){
			if( *t == L'\0' || *t == L'/' ){
				return false;
			}
			bool bMatched = false;
			if( !MatchCharClass( &p, *t, &bMatched ) ){
				return false;	// 閉じていない文字クラスには一致しない
			}
			if( !bMatched ){
				return false;
			}
			++t;
			continue;
		}

		if( c == L'*' ){
			if( p[1] == L'*' ){
				// "**": 先頭または '/' の直後にあり、末尾または '/' の直前にある場合のみ特別扱いする
				const wchar_t* pAfter = p + 2;
				const bool bAtStart = ( p == pStart || p[-1] == L'/' );
				const bool bAtEnd = ( *pAfter == L'\0' || *pAfter == L'/' );
				if( bAtStart && bAtEnd ){
					if( *pAfter == L'\0' ){
						return true;	// 末尾の "**" (または "/**"): 残りすべてに一致する
					}
					// "**/": 0 階層以上のディレクトリに一致する
					const wchar_t* pRest = pAfter + 1;
					const wchar_t* tt = t;
					for(;;){
						if( WildMatchImpl( pRest, tt, pStart ) ){
							return true;
						}
						const wchar_t* pSlash = wcschr( tt, L'/' );
						if( pSlash == NULL ){
							return false;
						}
						tt = pSlash + 1;
					}
				}
				// 特別扱いしない "**" は "*" と同じ
				while( *p == L'*' ){
					++p;
				}
			}else{
				++p;
			}

			// "*": '/' 以外の 0 文字以上に一致する
			if( *p == L'\0' ){
				return wcschr( t, L'/' ) == NULL;
			}
			for( const wchar_t* tt = t; ; ++tt ){
				if( WildMatchImpl( p, tt, pStart ) ){
					return true;
				}
				if( *tt == L'\0' || *tt == L'/' ){
					return false;
				}
			}
		}

		// 通常の文字 (英大文字と小文字を区別する)
		if( *t != c ){
			return false;
		}
		++p;
		++t;
	}
	return *t == L'\0';
}

/*!
 * CGitIgnorePattern::WildMatch - ワイルドカードを照合する
 *
 * *, ?, [...], **, \ に対応する。'*' と '?' と [...] は '/' に一致しない。
 * 英大文字と小文字は区別する。
 *
 *	@return true: 一致する / false: 一致しない
 */
bool CGitIgnorePattern::WildMatch(
	const wchar_t*	pattern,	//!< [in] パターン
	const wchar_t*	text		//!< [in] 照合する文字列
)
{
	return WildMatchImpl( pattern, text, pattern );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CGitIgnorePattern                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CGitIgnorePattern::CGitIgnorePattern - コンストラクタ
 */
CGitIgnorePattern::CGitIgnorePattern()
: m_bNegative( false )
, m_bDirectoryOnly( false )
, m_bAnchored( false )
{
}

/*!
 * CGitIgnorePattern::Parse - .gitignore の 1 行を解析する
 *
 * 否定 (先頭の '!')、ディレクトリのみ (末尾の '/')、位置の固定 ('/' を含む) を判定し、
 * それらを取り除いたパターン本体を保持する。行末の空白は取り除く (\ でエスケープした空白は残す)。
 *
 *	@return true: パターンとして解析した / false: 空行・コメント行
 */
bool CGitIgnorePattern::Parse(
	const std::wstring&	line	//!< [in] .gitignore の 1 行
)
{
	std::wstring s = line;

	// 行末の改行を取り除く
	while( !s.empty() && ( s[s.size() - 1] == L'\r' || s[s.size() - 1] == L'\n' ) ){
		s.erase( s.size() - 1 );
	}

	// 空行、コメント行
	if( s.empty() || s[0] == L'#' ){
		return false;
	}

	// 行末の空白を取り除く (バックスラッシュでエスケープされた空白は残す)
	while( !s.empty() && s[s.size() - 1] == L' ' ){
		if( 2 <= s.size() && s[s.size() - 2] == L'\\' ){
			break;
		}
		s.erase( s.size() - 1 );
	}
	if( s.empty() ){
		return false;
	}

	// 否定パターン
	m_bNegative = false;
	if( s[0] == L'!' ){
		m_bNegative = true;
		s.erase( 0, 1 );
	}

	// ディレクトリにのみ一致する (末尾の '/')
	m_bDirectoryOnly = false;
	if( !s.empty() && s[s.size() - 1] == L'/' ){
		m_bDirectoryOnly = true;
		s.erase( s.size() - 1 );
	}
	if( s.empty() ){
		return false;
	}

	// '/' を含む場合は、.gitignore のあるフォルダからの相対パスで照合する (先頭の '/' は取り除く)
	m_bAnchored = ( s.find( L'/' ) != std::wstring::npos );
	if( s[0] == L'/' ){
		s.erase( 0, 1 );
	}
	if( s.empty() ){
		return false;
	}

	m_pattern = s;
	return true;
}

/*!
 * CGitIgnorePattern::Match - パスがこのパターンに一致するかを返す
 *
 * 位置を固定したパターンはパス全体で、そうでないパターンはパスの最後の要素 (名前) で照合する。
 * 否定パターンでも、一致するかどうかだけを返す。
 *
 *	@return true: 一致する / false: 一致しない
 */
bool CGitIgnorePattern::Match(
	const std::wstring&	relPath,		//!< [in] .gitignore のあるフォルダからの相対パス ('/' 区切り)
	bool				isDirectory		//!< [in] フォルダか
) const
{
	if( m_bDirectoryOnly && !isDirectory ){
		return false;
	}
	if( m_bAnchored ){
		return WildMatch( m_pattern.c_str(), relPath.c_str() );
	}

	// '/' を含まないパターンは、ファイル名・ディレクトリ名 (パスの最後の要素) で照合する
	const std::wstring::size_type pos = relPath.rfind( L'/' );
	const wchar_t* pszName = ( pos == std::wstring::npos ) ? relPath.c_str() : relPath.c_str() + pos + 1;
	return WildMatch( m_pattern.c_str(), pszName );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CGitIgnoreFile                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CGitIgnoreFile::CGitIgnoreFile - コンストラクタ
 */
CGitIgnoreFile::CGitIgnoreFile(
	const std::wstring&	baseDir		//!< [in] .gitignore のあるフォルダの、リポジトリのルートからの相対パス (ルートは空文字列)
)
: m_baseDir( baseDir )
{
}

/*!
 * CGitIgnoreFile::LoadFromString - .gitignore の内容を読み込む
 *
 * 改行は CRLF / LF の両方を受け付ける。空行・コメント行は読み飛ばす。
 */
void CGitIgnoreFile::LoadFromString(
	const std::wstring&	content		//!< [in] .gitignore の内容 (複数行の文字列)
)
{
	m_patterns.clear();

	std::wstring::size_type begin = 0;
	while( begin <= content.size() ){
		std::wstring::size_type end = content.find( L'\n', begin );
		if( end == std::wstring::npos ){
			end = content.size();
		}
		CGitIgnorePattern pattern;
		if( pattern.Parse( content.substr( begin, end - begin ) ) ){
			m_patterns.push_back( pattern );
		}
		begin = end + 1;
	}
}

/*!
 * CGitIgnoreFile::LoadFromFile - .gitignore ファイルを読み込む
 *
 * ファイルの文字コードは UTF-8 とみなす (BOM があれば取り除く)。
 *
 *	@return true: 読み込んだ / false: ファイルを開けない
 */
bool CGitIgnoreFile::LoadFromFile(
	const wchar_t*	pszFilePath		//!< [in] .gitignore ファイルのパス
)
{
	FILE* fp = NULL;
	if( _wfopen_s( &fp, pszFilePath, L"rb" ) != 0 || fp == NULL ){
		return false;
	}
	std::string bytes;
	char buf[4096];
	size_t n;
	while( 0 < ( n = fread( buf, 1, sizeof(buf), fp ) ) ){
		bytes.append( buf, n );
	}
	fclose( fp );

	// UTF-8 の BOM を取り除く
	if( 3 <= bytes.size()
	 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF ){
		bytes.erase( 0, 3 );
	}

	std::wstring content;
	if( !bytes.empty() ){
		int nLen = ::MultiByteToWideChar( CP_UTF8, 0, bytes.data(), (int)bytes.size(), NULL, 0 );
		if( 0 < nLen ){
			content.resize( nLen );
			::MultiByteToWideChar( CP_UTF8, 0, bytes.data(), (int)bytes.size(), &content[0], nLen );
		}
	}
	LoadFromString( content );
	return true;
}

/*!
 * CGitIgnoreFile::Check - パスがこの .gitignore で除外されるかを判定する
 *
 * 同じ .gitignore の中では後の行が優先される。この .gitignore のあるフォルダの外のパスは判定しない。
 *
 *	@return GITIGNORE_IGNORED: 除外する / GITIGNORE_INCLUDED: 否定パターンにより含める /
 *	        GITIGNORE_NONE: どのパターンにも一致しない
 */
EGitIgnoreResult CGitIgnoreFile::Check(
	const std::wstring&	relPathFromRoot,	//!< [in] リポジトリのルートからの相対パス ('/' 区切り)
	bool				isDirectory			//!< [in] フォルダか
) const
{
	// 基準フォルダからの相対パスに変換する (基準フォルダの外のパスは対象外)
	std::wstring relPath;
	if( m_baseDir.empty() ){
		relPath = relPathFromRoot;
	}else{
		const std::wstring prefix = m_baseDir + L"/";
		if( relPathFromRoot.compare( 0, prefix.size(), prefix ) != 0 ){
			return GITIGNORE_NONE;
		}
		relPath = relPathFromRoot.substr( prefix.size() );
	}
	if( relPath.empty() ){
		return GITIGNORE_NONE;
	}

	// 後の行が優先されるため、後ろから照合し、最初に一致した行で判定する
	for( std::vector<CGitIgnorePattern>::const_reverse_iterator it = m_patterns.rbegin(); it != m_patterns.rend(); ++it ){
		if( it->Match( relPath, isDirectory ) ){
			return it->IsNegative() ? GITIGNORE_INCLUDED : GITIGNORE_IGNORED;
		}
	}
	return GITIGNORE_NONE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CGitIgnoreMatcher                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CGitIgnoreMatcher::CGitIgnoreMatcher - コンストラクタ
 */
CGitIgnoreMatcher::CGitIgnoreMatcher()
{
	m_repositories.push_back( std::vector<CGitIgnoreFile>() );
}

/*!
 * CGitIgnoreMatcher::PushFile - .gitignore を積む
 *
 * フォルダに入るときに、そのフォルダの .gitignore を積む。
 */
void CGitIgnoreMatcher::PushFile(
	const CGitIgnoreFile&	file	//!< [in] 積む .gitignore
)
{
	m_repositories.back().push_back( file );
}

/*!
 * CGitIgnoreMatcher::PopFile - 最後に積んだ .gitignore を降ろす
 *
 * 積んでいない場合は何もしない。
 */
void CGitIgnoreMatcher::PopFile()
{
	if( !m_repositories.back().empty() ){
		m_repositories.back().pop_back();
	}
}

/*!
 * CGitIgnoreMatcher::BeginRepository - 入れ子のリポジトリに入る
 *
 * 親リポジトリの .gitignore を退避し、空の状態から積み直す。
 */
void CGitIgnoreMatcher::BeginRepository()
{
	m_repositories.push_back( std::vector<CGitIgnoreFile>() );
}

/*!
 * CGitIgnoreMatcher::EndRepository - 入れ子のリポジトリから出る
 *
 * 退避した親リポジトリの .gitignore に戻す。入れ子のリポジトリに入っていない場合は何もしない。
 */
void CGitIgnoreMatcher::EndRepository()
{
	if( 1 < m_repositories.size() ){
		m_repositories.pop_back();
	}
}

/*!
 * CGitIgnoreMatcher::IsIgnored - パスが適用中の .gitignore で除外されるかを返す
 *
 * 下位のフォルダの .gitignore が優先される (同じ .gitignore の中では後の行が優先)。
 *
 *	@return true: 除外する / false: 除外しない
 */
bool CGitIgnoreMatcher::IsIgnored(
	const std::wstring&	relPathFromRoot,	//!< [in] 適用中のリポジトリのルートからの相対パス ('/' 区切り)
	bool				isDirectory			//!< [in] フォルダか
) const
{
	// 下位の .gitignore が優先されるため、下位 (後に積んだもの) から判定し、最初に判定できたもので決める
	const std::vector<CGitIgnoreFile>& files = m_repositories.back();
	for( std::vector<CGitIgnoreFile>::const_reverse_iterator it = files.rbegin(); it != files.rend(); ++it ){
		const EGitIgnoreResult result = it->Check( relPathFromRoot, isDirectory );
		if( result != GITIGNORE_NONE ){
			return result == GITIGNORE_IGNORED;
		}
	}
	return false;
}
