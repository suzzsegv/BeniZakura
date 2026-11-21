/*!
 *	@file
 *	@brief	CColorTheme (カラーテーマの定義と適用) の単体テスト
 *
 * テーマの適用で、テーマが決める値 (文字色・背景色・太字・下線) だけが変わり、「色分け/表示」が変わらないこと、
 * テーマの切り替え時の「色分け/表示」の適用 (5 項目はテーマの値、それ以外はテーマとタイプの初期値から決めること、
 * 適用が必要かの判定、プリセットの 5 項目以外の値が共有データの初期値と同じこと)、
 * 色以外の表示の設定 (TAB 表示、矢印表示、カーソル形状) の初期値と適用、
 * テーマの名前と番号の変換、移行処理を確認する。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/18 新規作成
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
#include "CppUnitTest.h"
#include "doc/CColorTheme.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeniZakuraUnitTest
{
	/*!
	 * AssertThemeColorsEqual - テーマが決める値 (文字色・背景色・太字・下線) が等しいことを確認する
	 */
	static void AssertThemeColorsEqual(
		const ColorInfo*	pExpected,	//!< [in] 期待する色設定 (COLORIDX_LAST 個)
		const ColorInfo*	pActual		//!< [in] 確認する色設定 (COLORIDX_LAST 個)
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			const std::wstring strItem = L"item " + std::to_wstring( i );
			Assert::AreEqual( (unsigned long)pExpected[i].m_colTEXT, (unsigned long)pActual[i].m_colTEXT, strItem.c_str() );
			Assert::AreEqual( (unsigned long)pExpected[i].m_colBACK, (unsigned long)pActual[i].m_colBACK, strItem.c_str() );
			Assert::AreEqual( pExpected[i].m_bBoldFont, pActual[i].m_bBoldFont, strItem.c_str() );
			Assert::AreEqual( pExpected[i].m_bUnderLine, pActual[i].m_bUnderLine, strItem.c_str() );
		}
	}

	/*!
	 * AssertThemeDispEqual - テーマも持つ 5 項目の「色分け/表示」が等しいことを確認する
	 */
	static void AssertThemeDispEqual(
		const ColorInfo*	pExpected,	//!< [in] 期待する色設定 (COLORIDX_LAST 個)
		const ColorInfo*	pActual		//!< [in] 確認する色設定 (COLORIDX_LAST 個)
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			if( IsColorThemeDispItem( i ) ){
				const std::wstring strItem = L"item " + std::to_wstring( i );
				Assert::AreEqual( pExpected[i].m_bDisp, pActual[i].m_bDisp, strItem.c_str() );
			}
		}
	}

	/*!
	 * SetDispPattern - 「色分け/表示」を、項目ごとに交互の値にする (テーマの値と区別するため)
	 */
	static void SetDispPattern(
		ColorInfo*	pColorInfoArr,	//!< [i/o] 色設定 (COLORIDX_LAST 個)
		bool		bThemeDispItem	//!< [in] true: テーマも持つ 5 項目も設定する / false: 5 項目以外だけ設定する
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			if( bThemeDispItem || !IsColorThemeDispItem( i ) ){
				pColorInfoArr[i].m_bDisp = ( i % 2 == 0 );
			}
		}
	}

	/*!
	 * AssertDispPattern - 「色分け/表示」が、SetDispPattern() の値のままであることを確認する
	 */
	static void AssertDispPattern(
		const ColorInfo*	pColorInfoArr,	//!< [in] 色設定 (COLORIDX_LAST 個)
		bool				bThemeDispItem	//!< [in] true: テーマも持つ 5 項目も確認する / false: 5 項目以外だけ確認する
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			if( bThemeDispItem || !IsColorThemeDispItem( i ) ){
				const std::wstring strItem = L"item " + std::to_wstring( i );
				Assert::AreEqual( ( i % 2 == 0 ), pColorInfoArr[i].m_bDisp, strItem.c_str() );
			}
		}
	}

	TEST_CLASS( CColorThemeApplyTest )
	{
	public:
		/*!
		 * ApplyPreset - プリセットを適用すると、文字色・背景色・太字・下線だけがプリセットの値になる
		 *
		 * 「色分け/表示」は、テーマも持つ 5 項目を含めて変わらない。
		 */
		TEST_METHOD( ApplyPreset )
		{
			for( int nTheme = 0; nTheme < COLOR_THEME_MAX; nTheme++ ){
				ColorInfo arr[COLORIDX_LAST];
				SetPresetColorInfo( arr, COLOR_THEME_BENIZAKURA );
				SetDispPattern( arr, true );

				ApplyColorTheme( arr, nTheme, NULL );

				ColorInfo expected[COLORIDX_LAST];
				SetPresetColorInfo( expected, nTheme );
				AssertThemeColorsEqual( expected, arr );
				AssertDispPattern( arr, true );
				// 名前とインデックスは変わらない
				Assert::AreEqual( std::wstring( expected[COLORIDX_TEXT].m_szName ), std::wstring( arr[COLORIDX_TEXT].m_szName ) );
				Assert::AreEqual( (int)COLORIDX_RULER, arr[COLORIDX_RULER].m_nColorIdx );
			}
		}

		/*!
		 * ApplyCustom - カスタムを適用すると、文字色・背景色・太字・下線だけがカスタムの色になる
		 */
		TEST_METHOD( ApplyCustom )
		{
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_MONOKAI );
			custom[COLORIDX_TEXT].m_colTEXT = RGB( 0x12, 0x34, 0x56 );
			custom[COLORIDX_COMMENT].m_bBoldFont = !custom[COLORIDX_COMMENT].m_bBoldFont;
			custom[COLORIDX_TAB].m_bDisp = !custom[COLORIDX_TAB].m_bDisp;			// テーマも持つ項目 (ApplyColorTheme では使われない)
			custom[COLORIDX_SSTRING].m_bDisp = !custom[COLORIDX_SSTRING].m_bDisp;	// タイプだけが持つ項目 (使われない)

			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			SetDispPattern( arr, true );

			ApplyColorTheme( arr, COLOR_THEME_CUSTOM, custom );

			AssertThemeColorsEqual( custom, arr );
			AssertDispPattern( arr, true );
			Assert::AreEqual( (unsigned long)RGB( 0x12, 0x34, 0x56 ), (unsigned long)arr[COLORIDX_TEXT].m_colTEXT );
		}

		/*!
		 * ApplyInvalid - カスタムの色がない場合と不明なテーマ番号は、紅桜を適用する
		 */
		TEST_METHOD( ApplyInvalid )
		{
			ColorInfo expected[COLORIDX_LAST];
			SetPresetColorInfo( expected, COLOR_THEME_BENIZAKURA );

			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ApplyColorTheme( arr, COLOR_THEME_CUSTOM, NULL );
			AssertThemeColorsEqual( expected, arr );

			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ApplyColorTheme( arr, 99, NULL );
			AssertThemeColorsEqual( expected, arr );

			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ApplyColorTheme( arr, -1, NULL );
			AssertThemeColorsEqual( expected, arr );
		}

		/*!
		 * AssertAppliedDisp - ApplyColorThemeDisp() の結果の「色分け/表示」を確認する
		 *
		 * 5 項目はテーマの値、それ以外の項目はテーマの値とタイプの初期値の両方が「表示」のときに「表示」。
		 */
		static void AssertAppliedDisp(
			const ColorInfo*	pTheme,			//!< [in] テーマの色設定 (COLORIDX_LAST 個)
			const bool*			pbTypeInitDisp,	//!< [in] タイプの「色分け/表示」の初期値 (COLORIDX_LAST 個)
			const ColorInfo*	pActual			//!< [in] 確認する色設定 (COLORIDX_LAST 個)
		)
		{
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				const std::wstring strItem = L"item " + std::to_wstring( i );
				const bool bExpected = IsColorThemeDispItem( i ) ? pTheme[i].m_bDisp : ( pTheme[i].m_bDisp && pbTypeInitDisp[i] );
				Assert::AreEqual( bExpected, pActual[i].m_bDisp, strItem.c_str() );
			}
		}

		/*!
		 * SetTypeInitDispPattern - タイプの「色分け/表示」の初期値を、項目ごとに交互の値にする
		 */
		static void SetTypeInitDispPattern(
			bool*	pbTypeInitDisp	//!< [out] タイプの「色分け/表示」の初期値 (COLORIDX_LAST 個)
		)
		{
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				pbTypeInitDisp[i] = ( i % 2 == 0 );
			}
		}

		/*!
		 * ApplyDispPreset - プリセットの「色分け/表示」を適用すると、5 項目はテーマの値、それ以外はテーマとタイプの初期値の両方が表示のときに表示
		 */
		TEST_METHOD( ApplyDispPreset )
		{
			bool abTypeInitDisp[COLORIDX_LAST];
			SetTypeInitDispPattern( abTypeInitDisp );
			for( int nTheme = 0; nTheme < COLOR_THEME_MAX; nTheme++ ){
				ColorInfo arr[COLORIDX_LAST];
				SetPresetColorInfo( arr, COLOR_THEME_BENIZAKURA );
				arr[COLORIDX_TEXT].m_colTEXT = RGB( 0x12, 0x34, 0x56 );
				for( int i = 0; i < COLORIDX_LAST; i++ ){
					arr[i].m_bDisp = ( i % 3 == 0 );	// 適用前の値 (結果に影響しない)
				}
				ColorInfo before[COLORIDX_LAST];
				memcpy( before, arr, sizeof(before) );

				ApplyColorThemeDisp( arr, nTheme, NULL, abTypeInitDisp );

				ColorInfo theme[COLORIDX_LAST];
				SetPresetColorInfo( theme, nTheme );
				AssertAppliedDisp( theme, abTypeInitDisp, arr );
				AssertThemeColorsEqual( before, arr );	// 文字色などは変わらない
			}
		}

		/*!
		 * ApplyDispCustom - カスタムの「色分け/表示」を、プリセットと同じ規則で適用する
		 */
		TEST_METHOD( ApplyDispCustom )
		{
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_MONOKAI );
			custom[COLORIDX_TEXT].m_colTEXT = RGB( 0x12, 0x34, 0x56 );					// 使われない
			custom[COLORIDX_CURSORVLINE].m_bDisp = !custom[COLORIDX_CURSORVLINE].m_bDisp;
			custom[COLORIDX_SSTRING].m_bDisp = false;
			custom[COLORIDX_COMMENT].m_bDisp = true;

			bool abTypeInitDisp[COLORIDX_LAST];
			SetTypeInitDispPattern( abTypeInitDisp );
			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ColorInfo before[COLORIDX_LAST];
			memcpy( before, arr, sizeof(before) );

			ApplyColorThemeDisp( arr, COLOR_THEME_CUSTOM, custom, abTypeInitDisp );

			AssertAppliedDisp( custom, abTypeInitDisp, arr );
			AssertThemeColorsEqual( before, arr );
		}

		/*!
		 * ApplyDispTypeInit - テーマとタイプの初期値の組み合わせごとの結果 (5 項目はタイプの初期値によらない)
		 */
		TEST_METHOD( ApplyDispTypeInit )
		{
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_BENIZAKURA );
			bool abTypeInitDisp[COLORIDX_LAST];
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				abTypeInitDisp[i] = true;
			}
			custom[COLORIDX_SSTRING].m_bDisp = true;	abTypeInitDisp[COLORIDX_SSTRING] = false;	// テキストの引用符など
			custom[COLORIDX_WSTRING].m_bDisp = false;	abTypeInitDisp[COLORIDX_WSTRING] = true;
			custom[COLORIDX_COMMENT].m_bDisp = true;	abTypeInitDisp[COLORIDX_COMMENT] = true;
			custom[COLORIDX_TAB].m_bDisp = true;		abTypeInitDisp[COLORIDX_TAB] = false;		// 5 項目
			custom[COLORIDX_EOL].m_bDisp = false;		abTypeInitDisp[COLORIDX_EOL] = true;		// 5 項目

			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_BENIZAKURA );
			ApplyColorThemeDisp( arr, COLOR_THEME_CUSTOM, custom, abTypeInitDisp );

			Assert::IsFalse( arr[COLORIDX_SSTRING].m_bDisp );
			Assert::IsFalse( arr[COLORIDX_WSTRING].m_bDisp );
			Assert::IsTrue( arr[COLORIDX_COMMENT].m_bDisp );
			Assert::IsTrue( arr[COLORIDX_TAB].m_bDisp );
			Assert::IsFalse( arr[COLORIDX_EOL].m_bDisp );
		}

		/*!
		 * ApplyDispInvalid - カスタムの色がない場合と不明なテーマ番号は、紅桜の「色分け/表示」を適用する
		 */
		TEST_METHOD( ApplyDispInvalid )
		{
			ColorInfo theme[COLORIDX_LAST];
			SetPresetColorInfo( theme, COLOR_THEME_BENIZAKURA );
			bool abTypeInitDisp[COLORIDX_LAST];
			SetTypeInitDispPattern( abTypeInitDisp );
			const int anTheme[] = { COLOR_THEME_CUSTOM, 99, -1 };
			for( int nTheme : anTheme ){
				ColorInfo arr[COLORIDX_LAST];
				SetPresetColorInfo( arr, COLOR_THEME_VS2012DARK );
				ApplyColorThemeDisp( arr, nTheme, NULL, abTypeInitDisp );
				AssertAppliedDisp( theme, abTypeInitDisp, arr );
			}
		}

		/*!
		 * PresetDispDefault - プリセットの 5 項目以外の「色分け/表示」は、共有データの初期値 (紅桜の配色) と同じ
		 */
		TEST_METHOD( PresetDispDefault )
		{
			ColorInfo def[COLORIDX_LAST];
			SetPresetColorInfo( def, COLOR_THEME_BENIZAKURA );
			for( int nTheme = 0; nTheme < COLOR_THEME_MAX; nTheme++ ){
				ColorInfo arr[COLORIDX_LAST];
				SetPresetColorInfo( arr, nTheme );
				for( int i = 0; i < COLORIDX_LAST; i++ ){
					if( !IsColorThemeDispItem( i ) ){
						const std::wstring strItem = L"theme " + std::to_wstring( nTheme ) + L" item " + std::to_wstring( i );
						Assert::AreEqual( def[i].m_bDisp, arr[i].m_bDisp, strItem.c_str() );
					}
				}
			}
		}

		/*!
		 * ApplyNeededTheme - テーマ番号が異なる場合は、テーマの切り替え時の適用が必要
		 */
		TEST_METHOD( ApplyNeededTheme )
		{
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_BENIZAKURA );
			ColorThemeView view;
			SetPresetColorThemeView( &view, COLOR_THEME_BENIZAKURA );
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_BENIZAKURA, custom, &view, COLOR_THEME_MONOKAI, custom, &view ) );
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_MONOKAI, custom, &view, COLOR_THEME_CUSTOM, custom, &view ) );
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, custom, &view, COLOR_THEME_SAKURA, custom, &view ) );
			// プリセット同士で、表示の有無が同じテーマでも、切り替えたら適用する
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_BENIZAKURA, custom, &view, COLOR_THEME_SAKURA, custom, &view ) );
		}

		/*!
		 * ApplyNeededSameTheme - テーマ番号が同じ場合は、カスタムの「色分け/表示」・TAB 表示・矢印表示・カーソル形状が異なるときだけ適用が必要
		 */
		TEST_METHOD( ApplyNeededSameTheme )
		{
			ColorInfo oldCustom[COLORIDX_LAST];
			SetPresetColorInfo( oldCustom, COLOR_THEME_BENIZAKURA );
			ColorInfo newCustom[COLORIDX_LAST];
			ColorThemeView oldView;
			SetPresetColorThemeView( &oldView, COLOR_THEME_BENIZAKURA );
			ColorThemeView newView = oldView;

			// 同じプリセット: カスタムの値が異なっても適用しない
			memcpy( newCustom, oldCustom, sizeof(newCustom) );
			newCustom[COLORIDX_TAB].m_bDisp = !newCustom[COLORIDX_TAB].m_bDisp;
			newView.m_bTabArrow = !newView.m_bTabArrow;
			newView.m_nCaretType = 1 - newView.m_nCaretType;
			Assert::IsFalse( IsColorThemeApplyNeeded( COLOR_THEME_MONOKAI, oldCustom, &oldView, COLOR_THEME_MONOKAI, newCustom, &newView ) );

			// カスタム: 何も変えていない
			memcpy( newCustom, oldCustom, sizeof(newCustom) );
			newView = oldView;
			Assert::IsFalse( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ) );

			// カスタム: 文字色・背景色・太字・下線だけが異なる
			newCustom[COLORIDX_TEXT].m_colTEXT = RGB( 0x12, 0x34, 0x56 );
			newCustom[COLORIDX_TEXT].m_colBACK = RGB( 0x65, 0x43, 0x21 );
			newCustom[COLORIDX_COMMENT].m_bBoldFont = !newCustom[COLORIDX_COMMENT].m_bBoldFont;
			newCustom[COLORIDX_COMMENT].m_bUnderLine = !newCustom[COLORIDX_COMMENT].m_bUnderLine;
			Assert::IsFalse( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ) );

			// カスタム: いずれかの項目の「色分け/表示」が異なる (5 項目とそれ以外)
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				memcpy( newCustom, oldCustom, sizeof(newCustom) );
				newCustom[i].m_bDisp = !newCustom[i].m_bDisp;
				const std::wstring strItem = L"item " + std::to_wstring( i );
				Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ), strItem.c_str() );
			}

			// カスタム: TAB 表示・矢印表示・カーソル形状のどれかが異なる
			memcpy( newCustom, oldCustom, sizeof(newCustom) );
			newView = oldView;
			newView.m_szTabViewString[0] = L'>';
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ) );
			newView = oldView;
			newView.m_bTabArrow = !newView.m_bTabArrow;
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ) );
			newView = oldView;
			newView.m_nCaretType = 1 - newView.m_nCaretType;
			Assert::IsTrue( IsColorThemeApplyNeeded( COLOR_THEME_CUSTOM, oldCustom, &oldView, COLOR_THEME_CUSTOM, newCustom, &newView ) );
		}

		/*!
		 * PresetView - プリセットの色以外の表示の設定の初期値 (TAB 表示はテーマごと、カーソル形状は紅桜のみ MS-DOS 風)
		 */
		TEST_METHOD( PresetView )
		{
			struct { int nTheme; const wchar_t* pszTab; bool bArrow; } const aExpected[] = {
				{ COLOR_THEME_BENIZAKURA,		L"o.......",	false },
				{ COLOR_THEME_SAKURA,			L"^       ",	false },
				{ COLOR_THEME_MIFES_BLUE,		L">.......",	false },
				{ COLOR_THEME_HIDEMARU_BLACK,	L">       ",	true },
				{ COLOR_THEME_MONOKAI,			L"|       ",	false },
				{ COLOR_THEME_VS2012DARK,		L"|       ",	false },
			};
			Assert::AreEqual( (size_t)COLOR_THEME_MAX, _countof(aExpected) );
			for( const auto& e : aExpected ){
				const int nTheme = e.nTheme;
				ColorThemeView view;
				SetPresetColorThemeView( &view, nTheme );
				const std::wstring strTheme = L"theme " + std::to_wstring( nTheme );
				Assert::AreEqual( std::wstring( e.pszTab ), std::wstring( view.m_szTabViewString ), strTheme.c_str() );
				Assert::AreEqual( e.bArrow, view.m_bTabArrow, strTheme.c_str() );
				Assert::AreEqual( ( nTheme == COLOR_THEME_BENIZAKURA ) ? 1 : 0, view.m_nCaretType, strTheme.c_str() );
				Assert::AreEqual( view.m_nCaretType, GetColorThemeCaretType( nTheme, NULL ), strTheme.c_str() );
			}
			// 不明なテーマ番号とカスタム (値なし) は紅桜
			ColorThemeView view;
			SetPresetColorThemeView( &view, 99 );
			Assert::AreEqual( 1, view.m_nCaretType );
			Assert::AreEqual( 1, GetColorThemeCaretType( COLOR_THEME_CUSTOM, NULL ) );
			Assert::AreEqual( 1, GetColorThemeCaretType( -1, NULL ) );
		}

		/*!
		 * ApplyView - テーマの TAB 表示・矢印表示がタイプの値になり、カスタムのカーソル形状を返す
		 */
		TEST_METHOD( ApplyView )
		{
			ColorThemeView custom;
			wcscpy_s( custom.m_szTabViewString, _countof(custom.m_szTabViewString), L">-------" );
			custom.m_bTabArrow = true;
			custom.m_nCaretType = 0;

			wchar_t szTab[8+1] = L"abc";
			bool bArrow = false;
			ApplyColorThemeView( szTab, _countof(szTab), &bArrow, COLOR_THEME_CUSTOM, &custom );
			Assert::AreEqual( std::wstring( L">-------" ), std::wstring( szTab ) );
			Assert::IsTrue( bArrow );
			Assert::AreEqual( 0, GetColorThemeCaretType( COLOR_THEME_CUSTOM, &custom ) );

			// プリセットはプリセットの初期値 (カスタムの値は使わない)
			ApplyColorThemeView( szTab, _countof(szTab), &bArrow, COLOR_THEME_MONOKAI, &custom );
			Assert::AreEqual( std::wstring( L"|       " ), std::wstring( szTab ) );
			Assert::IsFalse( bArrow );
			ApplyColorThemeView( szTab, _countof(szTab), &bArrow, COLOR_THEME_HIDEMARU_BLACK, NULL );
			Assert::AreEqual( std::wstring( L">       " ), std::wstring( szTab ) );
			Assert::IsTrue( bArrow );
			Assert::AreEqual( 0, GetColorThemeCaretType( COLOR_THEME_MONOKAI, &custom ) );
			Assert::AreEqual( 1, GetColorThemeCaretType( COLOR_THEME_BENIZAKURA, &custom ) );

			// カスタムの値がない場合は紅桜
			wcscpy_s( szTab, _countof(szTab), L"abc" );
			bArrow = true;
			ApplyColorThemeView( szTab, _countof(szTab), &bArrow, COLOR_THEME_CUSTOM, NULL );
			Assert::AreEqual( std::wstring( L"o......." ), std::wstring( szTab ) );
			Assert::IsFalse( bArrow );
		}

		/*!
		 * ThemeDispItems - テーマの値だけで「色分け/表示」を決める項目は 5 つ
		 */
		TEST_METHOD( ThemeDispItems )
		{
			int nCount = 0;
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				if( IsColorThemeDispItem( i ) ){
					nCount++;
				}
			}
			Assert::AreEqual( 5, nCount );
			Assert::IsTrue( IsColorThemeDispItem( COLORIDX_CARETLINEBG ) );
			Assert::IsTrue( IsColorThemeDispItem( COLORIDX_UNDERLINE ) );
			Assert::IsTrue( IsColorThemeDispItem( COLORIDX_CURSORVLINE ) );
			Assert::IsTrue( IsColorThemeDispItem( COLORIDX_TAB ) );
			Assert::IsTrue( IsColorThemeDispItem( COLORIDX_EOL ) );
			Assert::IsFalse( IsColorThemeDispItem( COLORIDX_COMMENT ) );
		}

		/*!
		 * PresetNames - プリセットの配色は、すべての項目に名前とインデックスを設定する
		 */
		TEST_METHOD( PresetNames )
		{
			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_VS2012DARK );
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				Assert::AreEqual( i, arr[i].m_nColorIdx );
				Assert::IsTrue( arr[i].m_szName[0] != L'\0' );
			}
			Assert::AreEqual( std::wstring( L"テキスト" ), std::wstring( arr[COLORIDX_TEXT].m_szName ) );
		}

		/*!
		 * MigrateFromType - テーマの設定がない ini の移行で、タイプの色がカスタムの色になる
		 *
		 * 5 項目以外の「色分け/表示」は、タイプの値ではなく共有データの初期値 (紅桜の配色の値) になる。
		 */
		TEST_METHOD( MigrateFromType )
		{
			ColorInfo type0[COLORIDX_LAST];
			SetPresetColorInfo( type0, COLOR_THEME_MIFES_BLUE );
			type0[COLORIDX_TEXT].m_colBACK = RGB( 0x01, 0x02, 0x03 );
			SetDispPattern( type0, true );

			int nColorTheme = COLOR_THEME_BENIZAKURA;
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_BENIZAKURA );
			SetCustomColorThemeFromType( &nColorTheme, custom, type0 );

			Assert::AreEqual( COLOR_THEME_CUSTOM, nColorTheme );
			AssertThemeColorsEqual( type0, custom );
			AssertThemeDispEqual( type0, custom );
			ColorInfo def[COLORIDX_LAST];
			SetPresetColorInfo( def, COLOR_THEME_BENIZAKURA );
			int nDiffFromType = 0;
			for( int i = 0; i < COLORIDX_LAST; i++ ){
				if( !IsColorThemeDispItem( i ) ){
					const std::wstring strItem = L"item " + std::to_wstring( i );
					Assert::AreEqual( def[i].m_bDisp, custom[i].m_bDisp, strItem.c_str() );
					if( def[i].m_bDisp != type0[i].m_bDisp ){
						nDiffFromType++;
					}
				}
			}
			Assert::IsTrue( 0 < nDiffFromType );	// タイプの値と異なる項目があり、タイプの値を使っていないことを確認できる

			// 移行後にテーマを適用しても、タイプの色と「色分け/表示」は変わらない
			ColorInfo applied[COLORIDX_LAST];
			memcpy( applied, type0, sizeof(applied) );
			ApplyColorTheme( applied, nColorTheme, custom );
			AssertThemeColorsEqual( type0, applied );
			AssertDispPattern( applied, true );
		}
	};

	TEST_CLASS( CColorThemeNameTest )
	{
	public:
		/*!
		 * SelIndexOrder - 選択肢の表示順 (紅桜、サクラエディタ、秀丸エディタ Black、MIFES Blue、Monokai、VS2012 Dark、カスタム)
		 */
		TEST_METHOD( SelIndexOrder )
		{
			const int expected[COLOR_THEME_SEL_COUNT] = {
				COLOR_THEME_BENIZAKURA, COLOR_THEME_SAKURA, COLOR_THEME_HIDEMARU_BLACK, COLOR_THEME_MIFES_BLUE,
				COLOR_THEME_MONOKAI, COLOR_THEME_VS2012DARK, COLOR_THEME_CUSTOM,
			};
			for( int i = 0; i < COLOR_THEME_SEL_COUNT; i++ ){
				Assert::AreEqual( expected[i], GetColorThemeBySelIndex( i ) );
				Assert::AreEqual( i, GetColorThemeSelIndex( expected[i] ) );
			}
			Assert::AreEqual( std::wstring( L"カスタム" ), std::wstring( GetColorThemeDispName( COLOR_THEME_CUSTOM ) ) );
			Assert::AreEqual( (int)COLOR_THEME_BENIZAKURA, GetColorThemeBySelIndex( COLOR_THEME_SEL_COUNT ) );
			Assert::AreEqual( 0, GetColorThemeSelIndex( 99 ) );
		}

		/*!
		 * KeyNameRoundTrip - ini に保存する名前と、テーマ番号の変換
		 */
		TEST_METHOD( KeyNameRoundTrip )
		{
			for( int i = 0; i < COLOR_THEME_SEL_COUNT; i++ ){
				const int nColorTheme = GetColorThemeBySelIndex( i );
				Assert::AreEqual( nColorTheme, GetColorThemeFromKeyName( GetColorThemeKeyName( nColorTheme ) ) );
			}
			Assert::AreEqual( std::wstring( L"Monokai" ), std::wstring( GetColorThemeKeyName( COLOR_THEME_MONOKAI ) ) );
			Assert::AreEqual( std::wstring( L"Custom" ), std::wstring( GetColorThemeKeyName( COLOR_THEME_CUSTOM ) ) );
			Assert::AreEqual( (int)COLOR_THEME_VS2012DARK, GetColorThemeFromKeyName( L"vs2012dark" ) );	// 英大文字と小文字を区別しない
			Assert::AreEqual( (int)COLOR_THEME_BENIZAKURA, GetColorThemeFromKeyName( L"Unknown" ) );
			Assert::AreEqual( (int)COLOR_THEME_BENIZAKURA, GetColorThemeFromKeyName( NULL ) );
			Assert::IsTrue( IsColorThemePreset( COLOR_THEME_HIDEMARU_BLACK ) );
			Assert::IsFalse( IsColorThemePreset( COLOR_THEME_CUSTOM ) );
		}
	};
}
