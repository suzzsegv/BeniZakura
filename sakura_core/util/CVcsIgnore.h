/*!
 *	@file
 *	@brief	VCS (Git / Mercurial) の除外指定の適用状態
 *
 * Grep でフォルダをたどりながら、VCS の除外指定 (.gitignore / .hgignore) を適用するためのクラス。
 * VCS ごとの適用状態 (CVcsIgnoreScope の派生クラス) は互いに独立しており、
 * どれか 1 つでも除外していれば除外する (CVcsIgnore)。
 * 紅桜の共有データやウィンドウに依存せず、単体テストからも利用できるようにしている。
 * (プリコンパイル済みヘッダー StdAfx.h を使用しない)
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

#ifndef SAKURA_CVCSIGNORE_H_
#define SAKURA_CVCSIGNORE_H_

#include <memory>
#include <string>
#include <vector>
#include "CGitIgnore.h"

/*!
 * 除外指定の読み込み・照合時の警告
 */
struct SVcsIgnoreWarning
{
	std::wstring	strSource;		//!< 除外指定の種類 (例: ".hgignore")
	std::wstring	strFilePath;	//!< 警告の対象のファイルのフルパス
	int				nLine;			//!< 行番号 (行に関係しない警告は 0)
	std::wstring	strMessage;		//!< 警告の内容
};

/*!
 * 1 つの VCS の除外指定の適用状態 (抽象クラス)
 *
 * 検索フォルダがリポジトリの中なら Start()、外なら StartOutsideRepository() で開始した後、
 * フォルダに入るときに EnterDirectory()、出るときに LeaveDirectory() を呼ぶ。
 * IsIgnored() は現在のフォルダの中の名前について判定する。
 * フォルダのパスは、末尾に '\\' を付けたフルパスで指定する。
 */
class CVcsIgnoreScope
{
public:
	virtual ~CVcsIgnoreScope() {}

	//! リポジトリのルートから検索フォルダまで進める。途中のフォルダ (検索フォルダを含む) が除外されている場合は false
	virtual bool Start( const std::wstring& strRootDir, const std::wstring& strSearchDir ) = 0;

	//! リポジトリの外の検索フォルダから開始する (サブフォルダのリポジトリに入るまでは除外しない)
	virtual void StartOutsideRepository() = 0;

	//! 現在のフォルダの中のファイル・フォルダが除外されているか
	virtual bool IsIgnored( const wchar_t* pszName, bool isDirectory ) const = 0;

	//! 現在のフォルダの中のサブフォルダに入る
	virtual void EnterDirectory( const std::wstring& strDirPath, const wchar_t* pszName ) = 0;

	//! サブフォルダから出る (EnterDirectory() の処理を戻す)
	virtual void LeaveDirectory() = 0;

	//! 前回の取り出し以降に記録された警告を取り出す (pWarnings の末尾に追加する)
	virtual void TakeWarnings( std::vector<SVcsIgnoreWarning>* pWarnings ) = 0;
};

/*!
 * .gitignore の適用状態
 *
 * フォルダに入るときに、そのフォルダの .gitignore を積む。".git" (フォルダまたはファイル) のある
 * フォルダは入れ子のリポジトリ (サブモジュールなど) として、親リポジトリの .gitignore を退避する。
 * リポジトリの外から開始した場合は、".git" のあるフォルダに入ってから、そのリポジトリの .gitignore を適用する。
 */
class CGitIgnoreScope : public CVcsIgnoreScope
{
public:
	CGitIgnoreScope();

