/*!
 *	@file
 *	@brief	単体テスト用の一時フォルダ
 *
 * テストごとに一時フォルダを作り、ファイル・フォルダを作成できるようにする。
 * デストラクタで、一時フォルダを中身ごと削除する。
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

#ifndef SAKURA_TESTTEMPDIR_H_
#define SAKURA_TESTTEMPDIR_H_

#include <Windows.h>
#include <stdio.h>
#include <string>
#include "CppUnitTest.h"

namespace BeniZakuraUnitTest
{
	/*!
	 * 単体テスト用の一時フォルダ
	 */
	class CTestTempDir
	{
	public:
		/*!
		 * CTestTempDir::CTestTempDir - コンストラクタ (一時フォルダを作る)
		 */
		CTestTempDir()
		{
			wchar_t szTemp[MAX_PATH];
			wchar_t szPath[MAX_PATH];
			Microsoft::VisualStudio::CppUnitTestFramework::Assert::AreNotEqual( (DWORD)0, ::GetTempPathW( MAX_PATH, szTemp ) );
			Microsoft::VisualStudio::CppUnitTestFramework::Assert::AreNotEqual( (UINT)0, ::GetTempFileNameW( szTemp, L"bzt", 0, szPath ) );
			::DeleteFileW( szPath );	// GetTempFileName が作ったファイルを、同じ名前のフォルダに置き換える
			Microsoft::VisualStudio::CppUnitTestFramework::Assert::IsTrue( ::CreateDirectoryW( szPath, NULL ) != FALSE );
			m_strRoot = szPath;
		}

		/*!
		 * CTestTempDir::~CTestTempDir - デストラクタ (一時フォルダを中身ごと削除する)
		 */
		~CTestTempDir()
		{
			RemoveTree( m_strRoot );
		}

		/*!
		 * CTestTempDir::GetRootPath - 一時フォルダのパスを返す
		 *
		 *	@return 一時フォルダのパス (末尾に '\\' を付けない)
		 */
		const std::wstring& GetRootPath() const
		{
			return m_strRoot;
		}

		/*!
		 * CTestTempDir::GetPath - 一時フォルダの中のパスを返す
		 *
		 *	@return フルパス (pszRelPath が空文字列の場合は一時フォルダのパス)
		 */
		std::wstring GetPath(
			const wchar_t*	pszRelPath		//!< [in] 一時フォルダからの相対パス ('\\' 区切り)
		) const
		{
			if( pszRelPath[0] == L'\0' ){
				return m_strRoot;
			}
			return m_strRoot + L"\\" + pszRelPath;
		}

		/*!
		 * CTestTempDir::MakeDir - フォルダを作る (途中のフォルダも作る)
		 */
		void MakeDir(
			const wchar_t*	pszRelPath		//!< [in] 一時フォルダからの相対パス ('\\' 区切り)
		) const
		{
			const std::wstring strRel = pszRelPath;
			std::wstring::size_type pos = 0;
			for(;;){
				pos = strRel.find( L'\\', pos );
				const std::wstring strDir = GetPath( strRel.substr( 0, pos ).c_str() );
				::CreateDirectoryW( strDir.c_str(), NULL );
				if( pos == std::wstring::npos ){
					break;
				}
				++pos;
			}
		}

		/*!
		 * CTestTempDir::MakeFile - ファイルを作る (途中のフォルダも作る)
		 */
		void MakeFile(
			const wchar_t*	pszRelPath,				//!< [in] 一時フォルダからの相対パス ('\\' 区切り)
			const char*		pszContent = "hi\n"		//!< [in] ファイルの内容 (UTF-8)
		) const
		{
			const std::wstring strRel = pszRelPath;
			const std::wstring::size_type pos = strRel.rfind( L'\\' );
			if( pos != std::wstring::npos ){
				MakeDir( strRel.substr( 0, pos ).c_str() );
			}
			FILE* fp = NULL;
			Microsoft::VisualStudio::CppUnitTestFramework::Assert::AreEqual( 0, (int)_wfopen_s( &fp, GetPath( pszRelPath ).c_str(), L"wb" ) );
			fwrite( pszContent, 1, strlen( pszContent ), fp );
			fclose( fp );
		}

	private:
		/*!
		 * CTestTempDir::RemoveTree - フォルダを中身ごと削除する
		 */
		static void RemoveTree(
			const std::wstring&	strDir		//!< [in] 削除するフォルダのパス
		)
		{
			WIN32_FIND_DATAW fd;
			HANDLE hFind = ::FindFirstFileW( ( strDir + L"\\*" ).c_str(), &fd );
			if( hFind != INVALID_HANDLE_VALUE ){
				do{
					const std::wstring name = fd.cFileName;
					if( name == L"." || name == L".." ){
						continue;
					}
					const std::wstring strPath = strDir + L"\\" + name;
					if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
						RemoveTree( strPath );
					}else{
						::SetFileAttributesW( strPath.c_str(), FILE_ATTRIBUTE_NORMAL );
						::DeleteFileW( strPath.c_str() );
					}
				}while( ::FindNextFileW( hFind, &fd ) );
				::FindClose( hFind );
			}
			::RemoveDirectoryW( strDir.c_str() );
		}

		CTestTempDir( const CTestTempDir& );
		CTestTempDir& operator=( const CTestTempDir& );

		std::wstring	m_strRoot;	//!< 一時フォルダのパス
	};
}

#endif /* SAKURA_TESTTEMPDIR_H_ */
