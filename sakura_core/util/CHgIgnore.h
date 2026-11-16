/*!
 *	@file
 *	@brief	.hgignore (Mercurial) の除外指定の解釈
 *
 * Grep で .hgignore の除外指定に従うための照合処理。
 * 紅桜の共有データやウィンドウに依存せず、単体テストからも利用できるようにしている。
 * (プリコンパイル済みヘッダー StdAfx.h を使用しない)
 *
 * リポジトリのルートの .hgignore を読み込み、syntax: 行 (regexp / glob / rootglob)、行頭の書式指定、
 * include: / subinclude: を解釈する。パターンは Mercurial と同じ規則で正規表現に変換し、std::wregex で
 * ルートからの相対パス ('/' 区切り) の先頭からの一致を判定する。否定やディレクトリのみの指定はない
 * (glob / rootglob の末尾の '/' は、Mercurial と同じく正規化で取り除く)。
 *
 * Mercurial との主な違い:
 *   - .hgignore を UTF-8 として読む (日本語 Windows の Mercurial は CP932 として読む)
 *   - 正規表現は std::wregex (ECMAScript 形式)。解釈できないパターンは無視して警告する (Mercurial は中止する)
 *   - Mercurial で管理されているファイルでも、パターンに一致すれば除外とする
 *   - 次の Mercurial の不具合と思われる動作には合わせない
 *     - ':' のない "include" / "subinclude" で始まる行を、ルートからの glob として扱う
 *     - "syntax: include" / "syntax: subinclude" の以降の行を、ルートからの glob として扱う
 *     - 同じフォルダのファイルを 2 回 subinclude: すると、2 つ目以降のパターンを適用しない
 *     - include: で読み込んだファイルの中の subinclude: のパスを正しく解決しない
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

#ifndef SAKURA_CHGIGNORE_H_
#define SAKURA_CHGIGNORE_H_

#include <regex>
#include <string>
#include <vector>
#include "CVcsIgnore.h"	// SVcsIgnoreWarning

//! .hgignore の行の書式
enum EHgIgnoreSyntax {
	HGIGNORE_REGEXP,		//!< 正規表現 (regexp / re / relre)。パスの途中に一致すればよい
	HGIGNORE_GLOB,			//!< glob (glob / relglob)。どの階層からでも一致する
	HGIGNORE_ROOTGLOB,		//!< rootglob。ルートからのパスで一致する
	HGIGNORE_INCLUDE,		//!< include (パターンのファイルを読み込む)
	HGIGNORE_SUBINCLUDE,	//!< subinclude (パターンのファイルを、そのフォルダの中だけに適用する)
};

/*!
 * 1 つのリポジトリの .hgignore
 *
 * ルートの .hgignore と、そこから include: / subinclude: で読み込んだファイルのパターンを持つ。
 * パターンは適用範囲 (ルート、または subinclude: で読み込んだファイルのフォルダ) ごとにまとめる。
 */
class CHgIgnoreFile
{
public:
	CHgIgnoreFile();

	bool LoadFromFile( const std::wstring& strRootDir );
	void LoadFromString( const std::wstring& strRootDir, const std::wstring& strContent );

	bool IsIgnored( const std::wstring& strRelPath ) const;

	void TakeWarnings( std::vector<SVcsIgnoreWarning>* pWarnings );
	size_t GetPatternCount() const;

	static std::wstring PatternToRegex( EHgIgnoreSyntax eSyntax, const std::wstring& strPattern );
	static std::wstring GlobToRegex( const std::wstring& strGlob );
	static std::wstring NormalizeGlobPath( const std::wstring& strGlob );

private:
	//! 適用範囲ごとのパターン
	struct SScope
	{
		std::wstring				strBaseDir;		//!< 適用範囲のフォルダ (ルートからの相対パス、'/' 区切り。ルートは空文字列)
		std::vector<std::wstring>	regexSources;	//!< パターンを変換した正規表現
		std::vector<std::wregex>	regexes;		//!< パターンごとの正規表現 (まとめた正規表現を作れない場合に使う)
		std::wregex					combined;		//!< パターンをまとめた正規表現
		bool						bCombined;		//!< combined が使える
	};

	void Clear( const std::wstring& strRootDir );
	void ReadPatternFile( const std::wstring& strFilePath, size_t nScope, const std::wstring& strFromFile, int nFromLine, const std::wstring& strFromPattern );
	void ParseContent( const std::wstring& strContent, const std::wstring& strFilePath, size_t nScope );
	void IncludeFile( EHgIgnoreSyntax eSyntax, const std::wstring& strPattern, const std::wstring& strFilePath, int nLine, size_t nScope );
	void AddPattern( EHgIgnoreSyntax eSyntax, const std::wstring& strPattern, const std::wstring& strFilePath, int nLine, size_t nScope );
	void CombineScopes();
	void AddWarning( const std::wstring& strFilePath, int nLine, const std::wstring& strMessage ) const;
	bool MatchScope( const SScope& scope, const std::wstring& strPath ) const;

	std::wstring							m_strRootDir;		//!< リポジトリのルートのパス (末尾に '\\' を付けない)
	std::vector<SScope>						m_scopes;			//!< 適用範囲ごとのパターン
	std::vector<std::wstring>				m_includeStack;		//!< 読み込み中のファイル (循環の検出用)
	mutable std::vector<SVcsIgnoreWarning>	m_warnings;			//!< 記録した警告 (取り出していないもの)
	mutable bool							m_bMatchErrorWarned;	//!< 照合中の例外を警告した
};

#endif /* SAKURA_CHGIGNORE_H_ */
