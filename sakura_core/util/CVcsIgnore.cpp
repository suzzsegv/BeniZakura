/*!
 *	@file
 *	@brief	VCS (Git / Mercurial) の除外指定の適用状態
 *
 * Grep でフォルダをたどりながら、VCS ごとの除外指定を互いに独立して適用する (どれか 1 つで除外されれば除外)。
 * .gitignore は ".git" を境界とするリポジトリごとに、フォルダに入るたびにそのフォルダの .gitignore を積む。
 * .hgignore は ".hg" を境界とするリポジトリごとに、ルートの .hgignore を適用する。
 * 途中のフォルダが除外されている場合は、その中をすべて除外する (除外されたフォルダには入らない)。
 * 検索フォルダがリポジトリの外の場合は、たどる途中でリポジトリのフォルダに入ってから、その除外指定を適用する。
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

#include "CVcsIgnore.h"
#include "CHgIgnore.h"

#include <Windows.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         補助関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * JoinRelPath - リポジトリのルートからの相対パス ('/' 区切り) に、名前を 1 つ連結する
 *
 *	@return 連結した相対パス
 */
static std::wstring JoinRelPath(
	const std::wstring&	strRelDir,	//!< [in] フォルダの、リポジトリのルートからの相対パス (ルートは空文字列)
	const wchar_t*		pszName		//!< [in] ファイル名・フォルダ名
)
{
	if( strRelDir.empty() ){
		return pszName;
	}
	return strRelDir + L"/" + pszName;
}

/*!
 * PathExists - ファイルまたはフォルダが存在するかを返す
 *
 *	@return true: 存在する / false: 存在しない
 */
