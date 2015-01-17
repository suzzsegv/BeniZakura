#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Numeric.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/layout/CLayout.h"
#include "types/CTypeSupport.h"

static int IsNumber( const CStringRef& cStr, int offset );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         半角数値                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Numeric::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	int	nnn;

	if( _IsPosKeywordHead(cStr,nPos) && m_pTypeData->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
		&& (nnn = IsNumber(cStr, nPos)) > 0 )		/* 半角数字を表示する */
	{
		/* キーワード文字列の終端をセットする */
		this->m_nCOMMENTEND = nPos + nnn;
		return true;	/* 半角数値である */
	}
	return false;
}


bool CColor_Numeric::EndColor(const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
 * 数値なら長さを返す。
 * 10進数の整数または小数。16進数(正数)。
 * 文字列   数値(色分け)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1
 * .5       5
 * -.5      5
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * 半角数値
 *   1, 1.2, 1.2.3, .1, 0xabc, -.1, -1
 *   10進数, 16進数, 浮動小数点数, 負符号
 *   IPアドレスのドット連結(本当は数値じゃないんだよね)
 */
static int IsNumber(const CStringRef& cStr, int offset)
{
	register const wchar_t* p;
	register const wchar_t* q;
	register int i = 0;
	register int d = 0;

	p = cStr.GetPtr() + offset;
	q = cStr.GetPtr() + cStr.GetLength();

	if( *p == L'0' )  /* 10進数,Cの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == L'x' ) )  /* Cの16進数 */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= L'0' && *p <= L'9' )
				 || ( *p >= L'A' && *p <= L'F' )
				 || ( *p >= L'a' && *p <= L'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" なら "0" だけが数値 */
			if( i == 2 ) return 1;
			return i;
		}
		else if( *p >= L'0' && *p <= L'9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < L'0' || *p > L'9' )
				{
					if( *p == L'.' )
					{
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == L'.' ) break;  /* "." が連続なら中断 */
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == L'.' ) return i - 1;  /* 最後が "." なら含めない */
			return i;
		}
		else if( *p == L'.' )
		{
			while( p < q )
			{
				if( *p < L'0' || *p > L'9' )
				{
					if( *p == L'.' )
					{
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == L'.' ) break;  /* "." が連続なら中断 */
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == L'.' ) return i - 1;  /* 最後が "." なら含めない */
			return i;
		}
		/* "0" だけが数値 */
		return i;
	}

	else if( *p >= L'1' && *p <= L'9' )  /* 10進数 */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." が連続なら中断 */
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == L'.' ) return i - 1;  /* 最後が "." なら含めない */
		return i;
	}

	else if( *p == L'-' )  /* マイナス */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." が連続なら中断 */
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." だけなら数値でない */
		if( i == 1 ) return 0;
		if( *(p - 1) == L'.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}
		return i;
	}

	else if( *p == L'.' )  /* 小数点 */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." が連続なら中断 */
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." だけなら数値でない */
		if( i == 1 ) return 0;
		if( *(p - 1)  == L'.' ) return i - 1;  /* 最後が "." なら含めない */
		return i;
	}

	/* 数値ではない */
	return 0;
}
