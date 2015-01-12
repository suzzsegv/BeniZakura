/*!	@file	CColor_Comment_Cpp.cpp
	@brief	C++ #if 0 ～ #else ～ #endif コメント判定クラス

	@author	Suzuki Satoshi
*/
/*
	Copyright (C) 2012, 2013, 2014, Suzuki Satoshi

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "view/CEditView.h"
#include "CColor_Comment_Cpp_If0.h"

/*!
	C++ プリプロセッサ "#if 0" 開始判定

	@retval true: "#if 0" を検出した
	@retval false: 検出しなかった
 */
bool CColor_Comment_Cpp_If0::BeginColor( const CStringRef& rStr, int pos, ColorStrategyState& rColorStrategyState )
{
	if( !rStr.IsValid( ) ){
		return false;
	}

	const CEditDoc* pEditDoc = CEditDoc::GetInstance( 0 );
	const STypeConfig& rTypeSetting = pEditDoc->m_cDocType.GetDocumentAttribute( );

	if( rTypeSetting.m_ColorInfoArr[COLORIDX_COMMENT2].m_bDisp == false ){
		return false;
	}

	if( Match_CommentFrom( pos, rStr, rColorStrategyState ) ){
		this->commentEndPos = Match_CommentTo( pos + wcslen( L"#if 0" ), rStr, rColorStrategyState );

		return true;
	}

	return false;
}
/*!
	C++ プリプロセッサによるコメントアウトの終了判定

	@retval true: コメントアウト終了を検出した
	@retval false: 検出しなかった
 */
bool CColor_Comment_Cpp_If0::EndColor( const CStringRef& rStr, int pos, ColorStrategyState& rColorStrategyState )
{
	if( this->commentEndPos == 0 ){
		this->commentEndPos = Match_CommentTo( pos, rStr, rColorStrategyState );
	}else if( pos == this->commentEndPos ){
		return true;
	}

	return false;
}

/*!
	"#if 0" プリプロセッサ コメント開始判定用の文字列比較処理

	@retval true: "#if 0" を検出した
	@retval false: 検出しなかった
*/
bool CColor_Comment_Cpp_If0::Match_CommentFrom
	(
		int pos,								//!< [in] 探索開始位置
		const CStringRef& rStr,					//!< [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
		ColorStrategyState& rColorStrategyState //!< [in, out] 状態( #if 0 ネストレベル)
	)
{
	int len;

	len = wcslen( L"#if 0" );
	if( ( pos <= rStr.GetLength() - len )
	 && ( wcsncmp( &rStr.GetPtr()[pos], L"#if 0", len ) == 0 ) )
	{
		rColorStrategyState.cppPreprocessorIf0NestLevel = 1;
		return true;
	}

	return false;
}


/*!
	"#if 0" プリプロセッサ コメント終了判定用の文字列比較処理

	@return コメントアウト終了位置を返す。終了文字が検出されなかった場合には、nLineLen をそのまま返す。
*/
int CColor_Comment_Cpp_If0::Match_CommentTo
	(
		int pos,								//!< [in] 探索開始位置
		const CStringRef& rStr,					//!< [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
		ColorStrategyState& rColorStrategyState	//!< [in, out] 状態( #if 0 ネストレベル)
	)
{
	int i;
	int targetLen;
	int len;

	targetLen = rStr.GetLength( );
	for( i = pos; i <= ( targetLen - (int)wcslen( L"//" ) ); i++ ){
		if( wcsncmp( &rStr.GetPtr( )[i], L"/*", 2 ) == 0 ){
			i += 2;
			for(; i <= ( targetLen - (int)wcslen( L"*/" ) ); i++ ){
				if( wcsncmp( &rStr.GetPtr( )[i], L"*/", 2 ) == 0 ){
					break;
				}
			}
		}

		if( wcsncmp( &rStr.GetPtr( )[i], L"//", 2 ) == 0 ){
			return targetLen;
		}

		len = wcslen( L"#if " );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#if ", len ) == 0 ){
			rColorStrategyState.cppPreprocessorIf0NestLevel++;
		}

		len = wcslen( L"#if\t" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#if\t", len ) == 0 ){
			rColorStrategyState.cppPreprocessorIf0NestLevel++;
		}

		len = wcslen( L"#ifdef" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#ifdef", len ) == 0 ){
			rColorStrategyState.cppPreprocessorIf0NestLevel++;
		}

		len = wcslen( L"#ifndef" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#ifndef", len ) == 0 ){
			rColorStrategyState.cppPreprocessorIf0NestLevel++;
		}

		len = wcslen( L"$endif" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#endif", len ) == 0 ){
			rColorStrategyState.cppPreprocessorIf0NestLevel--;
			if (rColorStrategyState.cppPreprocessorIf0NestLevel == 0){
				return i + len;
			}
		}

		len = wcslen( L"#else" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#else", len ) == 0 ){
			if (rColorStrategyState.cppPreprocessorIf0NestLevel == 1){
				rColorStrategyState.cppPreprocessorIf0NestLevel = 0;
				return i + len;
			}
		}

		len = wcslen( L"#elif" );
		if( wcsncmp( &rStr.GetPtr( )[i], L"#elif", len ) == 0 ){
			if (rColorStrategyState.cppPreprocessorIf0NestLevel == 1){
				rColorStrategyState.cppPreprocessorIf0NestLevel = 0;
				return i + len;
			}
		}
	}

	return targetLen;
}