static bool PathExists(
	const std::wstring&	strPath		//!< [in] パス
)
{
	return ::GetFileAttributesW( strPath.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

/*!
 * DirectoryExists - フォルダが存在するかを返す
 *
 *	@return true: フォルダが存在する / false: 存在しない (ファイルの場合を含む)
 */
static bool DirectoryExists(
	const std::wstring&	strPath		//!< [in] パス
)
{
	const DWORD dwAttr = ::GetFileAttributesW( strPath.c_str() );
	return dwAttr != INVALID_FILE_ATTRIBUTES && ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) != 0;
}

/*!
 * SplitSearchDirNames - 検索フォルダの、ルートより下のフォルダ名を上位から順に取り出す
 *
 * 検索フォルダは「ルート + '\\' + フォルダ名 + '\\' + … + '\\'」の形のフルパスとする。
 *
 *	@return フォルダ名の一覧 (検索フォルダがルートの場合は空)
 */
static std::vector<std::wstring> SplitSearchDirNames(
	const std::wstring&	strRootDir,		//!< [in] リポジトリのルートのパス (末尾に '\\' を付けない)
	const std::wstring&	strSearchDir	//!< [in] 検索フォルダのフルパス (末尾に '\\' あり)
)
{
	std::vector<std::wstring> names;
	std::wstring::size_type pos = strRootDir.size() + 1;
	while( pos < strSearchDir.size() ){
		std::wstring::size_type posEnd = strSearchDir.find( L'\\', pos );
		if( posEnd == std::wstring::npos ){
			posEnd = strSearchDir.size();
		}
		if( pos < posEnd ){
			names.push_back( strSearchDir.substr( pos, posEnd - pos ) );
		}
		pos = posEnd + 1;
	}
	return names;
}

/*!
 * PushGitIgnoreFile - フォルダに .gitignore があれば読み込んで、適用中の .gitignore に積む
 *
 *	@return true: 積んだ / false: .gitignore がない (積んでいない)
 */
static bool PushGitIgnoreFile(
	CGitIgnoreMatcher*	pcMatcher,		//!< [i/o] 適用中の .gitignore
	const std::wstring&	strDirPath,		//!< [in] フォルダのパス (末尾に '\\' あり)
	const std::wstring&	strRelDir		//!< [in] フォルダの、リポジトリのルートからの相対パス
)
{
	CGitIgnoreFile cFile( strRelDir );
	if( !cFile.LoadFromFile( ( strDirPath + L".gitignore" ).c_str() ) ){
		return false;
	}
	pcMatcher->PushFile( cFile );
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CGitIgnoreScope                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CGitIgnoreScope::CGitIgnoreScope - コンストラクタ
 */
CGitIgnoreScope::CGitIgnoreScope()
{
}

/*!
 * CGitIgnoreScope::Start - リポジトリのルートから検索フォルダまで進める
 *
 * ルートから検索フォルダまでの各フォルダの .gitignore を、上位から順に積む。
 * 途中のフォルダ (検索フォルダを含む) が除外されている場合は、そこで止める。
 *
 *	@return true: 検索フォルダまで進めた / false: 途中のフォルダ (検索フォルダを含む) が除外されている
 */
bool CGitIgnoreScope::Start(
	const std::wstring&	strRootDir,		//!< [in] リポジトリのルートのパス (末尾に '\\' を付けない)
	const std::wstring&	strSearchDir	//!< [in] 検索フォルダのフルパス (末尾に '\\' あり。ルートまたはその下)
)
{
	m_cMatcher = CGitIgnoreMatcher();
	m_frames.clear();

	std::wstring strDir = strRootDir + L"\\";
	SFrame frame;
	frame.bPushedGitIgnore = PushGitIgnoreFile( &m_cMatcher, strDir, frame.strRelDir );
	frame.bNestedRepository = false;
	frame.bInRepository = true;
	m_frames.push_back( frame );

	const std::vector<std::wstring> names = SplitSearchDirNames( strRootDir, strSearchDir );
	for( size_t i = 0; i < names.size(); ++i ){
		if( IsIgnored( names[i].c_str(), true ) ){
			return false;
		}
		strDir += names[i];
		strDir += L"\\";
		EnterDirectory( strDir, names[i].c_str() );
	}
	return true;
}

/*!
 * CGitIgnoreScope::StartOutsideRepository - リポジトリの外の検索フォルダから開始する
 *
 * ".git" のあるサブフォルダに入るまでは除外しない。
 */
void CGitIgnoreScope::StartOutsideRepository()
{
	m_cMatcher = CGitIgnoreMatcher();
	m_frames.clear();

	SFrame frame;
	frame.bPushedGitIgnore = false;
	frame.bNestedRepository = false;
	frame.bInRepository = false;
	m_frames.push_back( frame );
}

/*!
 * CGitIgnoreScope::IsIgnored - 現在のフォルダの中のファイル・フォルダが除外されているかを返す
 *
 *	@return true: 除外されている / false: 除外されていない (リポジトリの外では常に false)
 */
bool CGitIgnoreScope::IsIgnored(
	const wchar_t*	pszName,		//!< [in] ファイル名・フォルダ名
	bool			isDirectory		//!< [in] フォルダか
) const
{
	if( m_frames.empty() || !m_frames.back().bInRepository ){
		return false;
	}
	return m_cMatcher.IsIgnored( JoinRelPath( m_frames.back().strRelDir, pszName ), isDirectory );
}

/*!
 * CGitIgnoreScope::EnterDirectory - 現在のフォルダの中のサブフォルダに入る
 *
 * ".git" のあるフォルダは入れ子のリポジトリとして、親リポジトリの .gitignore を退避し、
 * そのフォルダをルートとする (リポジトリの外から入った場合は、そこからリポジトリの中になる)。
 * リポジトリの中なら、そのフォルダに .gitignore があれば積む。
 */
void CGitIgnoreScope::EnterDirectory(
	const std::wstring&	strDirPath,		//!< [in] サブフォルダのパス (末尾に '\\' あり)
	const wchar_t*		pszName			//!< [in] サブフォルダの名前
)
{
	if( m_frames.empty() ){
		return;
	}
	const SFrame& parent = m_frames.back();
	SFrame frame;
	frame.bNestedRepository = PathExists( strDirPath + L".git" );
	frame.bPushedGitIgnore = false;
	if( frame.bNestedRepository ){
		m_cMatcher.BeginRepository();
		frame.bInRepository = true;
	}else{
		frame.bInRepository = parent.bInRepository;
		if( frame.bInRepository ){
			frame.strRelDir = JoinRelPath( parent.strRelDir, pszName );
		}
	}
	if( frame.bInRepository ){
		frame.bPushedGitIgnore = PushGitIgnoreFile( &m_cMatcher, strDirPath, frame.strRelDir );
	}
	m_frames.push_back( frame );
}

/*!
 * CGitIgnoreScope::LeaveDirectory - サブフォルダから出る
 *
 * EnterDirectory() で積んだ .gitignore を降ろし、入れ子のリポジトリなら親リポジトリの .gitignore に戻す。
 * ルートのフォルダからは出ない。
 */
void CGitIgnoreScope::LeaveDirectory()
{
	if( m_frames.size() <= 1 ){
		return;
	}
	const SFrame& frame = m_frames.back();
	if( frame.bPushedGitIgnore ){
		m_cMatcher.PopFile();
	}
	if( frame.bNestedRepository ){
		m_cMatcher.EndRepository();
	}
	m_frames.pop_back();
}

/*!
 * CGitIgnoreScope::TakeWarnings - 記録された警告を取り出す
 *
 * .gitignore の解釈では警告を記録しないため、何もしない。
 */
void CGitIgnoreScope::TakeWarnings(
	std::vector<SVcsIgnoreWarning>*	pWarnings	//!< [out] 警告の追加先
)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CHgIgnoreScope                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CHgIgnoreScope::CHgIgnoreScope - コンストラクタ
 */
CHgIgnoreScope::CHgIgnoreScope()
{
}

/*!
 * CHgIgnoreScope::~CHgIgnoreScope - デストラクタ
 */
CHgIgnoreScope::~CHgIgnoreScope()
{
}

/*!
 * CHgIgnoreScope::Start - リポジトリのルートから検索フォルダまで進める
 *
 * ルートの .hgignore を読み込み、ルートから検索フォルダまでの各フォルダが除外されていないかを調べる。
 * 途中のフォルダ (検索フォルダを含む) が除外されている場合は、そこで止める。
 *
 *	@return true: 検索フォルダまで進めた / false: 途中のフォルダ (検索フォルダを含む) が除外されている
 */
bool CHgIgnoreScope::Start(
	const std::wstring&	strRootDir,		//!< [in] リポジトリのルートのパス (末尾に '\' を付けない)
	const std::wstring&	strSearchDir	//!< [in] 検索フォルダのフルパス (末尾に '\' あり。ルートまたはその下)
)
{
	m_repositories.clear();
	m_frames.clear();
	m_warnings.clear();

	std::unique_ptr<CHgIgnoreFile> pcFile( new CHgIgnoreFile() );
	pcFile->LoadFromFile( strRootDir );
	m_repositories.push_back( std::move( pcFile ) );
	SFrame frame;
	frame.bNestedRepository = false;
	frame.bInRepository = true;
	m_frames.push_back( frame );

	std::wstring strDir = strRootDir + L"\\";
	const std::vector<std::wstring> names = SplitSearchDirNames( strRootDir, strSearchDir );
	for( size_t i = 0; i < names.size(); ++i ){
		if( IsIgnored( names[i].c_str(), true ) ){
			return false;
		}
		strDir += names[i];
		strDir += L"\\";
		EnterDirectory( strDir, names[i].c_str() );
	}
	return true;
}

/*!
 * CHgIgnoreScope::StartOutsideRepository - リポジトリの外の検索フォルダから開始する
 *
 * ".hg" のあるサブフォルダに入るまでは除外しない。
 */
void CHgIgnoreScope::StartOutsideRepository()
{
	m_repositories.clear();
	m_frames.clear();
	m_warnings.clear();

	SFrame frame;
	frame.bNestedRepository = false;
	frame.bInRepository = false;
	m_frames.push_back( frame );
}

/*!
 * CHgIgnoreScope::IsIgnored - 現在のフォルダの中のファイル・フォルダが除外されているかを返す
 *
 * .hgignore にはディレクトリのみの指定がないため、isDirectory は判定に使わない。
 *
 *	@return true: 除外されている / false: 除外されていない (リポジトリの外では常に false)
 */
bool CHgIgnoreScope::IsIgnored(
	const wchar_t*	pszName,		//!< [in] ファイル名・フォルダ名
	bool			isDirectory		//!< [in] フォルダか
) const
{
	if( m_frames.empty() || !m_frames.back().bInRepository || m_repositories.empty() ){
		return false;
	}
	return m_repositories.back()->IsIgnored( JoinRelPath( m_frames.back().strRelDir, pszName ) );
}

/*!
 * CHgIgnoreScope::EnterDirectory - 現在のフォルダの中のサブフォルダに入る
 *
 * ".hg" フォルダのあるフォルダは入れ子のリポジトリとして、そのフォルダの .hgignore を読み込んで適用する
 * (親リポジトリの .hgignore は退避する。リポジトリの外から入った場合は、そこからリポジトリの中になる)。
 */
void CHgIgnoreScope::EnterDirectory(
	const std::wstring&	strDirPath,		//!< [in] サブフォルダのパス (末尾に '\' あり)
	const wchar_t*		pszName			//!< [in] サブフォルダの名前
)
{
	if( m_frames.empty() ){
		return;
	}
	const SFrame& parent = m_frames.back();
	SFrame frame;
	frame.bNestedRepository = DirectoryExists( strDirPath + L".hg" );
	if( frame.bNestedRepository ){
		std::unique_ptr<CHgIgnoreFile> pcFile( new CHgIgnoreFile() );
		pcFile->LoadFromFile( strDirPath.substr( 0, strDirPath.size() - 1 ) );	// 末尾の '\' を除く
		m_repositories.push_back( std::move( pcFile ) );
		frame.bInRepository = true;
	}else{
		frame.bInRepository = parent.bInRepository;
		if( frame.bInRepository ){
			frame.strRelDir = JoinRelPath( parent.strRelDir, pszName );
		}
	}
	m_frames.push_back( frame );
}

/*!
 * CHgIgnoreScope::LeaveDirectory - サブフォルダから出る
 *
 * 入れ子のリポジトリから出る場合は、そのリポジトリの .hgignore の警告を退避してから、
 * 親リポジトリの .hgignore に戻す。ルートのフォルダからは出ない。
 */
void CHgIgnoreScope::LeaveDirectory()
{
	if( m_frames.size() <= 1 ){
		return;
	}
	if( m_frames.back().bNestedRepository && !m_repositories.empty() ){
		m_repositories.back()->TakeWarnings( &m_warnings );
		m_repositories.pop_back();
	}
	m_frames.pop_back();
}

/*!
 * CHgIgnoreScope::TakeWarnings - 前回の取り出し以降に記録された警告を取り出す
 *
 * 適用中のリポジトリの .hgignore の警告と、出たリポジトリの .hgignore の警告を取り出す。
 */
void CHgIgnoreScope::TakeWarnings(
	std::vector<SVcsIgnoreWarning>*	pWarnings	//!< [out] 警告の追加先
)
{
	pWarnings->insert( pWarnings->end(), m_warnings.begin(), m_warnings.end() );
	m_warnings.clear();
	for( size_t i = 0; i < m_repositories.size(); ++i ){
		m_repositories[i]->TakeWarnings( pWarnings );
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CVcsIgnore                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * CVcsIgnore::CVcsIgnore - コンストラクタ
 */
CVcsIgnore::CVcsIgnore()
{
}

/*!
 * CVcsIgnore::~CVcsIgnore - デストラクタ
 */
CVcsIgnore::~CVcsIgnore()
{
}

/*!
 * CVcsIgnore::AddScope - 適用状態を追加する
 *
 * Start() を済ませた適用状態を追加する。所有権を受け取る。
 */
void CVcsIgnore::AddScope(
	std::unique_ptr<CVcsIgnoreScope>	pcScope		//!< [in] 追加する適用状態
)
{
	if( pcScope ){
		m_scopes.push_back( std::move( pcScope ) );
	}
}

/*!
 * CVcsIgnore::IsIgnored - 現在のフォルダの中のファイル・フォルダが除外されているかを返す
 *
 * 追加した適用状態のどれか 1 つでも除外していれば除外する。
 *
 *	@return true: 除外されている / false: 除外されていない
 */
bool CVcsIgnore::IsIgnored(
	const wchar_t*	pszName,		//!< [in] ファイル名・フォルダ名
	bool			isDirectory		//!< [in] フォルダか
) const
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		if( m_scopes[i]->IsIgnored( pszName, isDirectory ) ){
			return true;
		}
	}
	return false;
}

/*!
 * CVcsIgnore::EnterDirectory - 現在のフォルダの中のサブフォルダに入る (すべての適用状態に伝える)
 */
void CVcsIgnore::EnterDirectory(
	const std::wstring&	strDirPath,		//!< [in] サブフォルダのパス (末尾に '\\' あり)
	const wchar_t*		pszName			//!< [in] サブフォルダの名前
)
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		m_scopes[i]->EnterDirectory( strDirPath, pszName );
	}
}

/*!
 * CVcsIgnore::LeaveDirectory - サブフォルダから出る (すべての適用状態に伝える)
 */
void CVcsIgnore::LeaveDirectory()
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		m_scopes[i]->LeaveDirectory();
	}
}

/*!
 * CVcsIgnore::TakeWarnings - 前回の取り出し以降に記録された警告を、すべての適用状態から取り出す
 */
void CVcsIgnore::TakeWarnings(
	std::vector<SVcsIgnoreWarning>*	pWarnings	//!< [out] 警告の追加先
)
{
	for( size_t i = 0; i < m_scopes.size(); ++i ){
		m_scopes[i]->TakeWarnings( pWarnings );
	}
}
