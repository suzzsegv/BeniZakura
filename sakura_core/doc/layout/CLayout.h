/*!	@file
	@brief テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayout;
class CLayoutMgr;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_



#include "CEol.h"// 2002/2/10 aroka
#include "doc/logic/CDocLine.h"// 2002/4/21 YAZAKI
#include "mem/CMemory.h"// 2002/4/21 YAZAKI
#include "view/colors/EColorIndexType.h"
#include "view/colors/ColorStrategyState.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CLayout
{
protected:
	friend class CLayoutMgr; //####仮
public:
	/*
	||  Constructors
	*/
	//2007.08.23 kobake コンストラクタでメンバ変数を初期化するようにした
	CLayout(
		const CDocLine*	pcDocLine,		//!< 実データへの参照
		CLogicPoint		ptLogicPos,		//!< 実データ参照位置
		CLogicInt		nLength,		//!< 実データ内データ長
		EColorIndexType	colorIndexPrev,
		ColorStrategyState colorStrategyStatePrev,
		CLayoutInt		nTypeIndent
	)
	{
		m_pPrev				= NULL;
		m_pNext				= NULL;
		m_pCDocLine			= pcDocLine;
		m_ptLogicPos		= ptLogicPos;		// 実データ参照位置
		m_nLength			= nLength;			// 実データ内データ長
		m_colorIndexPrev	= colorIndexPrev;	// 前行最終文字のカラーインデックス(パレット)番号
		colorStrategyState	= colorStrategyStatePrev;	// 前行最終文字のカラーリング状態
		m_nIndent			= nTypeIndent;		// このレイアウト行のインデント数
	}
	~CLayout();
	void DUMP( void );
	
	// m_ptLogicPos.xで補正したあとの文字列を得る
	const wchar_t* GetPtr() const   { return m_pCDocLine->GetPtr() + m_ptLogicPos.x; }
	CLogicInt GetLengthWithEOL() const    { return m_nLength;	}	//	ただしEOLは常に1文字とカウント？？
	CLogicInt GetLengthWithoutEOL() const { return m_nLength - (m_cEol.GetLen() ? 1 : 0);	}
	//CLogicInt GetLength() const {	return m_nLength;	}	//	CMemoryIterator用（EOL含む）
	CLayoutInt GetIndent() const {	return m_nIndent;	}	//!< このレイアウト行のインデントサイズを取得。単位は半角文字。	CMemoryIterator用

	//取得インターフェース
	CLogicInt GetLogicLineNo() const{ if(this)return m_ptLogicPos.GetY2(); else return CLogicInt(-1); } //$$$高速化
	CLogicInt GetLogicOffset() const{ return m_ptLogicPos.GetX2(); }
	CLogicPoint GetLogicPos() const{ return m_ptLogicPos; }
	EColorIndexType GetColorTypePrev() const{ return m_colorIndexPrev; }
	ColorStrategyState GetColorStrategyStatePrev() const{ return colorStrategyState; }
	CLayoutInt GetLayoutWidth() const{ return m_nLayoutWidth; }		// 2009.08.28 nasukoji	このレイアウト行の改行を含むレイアウト長を返す

	//変更インターフェース
	void OffsetLogicLineNo(CLogicInt n){ m_ptLogicPos.y+=n; }
	void SetColorTypePrev(EColorIndexType colorIndexPrev)
	{
		m_colorIndexPrev = colorIndexPrev;
	}
	void SetColorStrategyStatePrev(ColorStrategyState colorStrategyStatePrev)
	{
		colorStrategyState = colorStrategyStatePrev;
	}
	void SetLayoutWidth(CLayoutInt nWidth){ m_nLayoutWidth = nWidth; }

	//!レイアウト幅を計算。インデントも改行も含まない。2007.10.11 kobake
	CLayoutInt CalcLayoutWidth(const CLayoutMgr& cLayoutMgr) const;

	//! オフセット値をレイアウト単位に変換して取得。2007.10.17 kobake
	CLayoutInt CalcLayoutOffset(const CLayoutMgr& cLayoutMgr) const;

	//! 文字列参照を取得
	CStringRef GetStringRef() const{ return CStringRef(GetPtr(), GetLengthWithEOL()); }

	//チェーン属性
	CLayout* GetPrevLayout(){ return m_pPrev; }
	const CLayout* GetPrevLayout() const{ return m_pPrev; }
	CLayout* GetNextLayout(){ return m_pNext; }
	const CLayout* GetNextLayout() const{ return m_pNext; }
	void _SetPrevLayout(CLayout* pcLayout){ m_pPrev = pcLayout; }
	void _SetNextLayout(CLayout* pcLayout){ m_pNext = pcLayout; }

	//実データ参照
	const CDocLine* GetDocLineRef() const{ if(this)return m_pCDocLine; else return NULL; } //$$note:高速化

	//その他属性参照
	const CEol& GetLayoutEol() const{ return m_cEol; }

private:
	CLayout*			m_pPrev;
	CLayout*			m_pNext;

	//データ参照範囲
	const CDocLine*		m_pCDocLine;		//!< 実データへの参照
	CLogicPoint			m_ptLogicPos;		//!< 対応するロジック参照位置
	CLogicInt			m_nLength;			//!< このレイアウト行の長さ。文字単位。
	
	//その他属性
	EColorIndexType		m_colorIndexPrev;	//!< 前行最終文字のカラーインデックス(パレット)番号
	ColorStrategyState	colorStrategyState;	//!< 前行最終文字のカラーリング状態
	CLayoutInt			m_nIndent;			//!< このレイアウト行のインデント数 @@@ 2002.09.23 YAZAKI
	CEol				m_cEol;
	CLayoutInt			m_nLayoutWidth;		//!< このレイアウト行の改行を含むレイアウト長（「折り返さない」選択時のみ）	// 2009.08.28 nasukoji
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */



