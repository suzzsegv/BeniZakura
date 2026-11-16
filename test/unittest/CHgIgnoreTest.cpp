/*!
 *	@file
 *	@brief	CHgIgnore (.hgignore の除外指定の解釈) の単体テスト
 *
 * 期待値の一部は、Mercurial 6.1.3 の `hg debugignore` (正規表現への変換) と
 * `hg status -i` (除外されるファイル) の結果による。
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
#include <string>
#include <vector>
#include "CppUnitTest.h"
#include "util/CHgIgnore.h"
#include "TestTempDir.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeniZakuraUnitTest
{
	/*!
	 * IsHgVisible - パスが Grep の検索対象になるかを返す (親フォルダが除外されていれば除外)
	 *
	 *	@return true: 検索対象 / false: 除外される
	 */
	static bool IsHgVisible(
		const CHgIgnoreFile&	cFile,		//!< [in] .hgignore
		const std::wstring&		strPath		//!< [in] ルートからの相対パス ('/' 区切り)
	)
	{
		for( std::wstring::size_type pos = strPath.find( L'/' ); pos != std::wstring::npos; pos = strPath.find( L'/', pos + 1 ) ){
			if( cFile.IsIgnored( strPath.substr( 0, pos ) ) ){
				return false;
			}
		}
		return !cFile.IsIgnored( strPath );
	}

	/*!
	 * LoadHgIgnore - .hgignore の内容を読み込んだ CHgIgnoreFile を作る (include: を使わない場合用)
	 */
	static void LoadHgIgnore(
		CHgIgnoreFile*	pcFile,		//!< [out] 読み込み先
		const wchar_t*	pszContent	//!< [in] .hgignore の内容
	)
	{
		pcFile->LoadFromString( L"C:\\nonexistent-hg-root", pszContent );
	}

	/*!
	 * WarningMessages - 警告を取り出し、"行番号:内容" を ';' 区切りで連結した文字列にする
	 *
	 *	@return 連結した文字列
	 */
	static std::wstring WarningMessages(
		CHgIgnoreFile*	pcFile		//!< [i/o] 警告を取り出す .hgignore
	)
	{
		std::vector<SVcsIgnoreWarning> warnings;
		pcFile->TakeWarnings( &warnings );
		std::wstring s;
		for( size_t i = 0; i < warnings.size(); ++i ){
			if( i ){
				s += L";";
			}
			s += std::to_wstring( warnings[i].nLine ) + L":" + warnings[i].strMessage;
		}
		return s;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     正規表現への変換                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CHgIgnorePatternToRegexTest )
	{
	public:
		/*!
		 * SameAsHgDebugIgnore - hg debugignore が表示した変換結果と同じになることを確認する
		 */
		TEST_METHOD( SameAsHgDebugIgnore )
		{
			struct { EHgIgnoreSyntax eSyntax; const wchar_t* pattern; const wchar_t* expected; } cases[] = {
				{ HGIGNORE_REGEXP,		L"\\.log$",			L".*\\.log$" },
				{ HGIGNORE_REGEXP,		L"^root\\.txt$",	L"^root\\.txt$" },
				{ HGIGNORE_REGEXP,		L"foo",				L".*foo" },
				{ HGIGNORE_GLOB,		L"*.o",				L".*\\.o(?:/|$)" },
				{ HGIGNORE_GLOB,		L"build",			L"(?:|.*/)build(?:/|$)" },
				{ HGIGNORE_GLOB,		L"doc/*.tmp",		L"(?:|.*/)doc/[^/]*\\.tmp(?:/|$)" },
				{ HGIGNORE_GLOB,		L"lib/**.a",		L"(?:|.*/)lib/.*\\.a(?:/|$)" },
				{ HGIGNORE_GLOB,		L"{x,y}.dat",		L"(?:|.*/)(?:x|y)\\.dat(?:/|$)" },
				{ HGIGNORE_ROOTGLOB,	L"top.c",			L"top\\.c(?:/|$)" },
				{ HGIGNORE_GLOB,		L"**/cache",		L"(?:|.*/)(?:.*/)?cache(?:/|$)" },
				{ HGIGNORE_GLOB,		L"a?c",				L"(?:|.*/)a.c(?:/|$)" },
				{ HGIGNORE_GLOB,		L"[!x]y.txt",		L"(?:|.*/)[^x]y\\.txt(?:/|$)" },
				{ HGIGNORE_GLOB,		L"fo\\*o",			L"(?:|.*/)fo/[^/]*o(?:/|$)" },	// Windows では '\\' は '/'
				{ HGIGNORE_GLOB,		L"r*.md",			L"(?:|.*/)r[^/]*\\.md(?:/|$)" },
				{ HGIGNORE_GLOB,		L"*.bak",			L".*\\.bak(?:/|$)" },
				// glob / rootglob はパスとして正規化する (末尾の '/' も取り除く)
				{ HGIGNORE_GLOB,		L"build/",			L"(?:|.*/)build(?:/|$)" },
				{ HGIGNORE_GLOB,		L"a//b",			L"(?:|.*/)a/b(?:/|$)" },
				{ HGIGNORE_GLOB,		L"./c",				L"(?:|.*/)c(?:/|$)" },
				{ HGIGNORE_GLOB,		L"d/./e",			L"(?:|.*/)d/e(?:/|$)" },
				{ HGIGNORE_GLOB,		L"f/../g",			L"(?:|.*/)g(?:/|$)" },
				{ HGIGNORE_GLOB,		L"*.o/",			L".*\\.o(?:/|$)" },
				{ HGIGNORE_GLOB,		L"../x",			L"(?:|.*/)\\.\\./x(?:/|$)" },
				{ HGIGNORE_GLOB,		L"x/..",			L"(?:|.*/)\\.(?:/|$)" },
				{ HGIGNORE_GLOB,		L".",				L"(?:|.*/)\\.(?:/|$)" },
				{ HGIGNORE_GLOB,		L"./",				L"(?:|.*/)\\.(?:/|$)" },
				{ HGIGNORE_GLOB,		L"//a",				L"(?:|.*/)//a(?:/|$)" },
				{ HGIGNORE_GLOB,		L"///a",			L"(?:|.*/)///a(?:/|$)" },
				{ HGIGNORE_GLOB,		L"/a",				L"(?:|.*/)/a(?:/|$)" },
				{ HGIGNORE_GLOB,		L"a/b/../../c",		L"(?:|.*/)c(?:/|$)" },
				{ HGIGNORE_GLOB,		L"a/../../b",		L"(?:|.*/)\\.\\./b(?:/|$)" },
				{ HGIGNORE_GLOB,		L"c:x",				L"(?:|.*/)c:x(?:/|$)" },
				{ HGIGNORE_GLOB,		L"c:/x/",			L"(?:|.*/)c:/x(?:/|$)" },
				{ HGIGNORE_GLOB,		L"\\a",				L"(?:|.*/)/a(?:/|$)" },
				{ HGIGNORE_GLOB,		L"x\\y/",			L"(?:|.*/)x/y(?:/|$)" },
				{ HGIGNORE_ROOTGLOB,	L"r//s/",			L"r/s(?:/|$)" },
				{ HGIGNORE_ROOTGLOB,	L"out/./*.o",		L"out/[^/]*\\.o(?:/|$)" },
				{ HGIGNORE_ROOTGLOB,	L"../y",			L"\\.\\./y(?:/|$)" },
				{ HGIGNORE_ROOTGLOB,	L"q/..",			L"\\.(?:/|$)" },
				// regexp は正規化しない
				{ HGIGNORE_REGEXP,		L"re/",				L".*re/" },
			};
			for( size_t i = 0; i < _countof(cases); ++i ){
				Assert::AreEqual( std::wstring( cases[i].expected ), CHgIgnoreFile::PatternToRegex( cases[i].eSyntax, cases[i].pattern ), cases[i].pattern );
			}
		}

		TEST_METHOD( GlobCharClass )
		{
			Assert::AreEqual( std::wstring( L"[a-z]" ), CHgIgnoreFile::GlobToRegex( L"[a-z]" ) );
			Assert::AreEqual( std::wstring( L"[\\]a]" ), CHgIgnoreFile::GlobToRegex( L"[]a]" ) );
			Assert::AreEqual( std::wstring( L"[\\^a]" ), CHgIgnoreFile::GlobToRegex( L"[^a]" ) );
			Assert::AreEqual( std::wstring( L"\\[abc" ), CHgIgnoreFile::GlobToRegex( L"[abc" ) );	// 閉じていない '['
			Assert::AreEqual( std::wstring( L"a\\.b\\+c\\(d\\)" ), CHgIgnoreFile::GlobToRegex( L"a.b+c(d)" ) );
			Assert::AreEqual( std::wstring( L"a\\}b,c" ), CHgIgnoreFile::GlobToRegex( L"a}b,c" ) );	// グループの外の '}' と ','
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          照合                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CHgIgnoreMatchTest )
	{
	public:
		TEST_METHOD( RegexpMatchesAnywhere )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"\\.log$\n^gen/\nfoo\n" );
			Assert::IsTrue( cFile.IsIgnored( L"a.log" ) );
			Assert::IsTrue( cFile.IsIgnored( L"x/y/a.log" ) );
			Assert::IsFalse( cFile.IsIgnored( L"a.log.txt" ) );
			Assert::IsTrue( cFile.IsIgnored( L"gen/a.c" ) );
			Assert::IsFalse( cFile.IsIgnored( L"src/gen/a.c" ) );
			Assert::IsTrue( cFile.IsIgnored( L"afoo/z/k.c" ) );		// パスの途中に一致すればよい
			// 英大文字と小文字を区別する
			Assert::IsFalse( cFile.IsIgnored( L"A.LOG" ) );
		}

		TEST_METHOD( GlobMatchesAnyDepthAndInside )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"syntax: glob\n*.o\nbuild\ndoc/*.tmp\na?c\n" );
			Assert::IsTrue( cFile.IsIgnored( L"x.o" ) );
			Assert::IsTrue( cFile.IsIgnored( L"src/x.o" ) );
			Assert::IsTrue( cFile.IsIgnored( L"build" ) );
			Assert::IsTrue( cFile.IsIgnored( L"build/x.c" ) );		// 一致したフォルダの中
			Assert::IsTrue( cFile.IsIgnored( L"src/build/x.c" ) );
			Assert::IsFalse( cFile.IsIgnored( L"build.txt" ) );
			Assert::IsTrue( cFile.IsIgnored( L"doc/a.tmp" ) );
			Assert::IsTrue( cFile.IsIgnored( L"x/doc/a.tmp" ) );	// '/' を含んでも、どの階層からでも一致
			Assert::IsFalse( cFile.IsIgnored( L"doc/x/a.tmp" ) );
			Assert::IsTrue( cFile.IsIgnored( L"a/c" ) );			// '?' は '/' にも一致する
		}

		/*!
		 * GlobTrailingSlash - glob の末尾の '/' はフォルダのみの指定にならないことを確認する
		 *
		 * hg status -i の結果: build/a.txt, logs, sub/build/b.txt, sub2/build, d/logs/c.txt が除外される。
		 * (末尾の '/' は正規化で取り除かれ、同じ名前のファイルにも一致する)
		 */
		TEST_METHOD( GlobTrailingSlash )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"syntax: glob\nbuild/\nlogs/\n" );
			Assert::IsTrue( cFile.IsIgnored( L"build" ) );
			Assert::IsTrue( cFile.IsIgnored( L"build/a.txt" ) );
			Assert::IsTrue( cFile.IsIgnored( L"sub/build/b.txt" ) );
			Assert::IsTrue( cFile.IsIgnored( L"sub2/build" ) );		// ファイル build
			Assert::IsTrue( cFile.IsIgnored( L"logs" ) );			// ファイル logs
			Assert::IsTrue( cFile.IsIgnored( L"d/logs/c.txt" ) );
			Assert::IsFalse( cFile.IsIgnored( L"keep.txt" ) );
		}

		TEST_METHOD( RootGlob )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"rootglob:top.c\nsyntax: rootglob\nout/*.o\n" );
			Assert::IsTrue( cFile.IsIgnored( L"top.c" ) );
			Assert::IsFalse( cFile.IsIgnored( L"src/top.c" ) );
			Assert::IsTrue( cFile.IsIgnored( L"out/a.o" ) );
			Assert::IsFalse( cFile.IsIgnored( L"x/out/a.o" ) );
		}

		TEST_METHOD( CommentsAndSpaces )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"# comment\nsyntax: glob   # trailing comment\n*.bak # comment\nkeep\\#hash\nwith space   \n\\\\#x\n" );
			Assert::AreEqual( (size_t)4, cFile.GetPatternCount() );
			Assert::IsTrue( cFile.IsIgnored( L"a.bak" ) );
			Assert::IsTrue( cFile.IsIgnored( L"keep#hash" ) );
			Assert::IsTrue( cFile.IsIgnored( L"with space" ) );
			Assert::IsFalse( cFile.IsIgnored( L"with space   " ) );
			Assert::AreEqual( std::wstring(), WarningMessages( &cFile ) );
		}

		TEST_METHOD( LinePrefixOverridesSyntax )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"syntax: glob\nre:^gen/\nrelre:tmp$\nregexp:zz\nglob:*.bak\nrelglob:r*.md\npath:abc\n" );
			Assert::IsTrue( cFile.IsIgnored( L"gen/a" ) );
			Assert::IsTrue( cFile.IsIgnored( L"x/a.tmp" ) );
			Assert::IsTrue( cFile.IsIgnored( L"azz" ) );
			Assert::IsTrue( cFile.IsIgnored( L"a.bak" ) );
			Assert::IsTrue( cFile.IsIgnored( L"x/r1.md" ) );
			// path: は .hgignore では書式指定にならない (glob のパターン "path:abc" として扱う)
			Assert::IsTrue( cFile.IsIgnored( L"path:abc" ) );
			Assert::IsFalse( cFile.IsIgnored( L"abc" ) );
		}

		TEST_METHOD( UnknownSyntaxKeepsPrevious )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"syntax: glob\n*.o\nsyntax: foo\n*.bak\n" );
			Assert::IsTrue( cFile.IsIgnored( L"a.bak" ) );		// glob のまま
			Assert::AreEqual( std::wstring( L"3:不明な書式 'foo' を無視" ), WarningMessages( &cFile ) );
			Assert::AreEqual( std::wstring(), WarningMessages( &cFile ) );	// 取り出した警告は消える
		}

		TEST_METHOD( InvalidRegexIsSkipped )
		{
			// std::wregex で解釈できないパターンは無視し、他のパターンは適用する
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"(unclosed\n(?i)\\.bak$\n\\.log$\n" );
			Assert::AreEqual( (size_t)1, cFile.GetPatternCount() );
			Assert::IsTrue( cFile.IsIgnored( L"a.log" ) );
			Assert::IsFalse( cFile.IsIgnored( L"a.bak" ) );
			Assert::AreEqual(
				std::wstring( L"1:正規表現として解釈できないため無視: (unclosed;2:正規表現として解釈できないため無視: (?i)\\.bak$" ),
				WarningMessages( &cFile ) );
		}

		TEST_METHOD( EmptyIgnoresNothing )
		{
			CHgIgnoreFile cFile;
			LoadHgIgnore( &cFile, L"" );
			Assert::AreEqual( (size_t)0, cFile.GetPatternCount() );
			Assert::IsFalse( cFile.IsIgnored( L"a.o" ) );
		}
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                ファイルの読み込み (include など)              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	TEST_CLASS( CHgIgnoreFileTest )
	{
	public:
		TEST_METHOD( LoadFromFileUtf8 )
		{
			CTestTempDir cDir;
			// UTF-8 (BOM 付き、CRLF)。"日本/" を含む
			cDir.MakeFile( L".hgignore", "\xEF\xBB\xBFsyntax: glob\r\n*.o\r\n\xE6\x97\xA5\xE6\x9C\xAC\r\n" );
			CHgIgnoreFile cFile;
			Assert::IsTrue( cFile.LoadFromFile( cDir.GetRootPath() ) );
			Assert::AreEqual( (size_t)2, cFile.GetPatternCount() );
			Assert::IsTrue( cFile.IsIgnored( L"a.o" ) );
			Assert::IsTrue( cFile.IsIgnored( L"x/\u65E5\u672C/a.c" ) );
		}

		TEST_METHOD( LoadFromMissingFile )
		{
			CTestTempDir cDir;
			CHgIgnoreFile cFile;
			Assert::IsFalse( cFile.LoadFromFile( cDir.GetRootPath() ) );
			Assert::IsFalse( cFile.IsIgnored( L"a.o" ) );
		}

		/*!
		 * IncludeAndSubinclude - include: / subinclude: の適用範囲が Mercurial と同じになることを確認する
		 *
		 * hg status -i の結果: bar, sub/a.inc, sub/deep/a.dp, sub/deep/a.inc が除外される。
		 */
		TEST_METHOD( IncludeAndSubinclude )
		{
			CTestTempDir cDir;
			cDir.MakeFile( L".hgignore", "subinclude:sub/.hgignore\nsyntax: foo\nbar\n" );
			cDir.MakeFile( L"sub\\.hgignore", "syntax: glob\ninclude:inc.txt\nsubinclude:deep/.hgignore\n/abs.txt\n" );
			cDir.MakeFile( L"sub\\inc.txt", "syntax: glob\n*.inc\n" );
			cDir.MakeFile( L"sub\\deep\\.hgignore", "syntax: glob\n*.dp\n^x\n" );
			CHgIgnoreFile cFile;
			Assert::IsTrue( cFile.LoadFromFile( cDir.GetRootPath() ) );

			struct { const wchar_t* path; bool ignored; } cases[] = {
				{ L"bar",				true  },
				{ L"a.inc",				false },
				{ L"sub/a.inc",			true  },	// include: は読み込み元 (sub/.hgignore) と同じ範囲
				{ L"sub/deep/a.inc",	true  },
				{ L"other/a.inc",		false },
				{ L"a.dp",				false },
				{ L"sub/a.dp",			false },
				{ L"sub/deep/a.dp",		true  },	// subinclude: の中の subinclude:
				{ L"sub/abs.txt",		false },	// glob の先頭の '/' は文字そのもの
			};
			for( size_t i = 0; i < _countof(cases); ++i ){
				Assert::AreEqual( cases[i].ignored, cFile.IsIgnored( cases[i].path ), cases[i].path );
			}
			Assert::AreEqual( std::wstring( L"2:不明な書式 'foo' を無視" ), WarningMessages( &cFile ) );
		}

		/*!
		 * IncludeBaseDir - include: のパスの基準フォルダが Mercurial と同じになることを確認する
		 *
		 * include: のパスは、その行を書いたファイルのフォルダではなく、適用範囲のフォルダ
		 * (ルート、または subinclude: で読み込んだファイルのフォルダ) からの相対パスとなる。
		 * hg status -i の結果: root_b, sub/sub_inc2, sub/x/sub_inc2 が除外される。
		 */
		TEST_METHOD( IncludeBaseDir )
		{
			CTestTempDir cDir;
			cDir.MakeFile( L".hgignore", "include:conf/a.txt\nsubinclude:sub/.hgignore\n" );
			cDir.MakeFile( L"conf\\a.txt", "include:b.txt\n" );
			cDir.MakeFile( L"b.txt", "syntax: glob\nroot_b\n" );
			cDir.MakeFile( L"conf\\b.txt", "syntax: glob\nconf_b\n" );
			cDir.MakeFile( L"sub\\.hgignore", "include:x/inc1.txt\n" );
			cDir.MakeFile( L"sub\\x\\inc1.txt", "include:inc2.txt\n" );
			cDir.MakeFile( L"sub\\inc2.txt", "syntax: glob\nsub_inc2\n" );
			cDir.MakeFile( L"sub\\x\\inc2.txt", "syntax: glob\nsubx_inc2\n" );
			CHgIgnoreFile cFile;
			Assert::IsTrue( cFile.LoadFromFile( cDir.GetRootPath() ) );

			struct { const wchar_t* path; bool ignored; } cases[] = {
				{ L"root_b",			true  },	// ルートの b.txt (conf/b.txt ではない)
				{ L"conf_b",			false },
				{ L"sub/sub_inc2",		true  },	// sub/inc2.txt (sub/x/inc2.txt ではない)
				{ L"sub/x/sub_inc2",	true  },
				{ L"sub/subx_inc2",		false },
			};
			for( size_t i = 0; i < _countof(cases); ++i ){
				Assert::AreEqual( cases[i].ignored, cFile.IsIgnored( cases[i].path ), cases[i].path );
			}
			Assert::AreEqual( std::wstring(), WarningMessages( &cFile ) );
		}

		TEST_METHOD( IncludeWarnings )
		{
			CTestTempDir cDir;
			cDir.MakeFile( L".hgignore", "include:a.txt\ninclude:missing.txt\nsubinclude:../outside/.hgignore\n" );
			cDir.MakeFile( L"a.txt", "include:b.txt\n" );
			cDir.MakeFile( L"b.txt", "include:a.txt\n\\.o$\n" );
			CHgIgnoreFile cFile;
			Assert::IsTrue( cFile.LoadFromFile( cDir.GetRootPath() ) );
			Assert::IsTrue( cFile.IsIgnored( L"x.o" ) );
			Assert::AreEqual(
				std::wstring( L"1:循環して読み込まれるファイル 'a.txt' を無視;2:読み込めないファイル 'missing.txt' を無視;3:リポジトリの外のファイル '../outside/.hgignore' を無視" ),
				WarningMessages( &cFile ) );
		}

		/*!
		 * SameResultAsHg - hg status -i と同じ結果になることを確認する
		 *
		 * 次の .hgignore を置いたリポジトリで hg status -i が除外したファイルと、同じ結果になることを確認する。
		 * Grep と同じく、親フォルダが除外されていれば除外とする。
		 */
		TEST_METHOD( SameResultAsHg )
		{
			CTestTempDir cDir;
			cDir.MakeFile( L".hgignore",
				"# default regexp\n\\.log$\n^root\\.txt$\nfoo\nsyntax: glob\n*.o\nbuild\ndoc/*.tmp\nlib/**.a\n"
				"{x,y}.dat\nrootglob:top.c\nCASE.TXT\nsubinclude:sub/.hgignore\n" );
			cDir.MakeFile( L"sub\\.hgignore", "syntax: glob\n*.bak\nlocal.txt\n" );
			CHgIgnoreFile cFile;
			Assert::IsTrue( cFile.LoadFromFile( cDir.GetRootPath() ) );

			struct { const wchar_t* path; bool visible; } cases[] = {
				{ L".hgignore",				true  },
				{ L"CASE.TXT2",				true  },
				{ L"a.bak",					true  },
				{ L"a.log",					false },
				{ L"afoo/z/k.c",			false },
				{ L"build",					false },
				{ L"case.txt",				true  },
				{ L"doc/a.tmp",				false },
				{ L"doc/x/b.tmp",			true  },
				{ L"foobar.c",				false },
				{ L"keep.c",				true  },
				{ L"lib/a.a",				false },
				{ L"lib/m/b.a",				false },
				{ L"local.txt",				true  },
				{ L"root.txt",				false },
				{ L"src/a.o",				false },
				{ L"src/build/b.c",			false },
				{ L"src/root.txt",			true  },
				{ L"src/top.c",				true  },
				{ L"sub/.hgignore",			true  },
				{ L"sub/a.bak",				false },
				{ L"sub/deep/local.txt",	false },
				{ L"sub/local.txt",			false },
				{ L"top.c",					false },
				{ L"x.dat",					false },
				{ L"x.log.txt",				true  },
				{ L"y.dat",					false },
				{ L"z.dat",					true  },
			};
			for( size_t i = 0; i < _countof(cases); ++i ){
				Assert::AreEqual( cases[i].visible, IsHgVisible( cFile, cases[i].path ), cases[i].path );
			}
		}
	};
}
