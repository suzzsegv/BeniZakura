/*!
 *	@file
 *	@brief	CColorTheme (カラーテーマの定義と適用) の単体テスト
 *
 * テーマの適用で、テーマが決める値 (文字色・背景色・太字・下線、5 項目の「色分け/表示」) だけが変わり、
 * タイプごとの「色分け/表示」が変わらないこと、テーマの名前と番号の変換、移行処理を確認する。
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
	 * AssertThemeValuesEqual - テーマが決める値 (文字色・背景色・太字・下線、5 項目の「色分け/表示」) が等しいことを確認する
	 */
	static void AssertThemeValuesEqual(
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
			if( IsColorThemeDispItem( i ) ){
				Assert::AreEqual( pExpected[i].m_bDisp, pActual[i].m_bDisp, strItem.c_str() );
			}
		}
	}

	/*!
	 * SetTypeDispPattern - タイプが決める「色分け/表示」を、項目ごとに交互の値にする (テーマの値と区別するため)
	 */
	static void SetTypeDispPattern(
		ColorInfo*	pColorInfoArr	//!< [i/o] 色設定 (COLORIDX_LAST 個)
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			if( !IsColorThemeDispItem( i ) ){
				pColorInfoArr[i].m_bDisp = ( i % 2 == 0 );
			}
		}
	}

	/*!
	 * AssertTypeDispPattern - タイプが決める「色分け/表示」が、SetTypeDispPattern() の値のままであることを確認する
	 */
	static void AssertTypeDispPattern(
		const ColorInfo*	pColorInfoArr	//!< [in] 色設定 (COLORIDX_LAST 個)
	)
	{
		for( int i = 0; i < COLORIDX_LAST; i++ ){
			if( !IsColorThemeDispItem( i ) ){
				const std::wstring strItem = L"item " + std::to_wstring( i );
				Assert::AreEqual( ( i % 2 == 0 ), pColorInfoArr[i].m_bDisp, strItem.c_str() );
			}
		}
	}

	TEST_CLASS( CColorThemeApplyTest )
	{
	public:
		/*!
		 * ApplyPreset - プリセットを適用すると、テーマが決める値だけがプリセットの値になる
		 */
		TEST_METHOD( ApplyPreset )
		{
			for( int nTheme = 0; nTheme < COLOR_THEME_MAX; nTheme++ ){
				ColorInfo arr[COLORIDX_LAST];
				SetPresetColorInfo( arr, COLOR_THEME_BENIZAKURA );
				SetTypeDispPattern( arr );
				arr[COLORIDX_CARETLINEBG].m_bDisp = !arr[COLORIDX_CARETLINEBG].m_bDisp;

				ApplyColorTheme( arr, nTheme, NULL );

				ColorInfo expected[COLORIDX_LAST];
				SetPresetColorInfo( expected, nTheme );
				AssertThemeValuesEqual( expected, arr );
				AssertTypeDispPattern( arr );
				// 名前とインデックスは変わらない
				Assert::AreEqual( std::wstring( expected[COLORIDX_TEXT].m_szName ), std::wstring( arr[COLORIDX_TEXT].m_szName ) );
				Assert::AreEqual( (int)COLORIDX_RULER, arr[COLORIDX_RULER].m_nColorIdx );
			}
		}

		/*!
		 * ApplyCustom - カスタムを適用すると、テーマが決める値だけがカスタムの色になる
		 */
		TEST_METHOD( ApplyCustom )
		{
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_MONOKAI );
			custom[COLORIDX_TEXT].m_colTEXT = RGB( 0x12, 0x34, 0x56 );
			custom[COLORIDX_COMMENT].m_bBoldFont = !custom[COLORIDX_COMMENT].m_bBoldFont;
			custom[COLORIDX_TAB].m_bDisp = !custom[COLORIDX_TAB].m_bDisp;
			custom[COLORIDX_SSTRING].m_bDisp = !custom[COLORIDX_SSTRING].m_bDisp;	// タイプが決める項目 (使われない)

			ColorInfo arr[COLORIDX_LAST];
			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			SetTypeDispPattern( arr );

			ApplyColorTheme( arr, COLOR_THEME_CUSTOM, custom );

			AssertThemeValuesEqual( custom, arr );
			AssertTypeDispPattern( arr );
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
			AssertThemeValuesEqual( expected, arr );

			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ApplyColorTheme( arr, 99, NULL );
			AssertThemeValuesEqual( expected, arr );

			SetPresetColorInfo( arr, COLOR_THEME_SAKURA );
			ApplyColorTheme( arr, -1, NULL );
			AssertThemeValuesEqual( expected, arr );
		}

		/*!
		 * ThemeDispItems - テーマが「色分け/表示」を決める項目は 5 つ
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
				Assert::IsTrue( arr[i].m_szName[0] != _T('\0') );
			}
			Assert::AreEqual( std::wstring( L"テキスト" ), std::wstring( arr[COLORIDX_TEXT].m_szName ) );
		}

		/*!
		 * MigrateFromType - テーマの設定がない ini の移行で、タイプの色がカスタムの色になる
		 */
		TEST_METHOD( MigrateFromType )
		{
			ColorInfo type0[COLORIDX_LAST];
			SetPresetColorInfo( type0, COLOR_THEME_MIFES_BLUE );
			type0[COLORIDX_TEXT].m_colBACK = RGB( 0x01, 0x02, 0x03 );
			SetTypeDispPattern( type0 );

			int nColorTheme = COLOR_THEME_BENIZAKURA;
			ColorInfo custom[COLORIDX_LAST];
			SetPresetColorInfo( custom, COLOR_THEME_BENIZAKURA );
			SetCustomColorThemeFromType( &nColorTheme, custom, type0 );

			Assert::AreEqual( COLOR_THEME_CUSTOM, nColorTheme );
			AssertThemeValuesEqual( type0, custom );

			// 移行後にテーマを適用しても、タイプの色とタイプごとの「色分け/表示」は変わらない
			ColorInfo applied[COLORIDX_LAST];
			memcpy( applied, type0, sizeof(applied) );
			ApplyColorTheme( applied, nColorTheme, custom );
			AssertThemeValuesEqual( type0, applied );
			AssertTypeDispPattern( applied );
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
