/*!
 *	@file
 *	@brief	.gitignore の除外指定の解釈
 *
 * Grep で .gitignore の除外指定に従うための照合処理。
 * 紅桜の共有データやウィンドウに依存せず、単体テストからも利用できるようにしている。
 * (プリコンパイル済みヘッダー StdAfx.h を使用しない)
 *
 * .gitignore の書式 (*, ?, [...], **, \ のエスケープ, ! の否定, 末尾の / , 先頭・途中の / による位置の固定)
 * を解釈する。同じ .gitignore の中では後の行が、上位より下位のフォルダの .gitignore が優先される。
 * 入れ子のリポジトリ (サブモジュールなど) の中には、親リポジトリの .gitignore を適用しない。
 *
 * git との主な違い:
 *   - 英大文字と小文字を区別して照合する
 *   - .git/info/exclude、core.excludesFile は扱わない (呼び出し側で .gitignore のみを読み込む)
 *   - Git で管理されているファイルでも、パターンに一致すれば除外とする
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

#ifndef SAKURA_CGITIGNORE_H_
#define SAKURA_CGITIGNORE_H_

#include <string>
#include <vector>

/*!
 * .gitignore の 1 行分のパターン
 *
 * パスは、その .gitignore のあるフォルダからの相対パスで、区切り文字は '/'、
 * 先頭に '/' を付けない形 (例: "src/main.cpp") で扱う。
 */
class CGitIgnorePattern
{
public:
	CGitIgnorePattern();

	//! .gitignore の 1 行を解析する。空行・コメント行の場合は false を返す
	bool Parse( const std::wstring& line );

	//! パスがこのパターンに一致するか (否定パターンでも、一致するかどうかだけを返す)
	bool Match( const std::wstring& relPath, bool isDirectory ) const;

	bool IsNegative() const { return m_bNegative; }			//!< 否定パターン (先頭の '!')
	bool IsDirectoryOnly() const { return m_bDirectoryOnly; }	//!< ディレクトリにのみ一致する (末尾の '/')
	bool IsAnchored() const { return m_bAnchored; }			//!< パスで照合する ('/' を含む)。偽ならファイル名・ディレクトリ名で照合する
	const std::wstring& GetPattern() const { return m_pattern; }

	//! ワイルドカードの照合 (*, ?, [...], **, \ に対応。'*' と '?' は '/' に一致しない)
	static bool WildMatch( const wchar_t* pattern, const wchar_t* text );

private:
	std::wstring	m_pattern;
	bool			m_bNegative;
	bool			m_bDirectoryOnly;
	bool			m_bAnchored;
};

//! .gitignore による判定結果
enum EGitIgnoreResult {
	GITIGNORE_NONE,		//!< どのパターンにも一致しない
	GITIGNORE_IGNORED,	//!< 除外する
	GITIGNORE_INCLUDED,	//!< 否定パターンにより含める
};

/*!
 * 1 つの .gitignore ファイル
 *
 * 基準フォルダ (リポジトリのルートからの相対パス。ルートの場合は空文字列) と、パターンの一覧を持つ。
 */
class CGitIgnoreFile
{
public:
	explicit CGitIgnoreFile( const std::wstring& baseDir = std::wstring() );

	//! .gitignore の内容 (複数行の文字列) を読み込む
	void LoadFromString( const std::wstring& content );

	//! .gitignore ファイルを読み込む (UTF-8 とみなす。BOM は取り除く)。ファイルを開けない場合は false
	bool LoadFromFile( const wchar_t* pszFilePath );

	//! リポジトリのルートからの相対パスについて判定する (後の行が優先)
	EGitIgnoreResult Check( const std::wstring& relPathFromRoot, bool isDirectory ) const;

	const std::wstring& GetBaseDir() const { return m_baseDir; }
	size_t GetPatternCount() const { return m_patterns.size(); }

private:
	std::wstring					m_baseDir;
	std::vector<CGitIgnorePattern>	m_patterns;
};

/*!
 * 適用中の .gitignore の管理
 *
 * フォルダに入るときに .gitignore を積み (PushFile)、出るときに降ろす (PopFile)。
 * 入れ子のリポジトリ (サブモジュールなど) に入るときは BeginRepository() で
 * 親リポジトリの .gitignore を退避し、出るときに EndRepository() で戻す。
 * パスは、そのとき適用中のリポジトリのルートからの相対パスで指定する。
 */
class CGitIgnoreMatcher
{
public:
	CGitIgnoreMatcher();

	void PushFile( const CGitIgnoreFile& file );	//!< .gitignore を積む
	void PopFile();									//!< 最後に積んだ .gitignore を降ろす

	void BeginRepository();	//!< 入れ子のリポジトリに入る (親リポジトリの .gitignore を退避する)
	void EndRepository();	//!< 入れ子のリポジトリから出る (親リポジトリの .gitignore に戻す)

	//! 除外するか (下位の .gitignore が優先、同じファイル内では後の行が優先)
	bool IsIgnored( const std::wstring& relPathFromRoot, bool isDirectory ) const;

	size_t GetFileCount() const { return m_repositories.back().size(); }	//!< 適用中のリポジトリの .gitignore の数

private:
	std::vector< std::vector<CGitIgnoreFile> >	m_repositories;	//!< リポジトリごとの .gitignore のスタック (末尾が適用中)
};

#endif /* SAKURA_CGITIGNORE_H_ */
