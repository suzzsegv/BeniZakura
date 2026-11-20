/*!
 *	@file
 *	@brief	DarkModeColor (ダークモードの判定と配色表) の単体テスト
 *
 * 画面の配色の設定 (ライト / ダーク / OS の設定に従う) と、OS のモード・ハイコントラストの組み合わせで、
 * ダークモードを使うかの判定が正しいこと、ダークモードの配色表が主なシステムカラーに暗い背景・明るい文字を
 * 返し、表にない番号は返さないことを確認する。
 *
 *	@author	Claude Opus 5.5
 *	@date	2026/11/20 新規作成
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
#include "CppUnitTest.h"
#include "uiparts/DarkModeColor.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeniZakuraUnitTest
{
	/*!
	 * GetLuminance - 色の明るさ (0〜255 の目安) を返す
	 *
	 *	@return 明るさ (R・G・B の加重平均)
	 */
	static int GetLuminance(
		COLORREF	col		//!< [in] 色
	)
	{
		return ( GetRValue( col ) * 299 + GetGValue( col ) * 587 + GetBValue( col ) * 114 ) / 1000;
	}

	TEST_CLASS( DarkModeColorTest )
	{
	public:
		//! ライトの設定では、OS のモードによらずダークモードを使わない
		TEST_METHOD( ModeLight )
		{
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_LIGHT, false, false ) );
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_LIGHT, true, false ) );
		}

		//! ダークの設定では、OS のモードによらずダークモードを使う
		TEST_METHOD( ModeDark )
		{
			Assert::IsTrue( IsDarkModeEnabledFor( UI_COLOR_MODE_DARK, false, false ) );
			Assert::IsTrue( IsDarkModeEnabledFor( UI_COLOR_MODE_DARK, true, false ) );
		}

		//! OS の設定に従う場合は、OS のモードがダークのときだけダークモードを使う
		TEST_METHOD( ModeSystem )
		{
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_SYSTEM, false, false ) );
			Assert::IsTrue( IsDarkModeEnabledFor( UI_COLOR_MODE_SYSTEM, true, false ) );
		}

		//! ハイコントラストのときは、どの設定でもダークモードを使わない
		TEST_METHOD( HighContrast )
		{
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_DARK, true, true ) );
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_SYSTEM, true, true ) );
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_LIGHT, false, true ) );
		}

		//! 不明な設定値はライトとして扱う
		TEST_METHOD( ModeInvalid )
		{
			Assert::IsFalse( IsDarkModeEnabledFor( -1, true, false ) );
			Assert::IsFalse( IsDarkModeEnabledFor( UI_COLOR_MODE_MAX, true, false ) );
		}

		//! 背景系のシステムカラーは暗く、文字系は明るい
		TEST_METHOD( PaletteContrast )
		{
			const int anBack[] = { COLOR_WINDOW, COLOR_MENU, COLOR_MENUBAR, COLOR_3DFACE, COLOR_INFOBK };
			const int anText[] = { COLOR_WINDOWTEXT, COLOR_MENUTEXT, COLOR_BTNTEXT, COLOR_INFOTEXT, COLOR_HIGHLIGHTTEXT };
			for( int nBack : anBack ){
				COLORREF colBack = 0;
				Assert::IsTrue( GetDarkModeSysColor( nBack, &colBack ) );
				Assert::IsTrue( GetLuminance( colBack ) < 64 );
				for( int nText : anText ){
					COLORREF colText = 0;
					Assert::IsTrue( GetDarkModeSysColor( nText, &colText ) );
					Assert::IsTrue( GetLuminance( colText ) - GetLuminance( colBack ) >= 128 );
				}
			}
			// 無効な文字は、背景より明るく、通常の文字より暗い
			COLORREF colGray = 0, colText = 0, colMenu = 0;
			Assert::IsTrue( GetDarkModeSysColor( COLOR_GRAYTEXT, &colGray ) );
			Assert::IsTrue( GetDarkModeSysColor( COLOR_MENUTEXT, &colText ) );
			Assert::IsTrue( GetDarkModeSysColor( COLOR_MENU, &colMenu ) );
			Assert::IsTrue( GetLuminance( colMenu ) < GetLuminance( colGray ) );
			Assert::IsTrue( GetLuminance( colGray ) < GetLuminance( colText ) );
		}

		//! 配色表にない番号は false を返し、出力を変更しない
		TEST_METHOD( PaletteUnknown )
		{
			COLORREF col = RGB( 1, 2, 3 );
			Assert::IsFalse( GetDarkModeSysColor( COLOR_SCROLLBAR, &col ) );
			Assert::IsFalse( GetDarkModeSysColor( -1, &col ) );
			Assert::AreEqual( (unsigned long)RGB( 1, 2, 3 ), (unsigned long)col );
		}
	};
}