	bool Start( const std::wstring& strRootDir, const std::wstring& strSearchDir );
	void StartOutsideRepository();
	bool IsIgnored( const wchar_t* pszName, bool isDirectory ) const;
	void EnterDirectory( const std::wstring& strDirPath, const wchar_t* pszName );
	void LeaveDirectory();
	void TakeWarnings( std::vector<SVcsIgnoreWarning>* pWarnings );

private:
	//! 入ったフォルダごとの状態
	struct SFrame
	{
		std::wstring	strRelDir;			//!< リポジトリのルートからの相対パス ('/' 区切り、ルートは空文字列)
		bool			bPushedGitIgnore;	//!< .gitignore を積んだ
		bool			bNestedRepository;	//!< 入れ子のリポジトリのルート
		bool			bInRepository;		//!< リポジトリの中 (偽: 検索フォルダがリポジトリの外で、まだリポジトリに入っていない)
	};

	CGitIgnoreMatcher		m_cMatcher;		//!< 適用中の .gitignore
	std::vector<SFrame>		m_frames;		//!< 入ったフォルダのスタック (末尾が現在のフォルダ)
};

class CHgIgnoreFile;

/*!
 * .hgignore の適用状態
 *
 * リポジトリのルートの .hgignore (と include: / subinclude: で読み込んだファイル) を適用する。
 * ".hg" フォルダのあるフォルダは入れ子のリポジトリとして、親リポジトリの .hgignore を退避し、
 * そのフォルダの .hgignore を適用する。
 * リポジトリの外から開始した場合は、".hg" のあるフォルダに入ってから、そのリポジトリの .hgignore を適用する。
 */
class CHgIgnoreScope : public CVcsIgnoreScope
{
public:
	CHgIgnoreScope();
	~CHgIgnoreScope();

	bool Start( const std::wstring& strRootDir, const std::wstring& strSearchDir );
	void StartOutsideRepository();
	bool IsIgnored( const wchar_t* pszName, bool isDirectory ) const;
	void EnterDirectory( const std::wstring& strDirPath, const wchar_t* pszName );
	void LeaveDirectory();
	void TakeWarnings( std::vector<SVcsIgnoreWarning>* pWarnings );

private:
	CHgIgnoreScope( const CHgIgnoreScope& );
	CHgIgnoreScope& operator=( const CHgIgnoreScope& );

	//! 入ったフォルダごとの状態
	struct SFrame
	{
		std::wstring	strRelDir;			//!< リポジトリのルートからの相対パス ('/' 区切り、ルートは空文字列)
		bool			bNestedRepository;	//!< 入れ子のリポジトリのルート
		bool			bInRepository;		//!< リポジトリの中 (偽: 検索フォルダがリポジトリの外で、まだリポジトリに入っていない)
	};

	std::vector< std::unique_ptr<CHgIgnoreFile> >	m_repositories;	//!< リポジトリごとの .hgignore のスタック (末尾が適用中)
	std::vector<SFrame>								m_frames;		//!< 入ったフォルダのスタック (末尾が現在のフォルダ)
	std::vector<SVcsIgnoreWarning>					m_warnings;		//!< 出たリポジトリの .hgignore の警告 (取り出していないもの)
};

/*!
 * 適用中の VCS の除外指定の集まり
 *
 * 追加した CVcsIgnoreScope のどれか 1 つでも除外していれば除外する。
 * フォルダに入る・出る操作は、すべての CVcsIgnoreScope に伝える。
 */
class CVcsIgnore
{
public:
	CVcsIgnore();
	~CVcsIgnore();

	void AddScope( std::unique_ptr<CVcsIgnoreScope> pcScope );	//!< 適用状態を追加する (所有権を受け取る)
	bool IsEmpty() const { return m_scopes.empty(); }			//!< 適用状態が 1 つもないか

	bool IsIgnored( const wchar_t* pszName, bool isDirectory ) const;
	void EnterDirectory( const std::wstring& strDirPath, const wchar_t* pszName );
	void LeaveDirectory();
	void TakeWarnings( std::vector<SVcsIgnoreWarning>* pWarnings );

private:
	CVcsIgnore( const CVcsIgnore& );
	CVcsIgnore& operator=( const CVcsIgnore& );

	std::vector< std::unique_ptr<CVcsIgnoreScope> >	m_scopes;
};

#endif /* SAKURA_CVCSIGNORE_H_ */
