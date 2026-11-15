/*!
 *	@file
 *	@brief	CVcsIgnore (VCS の除外指定の適用状態) の単体テスト
 *
 * 一時フォルダにファイル構成を作り、Grep と同じ順序でフォルダをたどって、検索対象となるファイルを確認する。
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

#include <Windows.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include "CppUnitTest.h"
#include "util/CVcsIgnore.h"
#include "TestTempDir.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeniZakuraUnitTest
{
	/*!
	 * CollectFiles - Grep と同じ順序でフォルダをたどり、除外されていないファイルを集める
	 *
	 * フォルダ ".git" と ".hg" には入らない (Grep と同じ)。
	 */
	static void CollectFiles(
		CVcsIgnore*					pcVcsIgnore,	//!< [i/o] 適用中の除外指定 (strDirPath のフォルダの状態)
		const std::wstring&			strDirPath,		//!< [in] フォルダのパス (末尾に '\\' あり)
		const std::wstring&			strRelDir,		//!< [in] 検索フォルダからの相対パス ('/' 区切り。検索フォルダは空文字列)
		std::vector<std::wstring>*	pFiles			//!< [out] 除外されていないファイル (検索フォルダからの相対パス)
	)
	{
		WIN32_FIND_DATAW fd;
		HANDLE hFind = ::FindFirstFileW( ( strDirPath + L"*" ).c_str(), &fd );
		if( hFind == INVALID_HANDLE_VALUE ){
			return;
		}
		std::vector<std::wstring> dirs;
		do{
			const std::wstring name = fd.cFileName;
			if( name == L"." || name == L".." ){
				continue;
			}
			if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
				if( name != L".git" && name != L".hg" ){
					dirs.push_back( name );
				}
			}else if( !pcVcsIgnore->IsIgnored( name.c_str(), false ) ){
				pFiles->push_back( strRelDir.empty() ? name : strRelDir + L"/" + name );
			}
		}while( ::FindNextFileW( hFind, &fd ) );
		::FindClose( hFind );

		for( size_t i = 0; i < dirs.size(); ++i ){
			if( pcVcsIgnore->IsIgnored( dirs[i].c_str(), true ) ){
				continue;
			}
			const std::wstring strSubDir = strDirPath + dirs[i] + L"\\";
			pcVcsIgnore->EnterDirectory( strSubDir, dirs[i].c_str() );
			CollectFiles( pcVcsIgnore, strSubDir, strRelDir.empty() ? dirs[i] : strRelDir + L"/" + dirs[i], pFiles );
			pcVcsIgnore->LeaveDirectory();
		}
	}

	/*!
	 * CollectGitFiles - .gitignore を適用して検索フォルダの中をたどり、除外されていないファイルを集める
	 *
	 *	@return 除外されていないファイル (検索フォルダからの相対パス、昇順)。検索フォルダが除外されている場合は "(ignored)" のみ
	 */
	static std::vector<std::wstring> CollectGitFiles(
		const CTestTempDir&	cDir,			//!< [in] 一時フォルダ (リポジトリのルート)
		const wchar_t*		pszSearchDir	//!< [in] 検索フォルダ (ルートからの相対パス、'\\' 区切り。ルートは空文字列)
	)
	{
		const std::wstring strSearchDir = cDir.GetPath( pszSearchDir ) + L"\\";
		std::unique_ptr<CVcsIgnoreScope> pcScope( new CGitIgnoreScope() );
		std::vector<std::wstring> files;
		if( !pcScope->Start( cDir.GetRootPath(), strSearchDir ) ){
			files.push_back( L"(ignored)" );
			return files;
		}
		CVcsIgnore cVcsIgnore;
		cVcsIgnore.AddScope( std::move( pcScope ) );
		CollectFiles( &cVcsIgnore, strSearchDir, L"", &files );
		std::sort( files.begin(), files.end() );
		return files;
	}

	/*!
	 * JoinLines - 文字列の一覧を、比較・表示用に 1 つの文字列 (';' 区切り) にする
	 *
	 *	@return 連結した文字列
	 */
	static std::wstring JoinLines(
		const std::vector<std::wstring>&	lines	//!< [in] 文字列の一覧
	)
	{
		std::wstring s;
		for( size_t i = 0; i < lines.size(); ++i ){
			if( i ){
				s += L";";
			}
			s += lines[i];
		}
		return s;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      CGitIgnoreScope                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnoreScopeTest )
	{
	public:
		TEST_METHOD( AppliesGitIgnoreOfEachFolder )
		{
			CTestTempDir cDir;
			cDir.MakeDir( L".git" );
			cDir.MakeFile( L".gitignore", "*.log\nbuild/\n" );
			cDir.MakeFile( L"a.log" );
			cDir.MakeFile( L"a.c" );
			cDir.MakeFile( L"build\\x.c" );
			cDir.MakeFile( L"src\\.gitignore", "!keep.log\n*.tmp\n" );
			cDir.MakeFile( L"src\\keep.log" );
			cDir.MakeFile( L"src\\b.log" );
			cDir.MakeFile( L"src\\b.tmp" );
			cDir.MakeFile( L"src\\sub\\c.c" );
			cDir.MakeFile( L"c.tmp" );

			Assert::AreEqual(
				std::wstring( L".gitignore;a.c;c.tmp;src/.gitignore;src/keep.log;src/sub/c.c" ),
				JoinLines( CollectGitFiles( cDir, L"" ) ) );
		}

		TEST_METHOD( StartAppliesAncestorGitIgnore )
		{
			// 検索フォルダより上のフォルダの .gitignore も適用する
			CTestTempDir cDir;
			cDir.MakeDir( L".git" );
			cDir.MakeFile( L".gitignore", "*.log\n/src/sub/root-only.txt\n" );
			cDir.MakeFile( L"src\\.gitignore", "*.tmp\n" );
			cDir.MakeFile( L"src\\sub\\a.log" );
			cDir.MakeFile( L"src\\sub\\a.tmp" );
			cDir.MakeFile( L"src\\sub\\root-only.txt" );
			cDir.MakeFile( L"src\\sub\\a.c" );

			Assert::AreEqual( std::wstring( L"a.c" ), JoinLines( CollectGitFiles( cDir, L"src\\sub" ) ) );
		}

		TEST_METHOD( SearchFolderIgnored )
		{
			// 検索フォルダ自体、または途中のフォルダが除外されている場合は、中をすべて除外する
			CTestTempDir cDir;
			cDir.MakeDir( L".git" );
			cDir.MakeFile( L".gitignore", "build/\n" );
			cDir.MakeFile( L"build\\x\\a.c" );
			cDir.MakeFile( L"src\\a.c" );

			Assert::AreEqual( std::wstring( L"(ignored)" ), JoinLines( CollectGitFiles( cDir, L"build" ) ) );
			Assert::AreEqual( std::wstring( L"(ignored)" ), JoinLines( CollectGitFiles( cDir, L"build\\x" ) ) );
			Assert::AreEqual( std::wstring( L"a.c" ), JoinLines( CollectGitFiles( cDir, L"src" ) ) );
		}

		TEST_METHOD( NestedRepository )
		{
			// 入れ子のリポジトリ (".git" ファイルのサブモジュール) には、親リポジトリの .gitignore を適用しない
			CTestTempDir cDir;
			cDir.MakeDir( L".git" );
			cDir.MakeFile( L".gitignore", "*.o\n" );
			cDir.MakeFile( L"a.o" );
			cDir.MakeFile( L"lib\\.git", "gitdir: ../.git/modules/lib\n" );
			cDir.MakeFile( L"lib\\.gitignore", "*.tmp\n" );
			cDir.MakeFile( L"lib\\b.o" );
			cDir.MakeFile( L"lib\\b.tmp" );
			cDir.MakeFile( L"src\\c.o" );
			cDir.MakeFile( L"src\\c.tmp" );

			Assert::AreEqual(
				std::wstring( L".gitignore;lib/.git;lib/.gitignore;lib/b.o;src/c.tmp" ),
				JoinLines( CollectGitFiles( cDir, L"" ) ) );
		}

		TEST_METHOD( LeaveDirectoryRestoresState )
		{
			// サブフォルダから出ると、そのフォルダの .gitignore は適用されなくなる
			CTestTempDir cDir;
			cDir.MakeDir( L".git" );
			cDir.MakeFile( L"sub\\.gitignore", "*.tmp\n" );

			CGitIgnoreScope cScope;
			Assert::IsTrue( cScope.Start( cDir.GetRootPath(), cDir.GetRootPath() + L"\\" ) );
			Assert::IsFalse( cScope.IsIgnored( L"a.tmp", false ) );
			cScope.EnterDirectory( cDir.GetPath( L"sub" ) + L"\\", L"sub" );
			Assert::IsTrue( cScope.IsIgnored( L"a.tmp", false ) );
			cScope.LeaveDirectory();
			Assert::IsFalse( cScope.IsIgnored( L"a.tmp", false ) );
			// ルートより上には出ない
			cScope.LeaveDirectory();
			Assert::IsFalse( cScope.IsIgnored( L"a.tmp", false ) );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         CVcsIgnore                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CVcsIgnoreTest )
	{
	public:
		TEST_METHOD( EmptyIgnoresNothing )
		{
			CVcsIgnore cVcsIgnore;
			Assert::IsTrue( cVcsIgnore.IsEmpty() );
			Assert::IsFalse( cVcsIgnore.IsIgnored( L"a.o", false ) );
		}
	};
}
