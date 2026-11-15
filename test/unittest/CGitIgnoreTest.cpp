/*!
 *	@file
 *	@brief	CGitIgnore (.gitignore の除外指定の解釈) の単体テスト
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
#include <string>
#include "CppUnitTest.h"
#include "util/CGitIgnore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeniZakuraUnitTest
{
	/*!
	 * MatchLine - .gitignore の 1 行をパターンとして、パスと照合する
	 *
	 *	@return true: 一致する / false: 一致しない
	 */
	static bool MatchLine(
		const wchar_t*	line,					//!< [in] .gitignore の 1 行
		const wchar_t*	relPath,				//!< [in] 照合するパス ('/' 区切り)
		bool			isDirectory = false		//!< [in] フォルダか
	)
	{
		CGitIgnorePattern pattern;
		Assert::IsTrue( pattern.Parse( line ), line );
		return pattern.Match( relPath, isDirectory );
	}

	/*!
	 * IsIgnored - リポジトリのルートに置いた .gitignore で、パスが除外されるかを返す
	 *
	 *	@return true: 除外する / false: 除外しない
	 */
	static bool IsIgnored(
		const wchar_t*	content,				//!< [in] .gitignore の内容
		const wchar_t*	relPath,				//!< [in] リポジトリのルートからの相対パス ('/' 区切り)
		bool			isDirectory = false		//!< [in] フォルダか
	)
	{
		CGitIgnoreFile file;
		file.LoadFromString( content );
		CGitIgnoreMatcher matcher;
		matcher.PushFile( file );
		return matcher.IsIgnored( relPath, isDirectory );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         行の解析                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnorePatternParseTest )
	{
	public:
		TEST_METHOD( BlankAndComment )
		{
			CGitIgnorePattern p;
			Assert::IsFalse( p.Parse( L"" ) );
			Assert::IsFalse( p.Parse( L"\r" ) );
			Assert::IsFalse( p.Parse( L"   " ) );
			Assert::IsFalse( p.Parse( L"# comment" ) );
			Assert::IsFalse( p.Parse( L"/" ) );
			Assert::IsFalse( p.Parse( L"!" ) );
		}

		TEST_METHOD( EscapedHash )
		{
			// "\#" で始まる行はコメントではない
			Assert::IsTrue( MatchLine( L"\\#file", L"#file" ) );
		}

		TEST_METHOD( EscapedExclamation )
		{
			CGitIgnorePattern p;
			Assert::IsTrue( p.Parse( L"\\!important" ) );
			Assert::IsFalse( p.IsNegative() );
			Assert::IsTrue( p.Match( L"!important", false ) );
		}

		TEST_METHOD( TrailingSpaces )
		{
			CGitIgnorePattern p;
			Assert::IsTrue( p.Parse( L"foo.txt   \r" ) );
			Assert::AreEqual( std::wstring( L"foo.txt" ), p.GetPattern() );
			Assert::IsTrue( p.Match( L"foo.txt", false ) );
		}

		TEST_METHOD( EscapedTrailingSpace )
		{
			Assert::IsTrue( MatchLine( L"foo\\ ", L"foo " ) );
			Assert::IsFalse( MatchLine( L"foo\\ ", L"foo" ) );
		}

		TEST_METHOD( Flags )
		{
			CGitIgnorePattern p;
			Assert::IsTrue( p.Parse( L"!/build/" ) );
			Assert::IsTrue( p.IsNegative() );
			Assert::IsTrue( p.IsDirectoryOnly() );
			Assert::IsTrue( p.IsAnchored() );
			Assert::AreEqual( std::wstring( L"build" ), p.GetPattern() );

			Assert::IsTrue( p.Parse( L"*.o" ) );
			Assert::IsFalse( p.IsNegative() );
			Assert::IsFalse( p.IsDirectoryOnly() );
			Assert::IsFalse( p.IsAnchored() );

			Assert::IsTrue( p.Parse( L"doc/frotz" ) );
			Assert::IsTrue( p.IsAnchored() );

			// 末尾の '/' だけでは位置を固定しない
			Assert::IsTrue( p.Parse( L"frotz/" ) );
			Assert::IsFalse( p.IsAnchored() );
			Assert::IsTrue( p.IsDirectoryOnly() );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     ワイルドカードの照合                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnoreWildMatchTest )
	{
	public:
		TEST_METHOD( Literal )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"abc", L"abc" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"abc", L"abcd" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"abc", L"ab" ) );
			// 英大文字と小文字を区別する
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"abc", L"ABC" ) );
		}

		TEST_METHOD( Star )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"*.txt", L"a.txt" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"*.txt", L".txt" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a*b*c", L"aXXbYYc" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"*.txt", L"a.txt.bak" ) );
			// '*' は '/' に一致しない
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"*.txt", L"dir/a.txt" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a*", L"a/b" ) );
		}

		TEST_METHOD( Question )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a?c", L"abc" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a?c", L"ac" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a?c", L"a/c" ) );
		}

		TEST_METHOD( CharClass )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[abc].o", L"b.o" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"[abc].o", L"d.o" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[a-c]x", L"cx" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"[a-c]x", L"dx" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[!a-c]x", L"dx" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"[!a-c]x", L"ax" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[^a]x", L"bx" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[]a]", L"]" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[a-]", L"-" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"[\\]]", L"]" ) );
			// 文字クラスは '/' に一致しない
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a[/]b", L"a/b" ) );
			// 閉じていない文字クラスには一致しない
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"[abc", L"a" ) );
		}

		TEST_METHOD( Escape )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"\\*", L"*" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"\\*", L"a" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a\\?", L"a?" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a\\?", L"ab" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"\\[a]", L"[a]" ) );
		}

		TEST_METHOD( LeadingDoubleStar )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"**/foo", L"foo" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"**/foo", L"a/foo" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"**/foo", L"a/b/foo" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"**/foo", L"a/xfoo" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"**/foo/bar", L"a/foo/bar" ) );
		}

		TEST_METHOD( TrailingDoubleStar )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"abc/**", L"abc/x" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"abc/**", L"abc/x/y" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"abc/**", L"abc" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"abc/**", L"abcd/x" ) );
		}

		TEST_METHOD( MiddleDoubleStar )
		{
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a/**/b", L"a/b" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a/**/b", L"a/x/b" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a/**/b", L"a/x/y/b" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a/**/b", L"a/x/c" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a/**/b", L"ab" ) );
		}

		TEST_METHOD( NonSpecialDoubleStar )
		{
			// '/' で区切られていない "**" は "*" と同じ
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"a**b", L"aXb" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"a**b", L"a/b" ) );
			Assert::IsTrue( CGitIgnorePattern::WildMatch( L"**.txt", L"a.txt" ) );
			Assert::IsFalse( CGitIgnorePattern::WildMatch( L"**.txt", L"d/a.txt" ) );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     パターンとパスの照合                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnorePatternMatchTest )
	{
	public:
		TEST_METHOD( NameMatchesAnyDepth )
		{
			// '/' を含まないパターンは、どの階層の名前にも一致する
			Assert::IsTrue( MatchLine( L"*.o", L"a.o" ) );
			Assert::IsTrue( MatchLine( L"*.o", L"src/sub/a.o" ) );
			Assert::IsTrue( MatchLine( L"build", L"x/build", true ) );
			Assert::IsTrue( MatchLine( L"build", L"x/build", false ) );
		}

		TEST_METHOD( AnchoredMatchesFromBase )
		{
			Assert::IsTrue( MatchLine( L"/build", L"build", true ) );
			Assert::IsFalse( MatchLine( L"/build", L"x/build", true ) );
			Assert::IsTrue( MatchLine( L"doc/frotz", L"doc/frotz" ) );
			Assert::IsFalse( MatchLine( L"doc/frotz", L"a/doc/frotz" ) );
			Assert::IsTrue( MatchLine( L"doc/*.txt", L"doc/a.txt" ) );
			Assert::IsFalse( MatchLine( L"doc/*.txt", L"doc/sub/a.txt" ) );
		}

		TEST_METHOD( DirectoryOnly )
		{
			Assert::IsTrue( MatchLine( L"build/", L"build", true ) );
			Assert::IsFalse( MatchLine( L"build/", L"build", false ) );
			Assert::IsTrue( MatchLine( L"build/", L"a/build", true ) );
			Assert::IsTrue( MatchLine( L"/out/", L"out", true ) );
			Assert::IsFalse( MatchLine( L"/out/", L"out", false ) );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       .gitignore ファイル                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnoreFileTest )
	{
	public:
		TEST_METHOD( LoadFromString )
		{
			CGitIgnoreFile file;
			file.LoadFromString( L"# comment\r\n*.o\r\n\r\n!keep.o\r\nbuild/" );
			Assert::AreEqual( (size_t)3, file.GetPatternCount() );
		}

		TEST_METHOD( LaterLineWins )
		{
			Assert::IsTrue( IsIgnored( L"*.o\n", L"a.o" ) );
			Assert::IsFalse( IsIgnored( L"*.o\n!keep.o\n", L"keep.o" ) );
			Assert::IsTrue( IsIgnored( L"*.o\n!keep.o\n", L"other.o" ) );
			// 否定の後に再度除外
			Assert::IsTrue( IsIgnored( L"*.o\n!keep.o\nkeep.o\n", L"keep.o" ) );
		}

		TEST_METHOD( NotMatched )
		{
			Assert::IsFalse( IsIgnored( L"*.o\n", L"a.c" ) );
			Assert::IsFalse( IsIgnored( L"", L"a.c" ) );
		}

		TEST_METHOD( BaseDirScope )
		{
			// sub/.gitignore のパターンは sub の中にだけ適用し、sub からの相対パスで照合する
			CGitIgnoreFile file( L"sub" );
			file.LoadFromString( L"/gen\n*.tmp\n" );
			Assert::AreEqual( (int)GITIGNORE_IGNORED, (int)file.Check( L"sub/gen", true ) );
			Assert::AreEqual( (int)GITIGNORE_NONE, (int)file.Check( L"sub/x/gen", true ) );
			Assert::AreEqual( (int)GITIGNORE_IGNORED, (int)file.Check( L"sub/x/a.tmp", false ) );
			Assert::AreEqual( (int)GITIGNORE_NONE, (int)file.Check( L"a.tmp", false ) );
			Assert::AreEqual( (int)GITIGNORE_NONE, (int)file.Check( L"subx/a.tmp", false ) );
			Assert::AreEqual( (int)GITIGNORE_NONE, (int)file.Check( L"sub", true ) );
		}

		TEST_METHOD( CheckResult )
		{
			CGitIgnoreFile file;
			file.LoadFromString( L"*.log\n!important.log\n" );
			Assert::AreEqual( (int)GITIGNORE_IGNORED, (int)file.Check( L"a.log", false ) );
			Assert::AreEqual( (int)GITIGNORE_INCLUDED, (int)file.Check( L"important.log", false ) );
			Assert::AreEqual( (int)GITIGNORE_NONE, (int)file.Check( L"a.txt", false ) );
		}

		TEST_METHOD( LoadFromFile )
		{
			wchar_t szTempDir[MAX_PATH];
			wchar_t szPath[MAX_PATH];
			Assert::AreNotEqual( (DWORD)0, ::GetTempPathW( MAX_PATH, szTempDir ) );
			Assert::AreNotEqual( (UINT)0, ::GetTempFileNameW( szTempDir, L"gi", 0, szPath ) );

			// UTF-8 (BOM 付き、CRLF)。"日本語/" を含む
			const char content[] = "\xEF\xBB\xBF*.o\r\n\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E/\r\n";
			FILE* fp = NULL;
			Assert::AreEqual( 0, (int)_wfopen_s( &fp, szPath, L"wb" ) );
			fwrite( content, 1, sizeof(content) - 1, fp );
			fclose( fp );

			CGitIgnoreFile file;
			const bool bLoaded = file.LoadFromFile( szPath );
			::DeleteFileW( szPath );

			Assert::IsTrue( bLoaded );
			Assert::AreEqual( (size_t)2, file.GetPatternCount() );
			Assert::AreEqual( (int)GITIGNORE_IGNORED, (int)file.Check( L"a.o", false ) );
			Assert::AreEqual( (int)GITIGNORE_IGNORED, (int)file.Check( L"x/\u65E5\u672C\u8A9E", true ) );
		}

		TEST_METHOD( LoadFromMissingFile )
		{
			CGitIgnoreFile file;
			Assert::IsFalse( file.LoadFromFile( L"Z:\\no\\such\\dir\\.gitignore" ) );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    適用中の .gitignore の管理                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CGitIgnoreMatcherTest )
	{
	public:
		TEST_METHOD( DeeperFileWins )
		{
			CGitIgnoreFile root;
			root.LoadFromString( L"*.log\n" );
			CGitIgnoreFile sub( L"sub" );
			sub.LoadFromString( L"!keep.log\n" );

			CGitIgnoreMatcher matcher;
			matcher.PushFile( root );
			matcher.PushFile( sub );
			Assert::IsFalse( matcher.IsIgnored( L"sub/keep.log", false ) );
			Assert::IsTrue( matcher.IsIgnored( L"sub/other.log", false ) );
			Assert::IsTrue( matcher.IsIgnored( L"keep.log", false ) );

			// sub を出ると、ルートの .gitignore だけが適用される
			matcher.PopFile();
			Assert::AreEqual( (size_t)1, matcher.GetFileCount() );
			Assert::IsTrue( matcher.IsIgnored( L"sub/keep.log", false ) );
		}

		TEST_METHOD( ShallowerAppliesWhenDeeperHasNoMatch )
		{
			CGitIgnoreFile root;
			root.LoadFromString( L"*.o\n" );
			CGitIgnoreFile sub( L"sub" );
			sub.LoadFromString( L"*.tmp\n" );

			CGitIgnoreMatcher matcher;
			matcher.PushFile( root );
			matcher.PushFile( sub );
			Assert::IsTrue( matcher.IsIgnored( L"sub/a.o", false ) );
			Assert::IsTrue( matcher.IsIgnored( L"sub/a.tmp", false ) );
			Assert::IsFalse( matcher.IsIgnored( L"sub/a.c", false ) );
		}

		TEST_METHOD( NestedRepository )
		{
			CGitIgnoreFile parent;
			parent.LoadFromString( L"*.o\n" );

			CGitIgnoreMatcher matcher;
			matcher.PushFile( parent );
			Assert::IsTrue( matcher.IsIgnored( L"a.o", false ) );

			// 入れ子のリポジトリでは、親リポジトリの .gitignore を適用しない
			matcher.BeginRepository();
			Assert::AreEqual( (size_t)0, matcher.GetFileCount() );
			Assert::IsFalse( matcher.IsIgnored( L"a.o", false ) );

			CGitIgnoreFile child;
			child.LoadFromString( L"*.tmp\n" );
			matcher.PushFile( child );
			Assert::IsTrue( matcher.IsIgnored( L"a.tmp", false ) );
			matcher.PopFile();

			// 出ると親リポジトリの .gitignore に戻る
			matcher.EndRepository();
			Assert::AreEqual( (size_t)1, matcher.GetFileCount() );
			Assert::IsTrue( matcher.IsIgnored( L"a.o", false ) );
			Assert::IsFalse( matcher.IsIgnored( L"a.tmp", false ) );
		}

		/*!
		 * SameResultAsGit - git と同じ結果になることを確認する
		 *
		 * 次の .gitignore を置いたリポジトリで `git ls-files -o --exclude-standard`
		 * (core.ignorecase=false) が列挙したファイルと、同じ結果になることを確認する。
		 * Grep と同じく、除外されたフォルダの中は列挙しない (親フォルダが除外されていれば除外)。
		 */
		TEST_METHOD( SameResultAsGit )
		{
			CGitIgnoreFile root;
			root.LoadFromString(
				L"*.log\n!keep.log\nbuild/\n/root-only.txt\ndoc/**/*.tmp\n**/cache\n\\#hash\n"
				L"a[0-9].txt\nout/\n!out/important.txt\n# comment\n*.bak   \nfoo\\ \n" );
			CGitIgnoreFile sub2( L"sub2" );
			sub2.LoadFromString( L"!*.bak\n/local.txt\n" );

			CGitIgnoreMatcher matcher;
			matcher.PushFile( root );
			matcher.PushFile( sub2 );

			struct { const wchar_t* path; bool visible; } cases[] = {
				{ L".gitignore",		true  },
				{ L"a.log",				false },
				{ L"keep.log",			true  },
				{ L"src/b.log",			false },
				{ L"root-only.txt",		false },
				{ L"src/root-only.txt",	true  },
				{ L"doc/a.tmp",			false },
				{ L"doc/x/b.tmp",		false },
				{ L"doc/x/y/c.tmp",		false },
				{ L"src/d.tmp",			true  },
				{ L"#hash",				false },
				{ L"src/#hash",			false },
				{ L"a1.txt",			false },
				{ L"a10.txt",			true  },
				{ L"ab.txt",			true  },
				{ L"out/important.txt",	false },	// 除外されたフォルダの中は含められない
				{ L"out/x.txt",			false },
				{ L"build.txt",			true  },
				{ L"src/build/x.c",		false },
				{ L"src/sub/y.c",		true  },
				{ L"cache/z.c",			false },
				{ L"src/cache/w.c",		false },
				{ L"x.bak",				false },
				{ L"sub2/.gitignore",	true  },
				{ L"sub2/x.bak",		true  },	// 下位の .gitignore で含める
				{ L"sub2/local.txt",	false },
				{ L"local.txt",			true  },
				{ L"foo ",				false },
				{ L"foo",				true  },
				{ L"src/main.c",		true  },
			};
			for( size_t i = 0; i < _countof(cases); ++i ){
				const std::wstring path = cases[i].path;
				bool bVisible = true;
				for( std::wstring::size_type pos = path.find( L'/' ); pos != std::wstring::npos; pos = path.find( L'/', pos + 1 ) ){
					if( matcher.IsIgnored( path.substr( 0, pos ), true ) ){
						bVisible = false;
						break;
					}
				}
				if( bVisible && matcher.IsIgnored( path, false ) ){
					bVisible = false;
				}
				Assert::AreEqual( cases[i].visible, bVisible, cases[i].path );
			}
		}

		TEST_METHOD( UnbalancedPopIsSafe )
		{
			CGitIgnoreMatcher matcher;
			matcher.PopFile();
			matcher.EndRepository();
			Assert::AreEqual( (size_t)0, matcher.GetFileCount() );
			Assert::IsFalse( matcher.IsIgnored( L"a.o", false ) );
		}
	};
}
